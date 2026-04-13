#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"

enum class GameState
{
	NONE,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_GAME_WON,
	GAME_STATE_GAME_OVER
};

class Entity;

//class representing an instance of our game
//it is owned by App
class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void ShutDown();
	void LoadAssets();
	void HandleQuitRequest();
	void CheckInputDeveloperCheats();
	void Update();
	void UpdateCameras();
	void Render() const;

	void InitAttractMode();
	void DeInitAttractMode();
	void CheckInputAttractMode();
	void UpdateAttractMode(float deltaSeconds);
	void RenderAttractMode() const;

	void InitPlayMode();
	void DeInitPlayMode();
	void CheckInputPlayMode();
	void UpdatePlayMode(float deltaSeconds);
	void RenderPlayMode() const;
	void RenderPlayModeUI() const;
	void RenderPlayModeDebugger() const;

	void InitGameWonMode();
	void UpdateGameWonMode(float deltaSeconds);
	void CheckInputGameWonMode();
	void RenderGameWonMode() const;
	void DeInitGameWonMode();

	void InitGameOverMode();
	void UpdateGameOverMode(float deltaSeconds);
	void CheckInputGameOverMode();
	void RenderGameOverMode() const;
	void DeInitGameOverMode();

	//mutators
	void SetGameState(GameState gameState);
	void SetTimeScale(float timeScale);

private: //methods
	//Testing variables
	float m_angularVelocity = 10.0f;
	float m_orientation = 0.0f;

	//cameras
	Camera m_worldCamera;
	Camera m_uiCamera;
	
	GameState m_gameState = GameState::NONE;
	Clock m_gameClock;

	bool m_debugMode = false;
};
