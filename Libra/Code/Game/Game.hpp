#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"

enum class GameState
{
	NONE,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_GAME_WON,
	GAME_STATE_GAME_OVER
};

class Entity;
class World;

//class representing an instance of our game
//it is owned by App
class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void ShutDown();
	void HandleQuitRequest();
	void LoadAssets();
	void CheckInputDeveloperCheats();
	void Update(float deltaSeconds);
	void UpdateCameras(float deltaSeconds);
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
	void SetGameState(GameState gameMode);
	void SetTimeScale(float timeScale);
	void SetWorldCamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration);
	void SetUICamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration);
	void SetWorldCameraPosition(Vec2 const& position);

	//getters
	bool AreEntitiesColliding(Entity* entityA, Entity* entityB) const;
private:
	void LoadAudio();
	void LoadTextures();

private: //methods
	//World Cam variables
	Camera	m_worldCamera;
	bool	m_shouldShakeWorldCam				= false;
	float	m_worldCamScreenShakeMagnitude		= 0.0f;
	float	m_worldCamScreenShakeDuration		= 0.0f;
	float	m_worldCamTimeSinceScreenShakeStart	= 0.0f;

	//UI Cam variables
	Camera	m_uiCamera;
	bool	m_shouldShakeUICam					= false;
	float	m_uiCamScreenShakeMagnitude			= 0.0f;
	float	m_uiCamScreenShakeDuration			= 0.0f;
	float	m_uiCamTimeSinceScreenShakeStart	= 0.0f;

	//Meta Game variables
	bool			m_isDebuggerCameraEnabled	= false;
	bool			m_runSingleStep				= false;
	bool			m_isPaused					= false;
	float			m_playerDeathIdleTime		= 3.0f;
	float			m_postPlayerDeathTimer		= 0.0f;
	float			m_timeScale					= 1.0f;
	GameState		m_gameState					= GameState::NONE;
	SoundPlaybackID	m_attractModeMusicId		= 0;
	SoundPlaybackID m_gameplayModeMusicId		= 0;
	SoundPlaybackID m_gameOverModeMusicId		= 0;
	SoundPlaybackID m_gameWonModeMusicId		= 0;

	//gameplay variables
	World*	m_world		= nullptr;
};
