#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"

class Game;

constexpr float FRAME_TIME_60_FPS = (1.0f / 60.0f);
constexpr float FRAME_TIME_10_FPS = (1.0f / 10.0f);

class App
{
public: //methods
	App();
	~App();
	void Startup();
	void ShutDown();
	void RunFrame();
	bool IsQuitting() const;
	void CheckInput();
	bool HandleQuitRequested();

private: //methods
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();
	void CreateGame();
	void DeleteGame();
	void InitialiseEngineSystems();
	void DeInitialiseSubSystems();
	void InitializeGameConfig();

private: //members
	bool m_isQuitting = false;
	float m_previousFrameStartTime = 0.0f;
	float m_currentFrameStartTime = 0.0f;
};