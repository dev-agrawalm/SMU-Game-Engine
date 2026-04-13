#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include <fstream>
#include "Engine/Core/FileUtils.hpp"

constexpr double CARAT_BLINK_RATE = 0.3;

DevConsole* g_console = nullptr;

Rgba8 const DevConsole::ERROR_MESSAGE	= Rgba8(255, 0, 0, 255);
Rgba8 const DevConsole::WARNING_MESSAGE	= Rgba8(255, 255, 0, 255);
Rgba8 const DevConsole::MAJOR_INFO		= Rgba8(0, 255, 0, 255);
Rgba8 const DevConsole::MINOR_INFO		= Rgba8(0, 255, 255, 255);
Rgba8 const DevConsole::COMMAND			= Rgba8(252, 173, 3, 255);
char  const DevConsole::CARAT_CHAR		= '|';


DevConsole::DevConsole(DevConsoleConfig const& config)
	: m_config(config)
{

}


DevConsole::~DevConsole()
{

}


void DevConsole::Startup()
{
	m_frameNumber = 0;
	m_enteredLinesMutex.lock();
	m_enteredLines.clear();
	m_enteredLinesMutex.unlock();
	g_eventSystem->SubscribeEventCallbackFunction("Clear", StaticClearCommand);
	g_eventSystem->SubscribeEventCallbackFunction("KeyDown", StaticHandleKeyInput);
	g_eventSystem->SubscribeEventCallbackFunction("Help", StaticHelpCommand);

	m_caratStopwatch = Stopwatch(m_devConsoleClock, CARAT_BLINK_RATE);

	LoadCommandHistoryFromFile();
}


void DevConsole::Shutdown()
{
	m_enteredLinesMutex.lock();
	m_enteredLines.clear();
	m_enteredLinesMutex.unlock();
	g_eventSystem->UnsubscribeEventCallbackFunction("KeyDown", StaticHandleKeyInput);
	g_eventSystem->UnsubscribeEventCallbackFunction("Help", StaticHelpCommand);
	g_eventSystem->UnsubscribeEventCallbackFunction("Clear", StaticClearCommand);
}


void DevConsole::BeginFrame()
{
	m_frameNumber++;
	if (m_mode == DevConsoleMode::OPEN_FULL)
	{
		m_config.m_inputSystem->ResetInput();
	}
}


void DevConsole::EndFrame()
{

}


void DevConsole::Update()
{
	if (m_mode != DevConsoleMode::HIDDEN)
	{
		if (m_caratStopwatch.CheckAndRestart())
		{
			m_showCarat = !m_showCarat;
		}

		m_caratPos = Clamp(m_caratPos, 0, (int) m_currentLine.size());

		m_enteredLinesCopy.clear();
		m_enteredLinesMutex.lock();
		m_enteredLinesCopy.resize(m_enteredLines.size());
		//memcpy(m_enteredLinesCopy[0], m_enteredLines[0], m_enteredLines.size());
		m_enteredLinesCopy = m_enteredLines;
		m_enteredLinesMutex.unlock();
	}
}


bool DevConsole::HandleCharInput(unsigned char input)
{
	if (m_mode == DevConsoleMode::HIDDEN)
		return false;

	if (input >= 32 && input <= 125)
	{
		std::string character(1, input);
		m_currentLine.insert(m_caratPos, character);
		m_caratPos++;
		
		m_autoCompleteString = "";
		FindRegisteredCommandStartingWithString(m_currentLine, m_autoCompleteString);
		return true;
	}

	return false;
}


