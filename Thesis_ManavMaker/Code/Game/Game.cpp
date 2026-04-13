#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/LevelEditor.hpp"
#include "Game/Level.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/UIButton.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include <filesystem>
#include "Engine/Core/NamedProperties.hpp"
#include <windows.h> 
#include "Engine/Window/Window.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/UICheckbox.hpp"

RandomNumberGenerator* g_rng = nullptr;
// SpriteSheet* g_spriteSheet_128x128 = nullptr;
// SpriteSheet* g_spriteSheet_128x64 = nullptr;
// SpriteSheet* g_spriteSheet_64x64 = nullptr;
// SpriteSheet* g_spriteSheet_32x32 = nullptr;
// SpriteSheet* g_spriteSheet_16x16 = nullptr;
Texture* g_spriteSheetTexture = nullptr;
BitmapFont* g_bitmapFont = nullptr;

bool g_muteSfx = false;

LevelEditor* g_levelEditor = nullptr;

Game::Game()
{

}


Game::~Game()
{
}


void Game::Startup()
{
	g_rng = new RandomNumberGenerator();
	g_spriteSheetTexture = g_theRenderer->CreateOrGetTexture("Data/Images/ManavMakerSpriteSheet.png");
// 	g_spriteSheet_128x128 = new SpriteSheet(*g_spriteSheetTexture, IntVec2(128, 128));
// 	g_spriteSheet_128x64 = new SpriteSheet(*g_spriteSheetTexture, IntVec2(128, 64));
// 	g_spriteSheet_64x64 = new SpriteSheet(*g_spriteSheetTexture, IntVec2(64, 64));
// 	g_spriteSheet_32x32 = new SpriteSheet(*g_spriteSheetTexture, IntVec2(32, 32));
// 	g_spriteSheet_16x16 = new SpriteSheet(*g_spriteSheetTexture, IntVec2(16, 16));
	g_bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelPix_MedBoldProp");
	g_bitmapFont->InitialiseUsingImage("Data/Fonts/SquirrelPix_MedBoldProp.png");

	TileDefinition::InitialiseDefinitions();
	EntityDefinition::InitialiseDefinitions();

	SetGameState(GameState::GAME_STATE_MAIN_MENU);
}


void Game::ShutDown()
{
	g_inputSystem->PopMouseConfigOfPriority(1);

	switch (m_gameState)
	{
		case GameState::GAME_STATE_EDITOR:		DeInitEditor();			break;
		case GameState::GAME_STATE_MAIN_MENU:	DeInitMainMenu();		break;
		case GameState::NONE:					//fall through
		default: break;
	}

	delete g_rng;
	g_rng = nullptr;
}


void Game::HandleQuitRequest()
{
	g_app->HandleQuitRequested();
}


void Game::CheckInputDeveloperCheats()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugMode = !m_debugMode;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}





void Game::Update()
{
	CheckInputDeveloperCheats();

	//check input
	switch (m_gameState)
	{
		case GameState::GAME_STATE_EDITOR:		CheckInputEditor();			break;
		case GameState::GAME_STATE_MAIN_MENU:	CheckInputMainMenu();		break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameState::GAME_STATE_EDITOR:		UpdateEditor();			break;
		case GameState::GAME_STATE_MAIN_MENU:	UpdateMainMenu();		break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}
}


void Game::Render() const
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_EDITOR:		RenderEditor();			break;
		case GameState::GAME_STATE_MAIN_MENU:	RenderMainMenu();		break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}
}


void Game::InitEditor()
{
	MouseConfig mouseConfig = {};
	mouseConfig.m_isHidden = false;
	mouseConfig.m_isLocked = false;
	mouseConfig.m_isRelative = false;
	mouseConfig.m_priority = 1;
	g_inputSystem->PushMouseConfig(mouseConfig);

	g_levelEditor = new LevelEditor();
	//m_levelPathToLoad = "Data/Levels/BowserLevel.xml";
	g_levelEditor->Initialize(m_levelPathToLoad);

	g_muteSfx = g_gameConfigBlackboard.GetValue("muteSfx", false);
}


void Game::DeInitEditor()
{
	if (g_levelEditor)
	{
		g_levelEditor->DeInit();
		delete g_levelEditor;
		g_levelEditor = nullptr;
	}

	g_inputSystem->PopMouseConfigOfPriority(1);
}


void Game::CheckInputEditor()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && !LevelEditor::s_showLevelSaveWindow && !g_levelEditor->m_inPlayMode)
	{
		SetGameState(GameState::GAME_STATE_MAIN_MENU);
	}
}


void Game::UpdateEditor()
{
	g_levelEditor->Update();
}


void Game::RenderEditor() const
{
	g_theRenderer->ClearScreen(Rgba8::GREY);
	g_levelEditor->Render();
}


