#include "Engine/UI/UISystem.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "ThirdParty/ImGUI/imgui_impl_win32.h"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "ThirdParty/ImGUI/imgui_impl_dx11.h"
#include<windows.h>
#include "ThirdParty/ImGUI/implot.h"

UISystem::UISystem(UISystemConfig const& config)
	: m_config(config)
{

}


void UISystem::Startup()
{
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(m_config.m_window->GetWindowHandle());
	ID3D11Device* device = m_config.m_renderer->GetDevice();
	ID3D11DeviceContext* context = m_config.m_renderer->GetDeviceContext();
	ImGui_ImplDX11_Init(device, context);
}


void UISystem::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}


void UISystem::BeginFrame()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();
}


void UISystem::EndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