bool DevConsole::HandleKeyInput(unsigned char input)
{
	if (m_mode == DevConsoleMode::HIDDEN)
		return false;

	if (input == KEYCODE_TAB)
	{
		if (m_autoCompleteString.length() > 0)
		{
			m_currentLine = m_autoCompleteString;
			m_caratPos = (int) m_currentLine.length();
			return true;
		}
	}
	else
	{
		m_autoCompleteString = "";
	}
	
	if (input == KEYCODE_ESCAPE)
	{
		OnEscape();
		return true;
	}

	if (input == KEYCODE_TILDE)
	{
		ToggleMode(m_mode);
		m_config.m_inputSystem->ResetInput();
		return true;
	}

	if (input == KEYCODE_ENTER)
	{
		OnInputEntered();
		return true;
	}

	if (input == KEYCODE_RIGHT_ARROW)
	{
		m_caratPos += 1;
		m_showCarat = true;
		m_caratStopwatch.Restart();
		return true;
	}

	if (input == KEYCODE_LEFT_ARROW) 
	{
		m_caratPos -= 1; 
		m_showCarat = true;
		m_caratStopwatch.Restart();
		return true;
	}

	if (input == KEYCODE_UP_ARROW)
	{
		int commandCount = (int) m_commandHistory.size();
		if (commandCount > 0)
		{
			m_commandHistoryOffset++;
			m_commandHistoryOffset = Clamp(m_commandHistoryOffset, 0, commandCount);
			int commandIndex = commandCount - m_commandHistoryOffset;
			m_currentLine = m_commandHistory[commandIndex];
			m_caratPos = (int) m_currentLine.length();
			return true;
		}
	}

	if (input == KEYCODE_DOWN_ARROW)
	{
		m_commandHistoryOffset--;
		if (m_commandHistoryOffset > 0)
		{
			int commandCount = (int) m_commandHistory.size();
			int commandIndex = commandCount - m_commandHistoryOffset;
			m_currentLine = m_commandHistory[commandIndex];
			m_caratPos = (int) m_currentLine.length();
		}
		else
		{
			m_commandHistoryOffset = 0;
			m_currentLine = "";
		}
		return true;
	}

	if (input == KEYCODE_BACKSPACE)
	{
		int deletePosition = m_caratPos - 1; //have to delete the char before the carat
		if (deletePosition >= 0)
		{
			m_currentLine.erase(deletePosition, 1);
			m_caratPos--;
			m_showCarat = true;
			m_caratStopwatch.Restart();
		}
		return true;
	}

	if (input == KEYCODE_DELETE)
	{
		int lineLength = (int) m_currentLine.length();
		int deletePosition = m_caratPos;
		if (deletePosition < lineLength)
		{
			m_currentLine.erase(deletePosition, 1);
			m_showCarat = true;
			m_caratStopwatch.Restart();
		}
		return true;
	}

	if (input == KEYCODE_HOME)
	{
		m_caratPos = 0;
		return true;
	}

	if (input == KEYCODE_END)
	{
		m_caratPos = (int) m_currentLine.length();
		return true;
	}

	return false;
}


void DevConsole::Render(AABB2 const& bounds, Renderer* overridingRenderer /*= nullptr*/) const
{
	Renderer* renderer = overridingRenderer ? overridingRenderer : m_config.m_defaultRenderer;

	if (renderer)
	{
		switch (m_mode)
		{
			case DevConsoleMode::OPEN_FULL:
			{
				RenderConsoleOpenFull(bounds, renderer);
				break;
			}
			case DevConsoleMode::HIDDEN: // fall through
			default:
				break;
		}
	}
	else
	{
		ERROR_RECOVERABLE("No renderer is provided for dev console. Exiting function without rendering anything");
	}
}


