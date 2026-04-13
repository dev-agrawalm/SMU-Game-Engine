#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"

class GameLauncher;

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
	void InitializeGameConfig();
	static bool StaticHandleQuitRequest(EventArgs& args);

private: //methods
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();
	void CreateGame();
	void DeleteGame();
	void InitialiseEngineSystems();
	void DeInitialiseSubSystems();

private: //members
	bool m_isQuitting = false;
};
