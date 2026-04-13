#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"

enum class GameState
{
	NONE,
	GAME_STATE_ATTRACT,
	GAME_STATE_LOBBY,
	GAME_STATE_PLAYING,
	GAME_STATE_GAME_WON,
	GAME_STATE_GAME_OVER
};

class World;
class Clock;
class XboxController;

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

	void InitLobbyMode();
	void DeInitLobbyMode();
	void CheckInputLobbyMode();
	void UpdateLobbyMode(float deltaSeconds);
	void RenderLobbyMode() const;

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
	void SetGameTimeScale(float scale);
	void ToggleEditorMode();
	void ToggleDebugRaycasts();
	void ToggleDebugActors();

	World* GetWorld() const;
	GameState GetGameState() const;
	Clock& GetGameStateClock() const;
	Clock& GetGameClock() const;
	bool IsEditorModeActive() const;
	bool ShouldDebugRaycasts() const;
	bool ShouldDebugActors() const;

private: //methods
	Camera m_uiCamera;
	World* m_world = nullptr;

	GameState m_gameState = GameState::NONE;
	Clock* m_gameClock = nullptr;
	Clock* m_gameStateClock = nullptr;

	bool m_inEditorMode = false;
	bool m_debugRaycasts = false;
	bool m_debugActors = false;

	SoundPlaybackID m_attractMusicID = 0;
	SoundPlaybackID m_playMusicID = 0;

	int m_firstPlayerControllerIndex = -999;
	XboxController const* m_controllers[NUM_XBOX_CONTROLLERS] = {};
};