void DevConsole::RenderConsoleOpenFull(AABB2 const& bounds, Renderer* renderer) const
{
	float lineHeight = bounds.GetDimensions().y * 0.025f;
	float fontHeight = lineHeight * 0.75f;

	std::vector<Vertex_PCU> consoleVertexes;
	Rgba8 consoleBgColor = Rgba8(127, 127, 127, 230);
	Rgba8 textInputBoxColor = Rgba8(255, 255, 255, 200);
	//Render text input box
	Vec2 textInputBoxMins = bounds.m_mins;
	Vec2 textInputBoxMaxs = Vec2(bounds.m_maxs.x, bounds.m_mins.y + lineHeight);
	AABB2 textInputBox = AABB2(textInputBoxMins, textInputBoxMaxs);
	AddVertsForAABB2ToVector(consoleVertexes, textInputBox, textInputBoxColor);
	//render console window
	Vec2 consoleMins = Vec2(bounds.m_mins.x, textInputBox.m_maxs.y);
	Vec2 consoleMaxs = bounds.m_maxs;
	AABB2 consoleBox = AABB2(consoleMins, consoleMaxs);
	AddVertsForAABB2ToVector(consoleVertexes, consoleBox, consoleBgColor);

	renderer->BindTexture(0, nullptr);
	renderer->DrawVertexArray((int) consoleVertexes.size(), consoleVertexes.data());

	//render text
	std::vector<Vertex_PCU> lineVertexes;
	std::string fontFilePathWithoutExtension = "Data/Fonts/" + m_config.m_defaultFontName;
	BitmapFont* font = renderer->CreateOrGetBitmapFont(fontFilePathWithoutExtension.c_str());

	Rgba8 autoCompleteLineColor = Rgba8(0, 0, 0, 150);
	if (m_autoCompleteString.length() > 0)
	{
		font->AddVertsForTextInAABB2(lineVertexes, textInputBox, fontHeight, m_autoCompleteString, autoCompleteLineColor, m_config.m_defaultFontAspect, BitmapFont::ALIGNED_CENTER_LEFT);
	}
	
	//render current line
	std::string currentLineText = m_currentLine;
	if (m_showCarat)
	{
		std::string carat(1, CARAT_CHAR);
		currentLineText.insert(m_caratPos, carat);
	}
	else
	{
		std::string carat(1, ' ');
		currentLineText.insert(m_caratPos, carat);
	}
	Rgba8 currentLineColor = Rgba8::BLACK;
	font->AddVertsForTextInAABB2(lineVertexes, textInputBox, fontHeight, currentLineText, currentLineColor, m_config.m_defaultFontAspect, BitmapFont::ALIGNED_CENTER_LEFT);


	//render previously entered lines
	int numLinesToRender = RoundDownToInt(consoleBox.GetDimensions().y / lineHeight);
	for (int lineIndex = 0; lineIndex <= numLinesToRender && lineIndex < (int) m_enteredLinesCopy.size(); lineIndex++)
	{
		int consoleLineIndex = (int) m_enteredLinesCopy.size() - 1 - lineIndex;
		DevConsoleLine const& line = m_enteredLinesCopy[consoleLineIndex];
		std::string text = line.m_text;
		if (!line.m_omitStamps)
		{
			text = Stringf("[%i, %.2f] ", line.m_frameStamp, line.m_timeStamp) + line.m_text;
		}

		Vec2 textMins = Vec2(bounds.m_mins.x, textInputBoxMaxs.y + lineIndex * lineHeight);
		Vec2 textMaxs = Vec2(bounds.m_maxs.x, textMins.y + lineHeight);
		AABB2 textBox = AABB2(textMins, textMaxs);

		font->AddVertsForTextInAABB2(lineVertexes, textBox, fontHeight, text, line.m_color
									 , m_config.m_defaultFontAspect, BitmapFont::ALIGNED_BOTTOM_LEFT
									 , TextDrawMode::SHRINK_TO_FIT);
	}

	Texture const& fontTexture = font->GetTexture();
	renderer->BindTexture(0, &fontTexture);
	renderer->DrawVertexArray((int) lineVertexes.size(), lineVertexes.data());
}


void DevConsole::AddLine(Rgba8 const& color, std::string const& text, bool omitStamps /*= false*/)
{
	DevConsoleLine newLine;
	newLine.m_color = color;
	newLine.m_text = text;
	newLine.m_frameStamp = m_frameNumber;
	float curTime = static_cast<float>(GetCurrentTimeSeconds());
	newLine.m_timeStamp = curTime;
	newLine.m_omitStamps = omitStamps;
	m_enteredLinesMutex.lock();
	m_enteredLines.push_back(newLine);
	m_enteredLinesMutex.unlock();
}