void Game::InitMainMenu()
{
	MouseConfig mouseConfig = {};
	mouseConfig.m_isHidden = false;
	mouseConfig.m_isLocked = false;
	mouseConfig.m_isRelative = false;
	mouseConfig.m_priority = 1;
	g_inputSystem->PushMouseConfig(mouseConfig);

	static float uiOrthoX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float uiOrthoY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(uiOrthoX, uiOrthoY));
	AABB2 uiCamOrtho = m_uiCamera.GetOrthoCamBoundingBox();

	Vec2 buttonDims = g_gameConfigBlackboard.GetValue("mainMenuButtonDims", Vec2(100.0f, 50.0f));
	Vec2 camSpaceMins = g_gameConfigBlackboard.GetValue("newLevelButtonMins", Vec2(850.0f, 500.0f));
	Vec2 camSpaceMaxs = camSpaceMins + buttonDims;
	m_newLevelButton = UIButton::CreateButton(m_uiCamera, camSpaceMins, camSpaceMaxs, nullptr, Rgba8::WHITE, nullptr, "New Level");
	SpriteSheet* uiSpriteSheet = g_game->CreateOrGetSpriteSheet(IntVec2(8, 32));
	SpriteDefinition const& newLevelButtonImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 12));
	m_newLevelButton->SetImage(&newLevelButtonImage);
	SpriteDefinition const& newLevelButtonHoverImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 13));
	m_newLevelButton->SetHoverImage(&newLevelButtonHoverImage);
	m_newLevelButton->SetHoverTint(Rgba8::WHITE);
	SpriteDefinition const& newLevelButtonClickedImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 14));
	m_newLevelButton->SetClickedImage(&newLevelButtonClickedImage);
	m_newLevelButton->SetClickedTint(Rgba8::WHITE);

	camSpaceMins = g_gameConfigBlackboard.GetValue("loadLevelButtonMins", Vec2(850.0f, 400.0f));;
	camSpaceMaxs = camSpaceMins + buttonDims;
	m_loadLevelButton = UIButton::CreateButton(m_uiCamera, camSpaceMins, camSpaceMaxs, nullptr, Rgba8::WHITE, nullptr, "Load Level");
	SpriteDefinition const& loadLevelButtonImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 15));
	m_loadLevelButton->SetImage(&loadLevelButtonImage);
	SpriteDefinition const& loadLevelButtonHoverImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 16));
	m_loadLevelButton->SetHoverImage(&loadLevelButtonHoverImage);
	m_loadLevelButton->SetHoverTint(Rgba8::WHITE);
	SpriteDefinition const& loadLevelButtonClickedImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 17));
	m_loadLevelButton->SetClickedImage(&loadLevelButtonClickedImage);
	m_loadLevelButton->SetClickedTint(Rgba8::WHITE);
	m_showLevelLoadWindow = false;

	m_levelPathToLoad = "";
}


void Game::DeInitMainMenu()
{
	delete m_newLevelButton;
	m_newLevelButton = nullptr;

	delete m_loadLevelButton;
	m_loadLevelButton = nullptr;

	g_inputSystem->PopMouseConfigOfPriority(1);
}


void Game::CheckInputMainMenu()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && !m_showLevelLoadWindow)
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && m_showLevelLoadWindow)
	{
		m_showLevelLoadWindow = false;
	}

	if (m_newLevelButton->IsMouseOverWidget())
	{
		m_newLevelButton->SetHoveredOnFrame(g_currentFrameNumber);

		if (g_inputSystem->IsMouseButtonPressed(0))
		{
			m_newLevelButton->SetClickedOnFrame(g_currentFrameNumber);
		}

		if (g_inputSystem->WasMouseButtonJustReleased(0))
		{
			m_levelPathToLoad = "";
			SetGameState(GameState::GAME_STATE_EDITOR);
			return;
		}
	}

	if (m_loadLevelButton->IsMouseOverWidget())
	{
		m_loadLevelButton->SetHoveredOnFrame(g_currentFrameNumber);

		if (g_inputSystem->IsMouseButtonPressed(0))
		{
			m_loadLevelButton->SetClickedOnFrame(g_currentFrameNumber);
		}

		if (g_inputSystem->WasMouseButtonJustReleased(0))
		{
			m_showLevelLoadWindow = true;
		}
	}
}


