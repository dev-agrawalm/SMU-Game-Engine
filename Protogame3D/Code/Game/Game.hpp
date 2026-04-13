#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"

enum class GameState
{
	NONE,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_GAME_WON,
	GAME_STATE_GAME_OVER
};

class Entity;
class Player;
class Prop;
class Clock;

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
	void CheckInputAttractMode();
	void UpdateAttractMode(float deltaSeconds);
	void RenderAttractMode() const;
	void DeInitAttractMode();

	void InitPlayMode();
	void CheckInputPlayMode();
	void UpdatePlayMode(float deltaSeconds);
	void RenderPlayMode() const;
	void DeInitPlayMode();

	void InitGameWonMode();
	void CheckInputGameWonMode();
	void UpdateGameWonMode(float deltaSeconds);
	void RenderGameWonMode() const;
	void DeInitGameWonMode();

	void InitGameOverMode();
	void UpdateGameOverMode(float deltaSeconds);
	void CheckInputGameOverMode();
	void RenderGameOverMode() const;
	void DeInitGameOverMode();

	//mutators
	void SetGameState(GameState gameState);
	void SetGameTimeScale(float scale);

	Player* GetPlayer() const;

protected:
	void RenderGrid() const;
	void RenderEntities() const;
	void UpdateEntities(float deltaSeconds);
	static bool Static_SetPlayerSpeed(EventArgs& args);

private: //methods
	bool m_debugMode = false;
	GameState m_gameState = GameState::NONE;
	
	Camera m_worldCamera;
	Camera m_uiCamera;

	std::vector<Entity*> m_entities;
	Player* m_player = nullptr;
	Prop* m_cube = nullptr;

	Clock* m_gameClock = nullptr;
	Clock* m_gameStateClock = nullptr;
};
