#pragma once
#include "Engine/Math/Vec2.hpp"

class Renderer;
class Window;

struct UISystemConfig
{
	Renderer* m_renderer = nullptr;
	Window* m_window = nullptr;
};

class UISystem
{
public:
	UISystem(UISystemConfig const& config);
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

private:
	UISystemConfig m_config;
};
