#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EventSystem.hpp"

class UIButton;
class SpriteSheet;

enum class GameState
{
	NONE,
	GAME_STATE_MAIN_MENU,
	GAME_STATE_EDITOR,
	//GAME_STATE_PLAYING,
};

class LevelEditor;
class Level;

class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void ShutDown();
	SpriteSheet* CreateOrGetSpriteSheet(IntVec2 const& gridLayout);
	//void LoadAssets();
	void HandleQuitRequest();
	void CheckInputDeveloperCheats();
	void Update();
	void Render() const;

	void InitEditor();
	void DeInitEditor();
	void CheckInputEditor();
	void UpdateEditor();
	void RenderEditor() const; 

	void InitMainMenu();
	void DeInitMainMenu();
	void CheckInputMainMenu();
	void UpdateMainMenu();
	void RenderMainMenu() const;

	void SetGameState(GameState gameState);
	//void SetTimeScale(float timeScale);
	//void PauseGame();

	//bool IsGamePaused() const;
	bool IsDebugModeActive() const;

	bool TestMethod(EventArgs& args);
private:
	Camera m_uiCamera;
	GameState m_gameState = GameState::NONE;
	//Clock m_gameClock;
	bool m_debugMode = false;

	UIButton* m_newLevelButton = nullptr;
	UIButton* m_loadLevelButton = nullptr;
	bool m_showLevelLoadWindow = false;
	std::string m_levelPathToLoad = "";
	int m_selectLevelIndex = -1;

	std::vector<SpriteSheet*> m_spriteSheets;
};