void Game::UpdateMainMenu()
{
	bool useWindowDialog = g_gameConfigBlackboard.GetValue("useWindowDialog", false);
	static char directory[1024] = "";
	if (m_showLevelLoadWindow)
	{
		if (useWindowDialog)
		{
			GetCurrentDirectoryA(1024, directory);
			OPENFILENAMEA openFileDialogOptions = {};
			char fileName[MAX_PATH] = "";

			openFileDialogOptions.lStructSize = sizeof(OPENFILENAMEA);
			openFileDialogOptions.hwndOwner = (HWND) g_window->GetWindowHandle();
			openFileDialogOptions.lpstrFilter = "XML Files\0*.XML\0\0";
			openFileDialogOptions.lpstrFile = fileName;
			openFileDialogOptions.nMaxFile = MAX_PATH;
			openFileDialogOptions.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			openFileDialogOptions.lpstrDefExt = "xml";
			openFileDialogOptions.lpstrTitle = "Load Level";

			if (GetOpenFileNameA(&openFileDialogOptions))
				m_levelPathToLoad = fileName;

			m_showLevelLoadWindow = false;
		}
		else
		{
			ImGui::Begin("Select Level");
			if (ImGui::BeginListBox("Levels on disk"))
			{
				int index = 0;
				for (const auto& file : std::filesystem::directory_iterator("Data/Levels"))
				{
					static std::string rootPath = "Data/Levels/";
					bool isSelected = index == m_selectLevelIndex;
					std::string levelPath = file.path().string();
					std::string levelNameWithExtension = levelPath;
					levelNameWithExtension.erase(0, rootPath.size());
					Strings levelNameComponents = SplitStringOnDelimiter(levelNameWithExtension, '.');
					std::string levelNameWithoutExtension = levelNameComponents[0];
					if (ImGui::Selectable(levelNameWithoutExtension.c_str(), isSelected))
					{
						m_selectLevelIndex = index;
						m_levelPathToLoad = levelPath;
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
					index++;
				}
				ImGui::EndListBox();
			}

			if (m_selectLevelIndex >= 0)
			{
				if (ImGui::Button("Load Level"))
				{
					SetGameState(GameState::GAME_STATE_EDITOR);
				}
			}

			ImGui::Text("If you don't see a level here, make sure it is in the Data/Levels folder.");
			ImGui::End();
		}
	}

	if (useWindowDialog)
	{
		if (!m_levelPathToLoad.empty())
		{
			SetCurrentDirectoryA(directory);
			SetGameState(GameState::GAME_STATE_EDITOR);
		}
	}
}


void Game::RenderMainMenu() const
{
	g_theRenderer->ClearScreen(Rgba8::CYAN);
	g_theRenderer->BeginCamera(m_uiCamera);

	m_newLevelButton->Render();
	m_loadLevelButton->Render();

	std::vector<Vertex_PCU> textVerts;
	static Vec2 welcomeToTextMins = g_gameConfigBlackboard.GetValue("welcomeToTextMins", Vec2(800.0f, 700.0f));
	static Vec2 manavMakerTextMins = g_gameConfigBlackboard.GetValue("manavMakerTextMins", Vec2(800.0f, 500.0f));
	g_bitmapFont->AddVertsForText2D(textVerts, welcomeToTextMins, 50.0f, "Welcome to");
	g_bitmapFont->AddVertsForText2D(textVerts, manavMakerTextMins, 84.0f, "ManavMaker");
	g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());

	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::SetGameState(GameState gameState)
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_EDITOR:		DeInitEditor();			break;
		case GameState::GAME_STATE_MAIN_MENU:	DeInitMainMenu();		break;
		case GameState::NONE:					//fall through
		default: break;
	}

	m_gameState = gameState;

	switch (m_gameState)
	{
		case GameState::GAME_STATE_EDITOR:		InitEditor();			break;
		case GameState::GAME_STATE_MAIN_MENU:	InitMainMenu();			break;
		case GameState::NONE:					//fall through
		default: break;
	}
}


// void Game::SetTimeScale(float timeScale)
// {
// 	m_gameClock.SetTimeScale((double) timeScale);
// }


// void Game::PauseGame()
// {
// 	m_gameClock.Pause();
// }


SpriteSheet* Game::CreateOrGetSpriteSheet(IntVec2 const& gridLayout)
{
	for (int spriteSheetIndex = 0; spriteSheetIndex < (int) m_spriteSheets.size(); spriteSheetIndex++)
	{
		SpriteSheet* spriteSheet = m_spriteSheets[spriteSheetIndex];
		if (spriteSheet->GetGridLayout() == gridLayout)
			return spriteSheet;
	}

	//SpriteSheet newSpriteSheet = SpriteSheet(*g_spriteSheetTexture, gridLayout);
	//m_spriteSheets.emplace_back(*g_spriteSheetTexture, gridLayout);
	SpriteSheet* newSpriteSheet = new SpriteSheet(*g_spriteSheetTexture, gridLayout);
	m_spriteSheets.push_back(newSpriteSheet);
	return m_spriteSheets.back();
}


// bool Game::IsGamePaused() const
// {
// 	return m_gameClock.IsPaused();
// }


bool Game::IsDebugModeActive() const
{
	return m_debugMode;
}


bool Game::TestMethod(EventArgs& args)
{
	UNUSED(args);
	return false;
}