void DevConsole::ExecuteCommand(std::string commandString)
{
	std::vector<std::string> commands = SplitStringOnDelimiter(commandString, '\n');

	for (int commandIndex = 0; commandIndex < (int) commands.size(); commandIndex++)
	{
		std::string& command = commands[commandIndex];
		std::vector<std::string> commandTokens = SplitStringOnDelimiter(command, ' ');
		std::string& commandName = commandTokens[0];
		EventArgs commandArgs;

		for (int tokenIndex = 1; tokenIndex < (int) commandTokens.size(); tokenIndex++)
		{
			std::vector<std::string> arguments = SplitStringOnDelimiter(commandTokens[tokenIndex], '=');
			std::string& key = arguments[0];
			std::string& value = arguments[1];
			commandArgs.AddProperty(key, value);
		}
		g_eventSystem->FireEvent(commandName, commandArgs);
	}
}


void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;

	if (m_mode == DevConsoleMode::OPEN_FULL)
	{
		m_showCarat = true;
		m_caratStopwatch.Restart();
		
		MouseConfig mouseConfig = {};
		mouseConfig.m_isHidden = false;
		mouseConfig.m_isLocked = false;
		mouseConfig.m_isRelative = false;
		mouseConfig.m_priority = m_config.m_mouseConfigPriority;
		m_config.m_inputSystem->PushMouseConfig(mouseConfig);
	}
	else
	{
		m_config.m_inputSystem->PopMouseConfigOfPriority(m_config.m_mouseConfigPriority);
	}
}


void DevConsole::ClearLog()
{
	m_enteredLinesMutex.lock();
	m_enteredLines.clear();
	m_enteredLinesMutex.unlock();
}


void DevConsole::ToggleMode(DevConsoleMode mode)
{
	if (m_mode == DevConsoleMode::HIDDEN)
	{
		SetMode(mode);
	}
	else
	{
		SetMode(DevConsoleMode::HIDDEN);
	}
}


DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}


bool DevConsole::StaticHandleKeyInput(EventArgs& args)
{
	std::string inputKey = "";
	args.GetProperty("InputKey", inputKey, inputKey);
	bool wasConsumed = g_console->HandleKeyInput(inputKey[0]);
	return wasConsumed;
}


bool DevConsole::StaticHelpCommand(EventArgs& args)
{
	std::string filter;
	args.GetProperty("filter", filter, "");
	std::vector<std::string> existingEventNames = g_eventSystem->GetAllEventNames();
	
	if (filter == "")
	{
		std::string heading = "#Registered Command Count: %i#";
		g_console->AddLine(Rgba8::BLACK, Stringf(heading.c_str(), (int) existingEventNames.size()), true);
	}

	int filteredEventsCount = 0;
	for (int eventIndex = 0; eventIndex < existingEventNames.size(); eventIndex++)
	{
		if (filter == "")
		{
			g_console->AddLine(MINOR_INFO, existingEventNames[eventIndex], true);
		}
		else
		{
			std::string eventName = existingEventNames[eventIndex];
			if (strstr(eventName.c_str(), filter.c_str()) != nullptr)
			{
				filteredEventsCount++;
			}
		}
	}

	if (filter != "")
	{
		std::string heading = "#Registered Command Count containing string " + filter + ": %i#";
		g_console->AddLine(Rgba8::BLACK, Stringf(heading.c_str(), filteredEventsCount), true);
		for (int eventIndex = 0; eventIndex < existingEventNames.size(); eventIndex++)
		{

			std::string eventName = existingEventNames[eventIndex];
			if (strstr(eventName.c_str(), filter.c_str()) != nullptr)
			{
				g_console->AddLine(MINOR_INFO, existingEventNames[eventIndex], true);
			}
		}
	}

	return true;
}


