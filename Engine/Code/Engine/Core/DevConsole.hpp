#pragma once
#include<string>
#include<vector>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include <mutex>
#include <atomic>

class Renderer;
class InputSystem;
struct AABB2;

enum class DevConsoleMode
{
	OPEN_FULL,
	HIDDEN
};


struct DevConsoleConfig
{
public:
	Renderer* m_defaultRenderer = nullptr;
	InputSystem* m_inputSystem = nullptr;
	std::string m_defaultFontName = "System";
	float m_defaultFontAspect = 0.6f;
	int m_maxHistoryLength = 128;
	int m_mouseConfigPriority = 100;
	std::string m_commandHistoryFilePath = "Data/CommandHistory.txt";
};


struct DevConsoleLine
{
	Rgba8 m_color;
	std::string m_text = "";
	int m_frameStamp = 0;
	float m_timeStamp = 0.0f;
	bool m_omitStamps = false;
};

typedef std::vector<DevConsoleLine> DevConsoleLines;

class DevConsole
{
public:
	static Rgba8 const ERROR_MESSAGE;
	static Rgba8 const WARNING_MESSAGE;
	static Rgba8 const MAJOR_INFO;
	static Rgba8 const MINOR_INFO;

private:
	static Rgba8 const COMMAND;
	static char	 const CARAT_CHAR;

public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Update();
	bool HandleCharInput(unsigned char input);
	bool HandleKeyInput(unsigned char input);
	void Render(AABB2 const& bounds, Renderer* overridingRenderer = nullptr) const;
	void AddLine(Rgba8 const& color, std::string const& text, bool omitStamps = false);
	void ExecuteCommand(std::string commandString);
	void ToggleMode(DevConsoleMode mode);
	void SetMode(DevConsoleMode mode);
	void ClearLog();
	DevConsoleMode GetMode() const;

private:
	void RenderConsoleOpenFull(AABB2 const& bounds, Renderer* renderer) const;
	void OnInputEntered();
	void HandleCommandInput(std::string commandText);
	void OnEscape();
	bool IsValidCommand(std::string const& commandText);
	void AddCommandToHistory(std::string commandText);

private:
	static bool StaticHandleKeyInput(EventArgs& args);
	static bool StaticHelpCommand(EventArgs& args);
	static bool StaticClearCommand(EventArgs& args);
	void LoadCommandHistoryFromFile();
	void FlushCommandHistoryToFile();
	void FindRegisteredCommandStartingWithString(std::string const& commandStart, std::string& out_commandName);

private:
	DevConsoleConfig m_config;
	DevConsoleMode m_mode = DevConsoleMode::HIDDEN;
	std::mutex m_enteredLinesMutex;
	DevConsoleLines m_enteredLinesCopy;
	DevConsoleLines m_enteredLines;
	std::string m_currentLine;
	std::atomic<int> m_frameNumber = 0;
	
	int m_caratPos = 0;
	bool m_showCarat = true;

	std::vector<std::string> m_commandHistory;
	int m_commandHistoryOffset = 0;

	std::string m_autoCompleteString;

	Clock m_devConsoleClock;
	Stopwatch m_caratStopwatch;
};