bool DevConsole::StaticClearCommand(EventArgs& args)
{
	UNUSED(args);
	g_console->ClearLog();
	return false;
}


void DevConsole::LoadCommandHistoryFromFile()
{
	m_commandHistory.clear();
	std::string commandHistory;
	bool success = FileReadToString(commandHistory, m_config.m_commandHistoryFilePath);
	if (!success || commandHistory.length() == 0)
		return;

	Strings commands = SplitStringOnDelimiter(commandHistory, '\n');
	m_commandHistory.resize(commands.size());
	for (int commandIndex = 0; commandIndex < (int) commands.size(); commandIndex++)
	{
		if ( (int) commands[commandIndex].size() > 0)
		{
			//commands[commandIndex].pop_back();
			m_commandHistory[commandIndex] = commands[commandIndex];
		}
	}
}


void DevConsole::FlushCommandHistoryToFile()
{
	std::ofstream file;
	file.open(m_config.m_commandHistoryFilePath.c_str(), std::ios::out);
	
	if (m_commandHistory.size() > 0)
	{
		file.write(m_commandHistory[0].c_str(), m_commandHistory[0].length());
		for (int commandIndex = 1; commandIndex < (int) m_commandHistory.size(); commandIndex++)
		{
			file.write("\n", 1);
			std::string& command = m_commandHistory[commandIndex];
			file.write(command.c_str(), command.length());
		}
	}
}


void DevConsole::FindRegisteredCommandStartingWithString(std::string const& commandStart, std::string& out_commandName)
{
	if (commandStart.length() == 0)
		return;

	std::vector<std::string> existingEventNames = g_eventSystem->GetAllEventNames();

	for (int nameIndex = 0; nameIndex < (int) existingEventNames.size(); nameIndex++)
	{
		std::string& eventName = existingEventNames[nameIndex];
		int commandStartLength = (int) commandStart.length();
		std::string evenNameSubStr = eventName.substr(0, commandStartLength);
		if (_strcmpi(evenNameSubStr.c_str(), commandStart.c_str()) == 0)
		{
			out_commandName = eventName;
			return;
		}
	}
}


void DevConsole::OnInputEntered()
{
	if (m_currentLine.length() == 0)
		return;

	HandleCommandInput(m_currentLine);
	m_currentLine = "";
	m_caratPos = 0;
}


void DevConsole::HandleCommandInput(std::string commandText)
{
	if (IsValidCommand(commandText))
	{
		AddLine(MAJOR_INFO, "Executing Command:" + commandText);
		ExecuteCommand(commandText);
	}
	else
	{
		std::string errorMessage = commandText + " : Invalid Command. Enter Help to see available commands.";
		AddLine(ERROR_MESSAGE, errorMessage, true);
	}
	AddCommandToHistory(m_currentLine);
}


void DevConsole::OnEscape()
{
	if (m_currentLine.length() > 0)
	{
		m_currentLine = "";
		m_caratPos = 0;
	}
	else
	{
		SetMode(DevConsoleMode::HIDDEN);
		m_config.m_inputSystem->ResetInput();
	}
}


bool DevConsole::IsValidCommand(std::string const& commandText)
{
	std::vector<std::string> commandTokens = SplitStringOnDelimiter(commandText, ' ');
	std::string& commandName = commandTokens[0];
	return g_eventSystem->DoesEventExist(commandName);
}


void DevConsole::AddCommandToHistory(std::string commandText)
{
	auto it = m_commandHistory.begin();
	while (it != m_commandHistory.end())
	{
		if (strcmp(it->c_str(), commandText.c_str()) == 0)
		{
			m_commandHistory.erase(it);
			break;
		}
		it++;
	}

	if (m_commandHistory.size() < m_config.m_maxHistoryLength)
	{
		m_commandHistory.push_back(commandText);
	}
	else
	{
		m_commandHistory.erase(m_commandHistory.begin());
		m_commandHistory.push_back(commandText);
	}
	m_commandHistoryOffset = 0;

	FlushCommandHistoryToFile();
}
