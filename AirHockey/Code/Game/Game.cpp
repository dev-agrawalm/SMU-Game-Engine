#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "Game/GameServer.hpp"

#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

RandomNumberGenerator* g_rng = nullptr;

constexpr float SLOW_TIME_SCALE = 0.2f;
constexpr float FAST_TIME_SCALE = 5.0f;

Game::Game()
{

}


Game::~Game()
{
}


void Game::Startup()
{
	DebugRenderSetVisible();
	LoadAssets();

	m_gameClock = new Clock();
	g_rng = new RandomNumberGenerator();
	SetGameState(GameState::GAME_STATE_ATTRACT);

	m_uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	m_uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, m_uiCanvasSizeX, m_uiCanvasSizeY);
}


void Game::LoadAssets()
{
	//load audio
	g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");

	//load textures
	g_theRenderer->CreateOrGetTexture("Data/Images/TestUV.png");
	g_theRenderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
}


void Game::ShutDown()
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		DeInitAttractMode();	break;
		case GameState::GAME_STATE_LAUNCHER:	DeInitLauncher();		break;
		case GameState::NONE:					//fallthrough
		default: break;
	}

	delete g_rng;
	g_rng = nullptr;
}


void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void Game::CheckInputDeveloperCheats()
{
	if (g_inputSystem->IsKeyPressed('O'))
	{
		m_gameClock->StepFrame();
	}

	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
	}

	if (g_inputSystem->IsKeyPressed('R'))
	{
		SetGameTimeScale(SLOW_TIME_SCALE);
	}
	else if (g_inputSystem->IsKeyPressed('T'))
	{
		SetGameTimeScale(1.0f);
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		SetGameTimeScale(FAST_TIME_SCALE);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}


void Game::Update()
{
	CheckInputDeveloperCheats();
	float deltaSeconds = static_cast<float>(m_gameClock->GetFrameDeltaSeconds());
	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);

	//check input
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		CheckInputAttractMode();	break;
		case GameState::GAME_STATE_LAUNCHER:	CheckInputLauncher();		break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		UpdateAttractMode(deltaSeconds);	break;
		case GameState::GAME_STATE_LAUNCHER:	UpdateLauncher(deltaSeconds);		break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}
}


// void Game::UpdateCameras()
// {
// 	if (m_player && m_freeCamMode)
// 	{
// 		Mat44 persProjMat = Mat44::CreatePerspectiveProjectionMatrix(60.0f, g_window->GetWindowConfig().m_aspectRatio, 0.1f, 100.0f);
// 		m_worldCamera.SetProjectionMatrix(persProjMat);
// 
// 		Mat44 camModelMat = m_player->GetModelMatrix();
// 		m_worldCamera.SetCameraModelMatrix(camModelMat);
// 	}
// 	else
// 	{
// 		m_worldCamera.SetCameraModelMatrix(m_camModelMatrix);
// 	}
// }


void Game::Render()
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		RenderAttractMode();	break;
		case GameState::GAME_STATE_LAUNCHER:	RenderLaunder();		break;
		case GameState::NONE: //fallthrough case
		default: break;
	}
}


void Game::InitAttractMode()
{
	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = true;
	config.m_isRelative = false;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);
}


void Game::DeInitAttractMode()
{
	g_inputSystem->PopMouseConfigOfPriority(0);
}


void Game::CheckInputAttractMode()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_START))
	{
		SetGameState(GameState::GAME_STATE_LAUNCHER);
	}
}


void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Game::RenderAttractMode()
{
 	g_theRenderer->ClearScreen(Rgba8::CYAN);

	std::vector<Vertex_PCU> text;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(text, textBox, 800.0f, "ATTRACT", Rgba8::WHITE);

	g_theRenderer->BeginCamera(m_uiCamera);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) text.size(), text.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitLauncher()
{
	m_gameClock->Unpause();

	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = true;
	config.m_isRelative = false;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);
}


void Game::DeInitLauncher()
{
	g_inputSystem->PopMouseConfigOfPriority(0);

	if (m_server)
	{
		if (m_server->IsServerActive())
			m_server->Shutdown();

		delete m_server;
		m_server = nullptr;
	}
}


void Game::CheckInputLauncher()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}

	if (g_inputSystem->WasKeyJustPressed('M') && m_server == nullptr)
	{
		IPv4Address bindingAddr = IPv4Address(51110, 127, 0, 0, 1);
		m_server = new GameServer(bindingAddr);
		m_server->Startup();
	}

	if (g_inputSystem->WasKeyJustPressed('S'))
	{
		m_showServerWindow = !m_showServerWindow;
	}

	if (g_inputSystem->WasKeyJustPressed('C'))
	{
		m_showClientWindow = !m_showClientWindow;
	}
}


void Game::UpdateLauncher(float deltaSeconds)
{
	if (m_server)
	{
		m_server->BeginFrame();
		m_server->Update(deltaSeconds);
		m_server->Render();
		m_server->EndFrame();
	}

	if (m_showServerWindow)
	{
		ImGui::Begin("Server Window");
		ImGui::Text("Network Interface");
		if (ImGui::Button("Refresh"))
		{
			PMIB_IPADDRTABLE pIPAddrTable;
			DWORD dwSize = 0;
			DWORD dwRetVal = 0;
			IN_ADDR IPAddr;

			pIPAddrTable = (MIB_IPADDRTABLE*) MALLOC(sizeof(MIB_IPADDRTABLE));
			if (pIPAddrTable)
			{
				// Make an initial call to GetIpAddrTable to get the
				// necessary size into the dwSize variable
				if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
					ERROR_INSUFFICIENT_BUFFER)
				{
					FREE(pIPAddrTable);
					pIPAddrTable = (MIB_IPADDRTABLE*) MALLOC(dwSize);

				}
				if (pIPAddrTable == NULL)
				{
					ERROR_AND_DIE("Failed to allocate memory for ip address table");
				}
			}
			if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR)
			{
				ERROR_AND_DIE(Stringf("Unable to get IP address table data. Error Code: %i", dwRetVal));
			}

			if (pIPAddrTable)
			{
				for (int addrIndex = 0; addrIndex < (int) pIPAddrTable->dwNumEntries; addrIndex++)
				{
					std::string ipAddr;
					IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[addrIndex].dwAddr;
					ipAddr.append(Stringf("%i.%i.%i.%i", IPAddr.S_un.S_un_b.s_b1, IPAddr.S_un.S_un_b.s_b2, IPAddr.S_un.S_un_b.s_b3, IPAddr.S_un.S_un_b.s_b4));
					m_ipAddrs.push_back(ipAddr);
				}

				FREE(pIPAddrTable);
				pIPAddrTable = nullptr;
			}
		}
		static std::string s_selectedIP = "";
		if (ImGui::BeginListBox("Network Interfaces"))
		{
			static int currentIndex = 0;
			for (int ipIndex = 0; ipIndex < (int) m_ipAddrs.size(); ipIndex++)
			{
				bool is_selected = (currentIndex == ipIndex);
				if (ImGui::Selectable(m_ipAddrs[ipIndex].c_str(), is_selected))
				{
					currentIndex = ipIndex;
					s_selectedIP = m_ipAddrs[ipIndex];
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::Separator();
		if (s_selectedIP.size() > 0)
		{
			memcpy(m_serverIpAddrStr, s_selectedIP.c_str(), s_selectedIP.size());
			m_serverIpAddrStr[s_selectedIP.size()] = '\0';
		}
		ImGui::InputText("Bind IPv4 Address", m_serverIpAddrStr, 16);
		ImGui::InputText("Bind Port", m_serverPortStr, 16);
		if (m_server && m_server->IsServerActive())
		{
			if (ImGui::Button("Disconnect"))
			{
				m_server->Shutdown();
				delete m_server;
				m_server = nullptr;
			}
		}
		else
		{
			if (ImGui::Button("Connect"))
			{
// 				if (m_server)
// 				{
// 					m_server->Shutdown();
// 					delete m_server;
// 					m_server = nullptr;
// 				}

				Strings ipElements = SplitStringOnDelimiter(s_selectedIP, '.');
				if (ipElements.size() > 0 && _stricmp(m_serverPortStr, "") != 0)
				{
					int b1 = atoi(ipElements[0].c_str());
					int b2 = atoi(ipElements[1].c_str());
					int b3 = atoi(ipElements[2].c_str());
					int b4 = atoi(ipElements[3].c_str());
					int port = atoi(m_serverPortStr);
					IPv4Address serverAddr = IPv4Address((uint16_t) port, (uint8_t) b1, (uint8_t) b2, (uint8_t) b3, (uint8_t) b4);
					m_server = new GameServer(serverAddr);
					m_server->Startup();
				}
			}
		}

		if (m_server)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode(m_server->GetDescription().c_str()))
			{
// 				std::vector<Client*> const& clients = m_server->GetClients();
// 				for (int i = 0; i < (int) clients.size(); i++)
// 				{
// 					ImGui::SetNextItemOpen(true, ImGuiCond_Once);
// 					if (ImGui::TreeNode(clients[i]->GetDescription().c_str()))
// 					{
// 						ImGui::TreePop();
// 					}
// 				}				
				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	if (m_showClientWindow)
	{
		ImGui::Begin("Client Window");
		ImGui::Text("Adapters");
		if (ImGui::Button("Refresh"))
		{
			PMIB_IPADDRTABLE pIPAddrTable;
			DWORD dwSize = 0;
			DWORD dwRetVal = 0;
			IN_ADDR IPAddr;

			pIPAddrTable = (MIB_IPADDRTABLE*) MALLOC(sizeof(MIB_IPADDRTABLE));
			if (pIPAddrTable)
			{
				if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
					ERROR_INSUFFICIENT_BUFFER)
				{
					FREE(pIPAddrTable);
					pIPAddrTable = (MIB_IPADDRTABLE*) MALLOC(dwSize);

				}
				if (pIPAddrTable == NULL)
				{
					ERROR_AND_DIE("Failed to allocate memory for ip address table");
				}
			}
			if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR)
			{
				ERROR_AND_DIE(Stringf("Unable to get IP address table data. Error Code: %i", dwRetVal));
			}

			if (pIPAddrTable)
			{
				for (int addrIndex = 0; addrIndex < (int) pIPAddrTable->dwNumEntries; addrIndex++)
				{
					std::string ipAddr;
					IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[addrIndex].dwAddr;
					ipAddr.append(Stringf("%i.%i.%i.%i", IPAddr.S_un.S_un_b.s_b1, IPAddr.S_un.S_un_b.s_b2, IPAddr.S_un.S_un_b.s_b3, IPAddr.S_un.S_un_b.s_b4));
					m_ipAddrs.push_back(ipAddr);
				}

				FREE(pIPAddrTable);
				pIPAddrTable = nullptr;
			}
		}
		static std::string s_selectedIP = "";
		if (ImGui::BeginListBox("Adapters"))
		{
			static int currentIndex = 0;
			for (int ipIndex = 0; ipIndex < (int) m_ipAddrs.size(); ipIndex++)
			{
				bool is_selected = (currentIndex == ipIndex);
				if (ImGui::Selectable(m_ipAddrs[ipIndex].c_str(), is_selected))
				{
					currentIndex = ipIndex;
					s_selectedIP = m_ipAddrs[ipIndex];
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}
		ImGui::Separator();
		if (s_selectedIP.size() > 0)
		{
			memcpy(m_clientIpAddrStr, s_selectedIP.c_str(), s_selectedIP.size());
			m_clientIpAddrStr[s_selectedIP.size()] = '\0';
		}
		ImGui::InputText("Client IPv4 Address", m_clientIpAddrStr, 16);
		ImGui::InputText("Client Port", m_clientPortStr, 16);
		if (m_server && m_server->IsServerActive())
		{
			if (ImGui::Button("Disconnect"))
			{
				m_server->Shutdown();
 				delete m_server;
 				m_server = nullptr;
			}
		}
		else
		{
			if (ImGui::Button("Connect"))
			{
				std::string serverIpStr = std::string(m_serverIpAddrStr);
				Strings clientIpElements = SplitStringOnDelimiter(s_selectedIP, '.');
				Strings serverIpElements = SplitStringOnDelimiter(serverIpStr, '.');
				if (clientIpElements.size() > 0 && _stricmp(m_clientPortStr, "") != 0 && serverIpElements.size() > 0 && _stricmp(m_serverPortStr, "") != 0)
				{
					int b1 = atoi(clientIpElements[0].c_str());
					int b2 = atoi(clientIpElements[1].c_str());
					int b3 = atoi(clientIpElements[2].c_str());
					int b4 = atoi(clientIpElements[3].c_str());
					int clientPort = atoi(m_clientPortStr);
					IPv4Address clientAddr = IPv4Address((uint16_t) clientPort, (uint8_t) b1, (uint8_t) b2, (uint8_t) b3, (uint8_t) b4);

					b1 = atoi(serverIpElements[0].c_str());
					b2 = atoi(serverIpElements[1].c_str());
					b3 = atoi(serverIpElements[2].c_str());
					b4 = atoi(serverIpElements[3].c_str());
					int serverPort = atoi(m_serverPortStr);
					IPv4Address serverAddr = IPv4Address((uint16_t) serverPort, (uint8_t) b1, (uint8_t) b2, (uint8_t) b3, (uint8_t) b4);

					m_server = new GameServer(clientAddr, serverAddr);
					m_server->Startup();
				}
			}
		}

		ImGui::Separator();
		ImGui::InputText("Server IPv4 Address", m_serverIpAddrStr, 16);
		ImGui::InputText("Server Port", m_serverPortStr, 16);

		if (m_server)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode(m_server->GetDescription().c_str()))
			{
// 				std::vector<Client*> const& clients = m_server->GetClients();
// 				for (int i = 0; i < (int) clients.size(); i++)
// 				{
// 					ImGui::SetNextItemOpen(true, ImGuiCond_Once);
// 					if (ImGui::TreeNode(clients[i]->GetDescription().c_str()))
// 					{
// 						ImGui::TreePop();
// 					}
// 				}				
				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

// 	Vec3 mouseWorldPos = GetScreenToWorldMousePos(m_camModelMatrix);
// 	DebugAddWorldSolidSphere(mouseWorldPos, 0.5f, 0.0f, Rgba8::RED, DebugRenderMode::XRAY);

// 	if (m_debugRender)
// 	{
// 		Vec3 playerPos = m_player->GetPosition();
// 		DebugAddScreenMessage(Stringf("Player Pos: x = %f  y = %f  z = %f", playerPos.x, playerPos.y, playerPos.z), 0.0f, 20.0f);
// 	}

// 	static bool s_shouldInputCatchup = true;
// 	AABB2 uiOrtho = AABB2(Vec2::ZERO, Vec2(m_uiCanvasSizeX, m_uiCanvasSizeY));
// 	AABB2 windowOrtho = AABB2(Vec2::ZERO, g_window->GetWindowDims());
// 	if (g_inputSystem->WasMouseButtonJustPressed(0))
// 	{
// 		m_inputData.m_uiDragPositions.clear();
// 		m_inputData.m_uiDragPositions.push_back(g_inputSystem->GetCurrentMouseWorldPosition2D(windowOrtho, uiOrtho));
// 		m_inputData.m_uiDragStart = m_inputData.m_uiDragPositions.back();
// 		m_inputData.m_uiDragEnd = m_inputData.m_uiDragPositions.back();

// 		m_inputData.m_worldDragPositions.clear();
// 		m_inputData.m_worldDragPositions.push_back(GetCurrentMousePos_ScreenToWorld(m_camModelMatrix));
// 		m_inputData.m_worldDragStart = m_inputData.m_worldDragPositions.back();
// 		m_inputData.m_worldDragEnd = m_inputData.m_worldDragPositions.back();
// 	}

// 	static float s_inputRegistrationThreshold = 0.1f;
// 	if (g_inputSystem->IsMouseButtonPressed(0))
// 	{
// 		Vec2 latestUIPosition = m_inputData.m_uiDragEnd;
// 		Vec2 currentUIPosition = g_inputSystem->GetCurrentMouseWorldPosition2D(windowOrtho, uiOrtho);
// 		if ((latestUIPosition - currentUIPosition).GetLength() > s_inputRegistrationThreshold)
// 		{
// 			m_inputData.m_uiDragPositions.push_back(g_inputSystem->GetCurrentMouseWorldPosition2D(windowOrtho, uiOrtho));
// 			m_inputData.m_uiDragEnd = m_inputData.m_uiDragPositions.back();
// 
// 			m_inputData.m_worldDragPositions.push_back(GetCurrentMousePos_ScreenToWorld(m_camModelMatrix));
// 			m_inputData.m_worldDragEnd = m_inputData.m_worldDragPositions.back();
// 
// 			float movementLength = 0.0f;
// 			for (int i = 0; i < (int) m_inputData.m_uiDragPositions.size() - 1; i++)
// 			{
// 				Vec2 pt1 = m_inputData.m_uiDragPositions[i];
// 				Vec2 pt2 = m_inputData.m_uiDragPositions[i+1];
// 				movementLength += (pt2 - pt1).GetLength();
// 			}
// 			float lengthAtMaxSpeed = g_gameConfigBlackboard.GetValue("lengthAtMaxSpeed", 100.0f);
// 			m_movementCatchupT = GetFraction(movementLength, 0.0f, lengthAtMaxSpeed);
// 		}
//	}

// 	static float s_inputCatchupStartSpeed = g_gameConfigBlackboard.GetValue("inputCatchUpStartSpeed", 10.0f);
// 	static float s_inputCatchupMaxSpeed = g_gameConfigBlackboard.GetValue("inputCatchUpMaxSpeed", 10.0f);
// 	float catchUpSpeed = 0.0f;
// 	if (s_shouldInputCatchup)
// 	{
// 		if ((int) m_inputData.m_uiDragPositions.size() > 0)
// 		{
// 			Vec2 uiPtToMove = m_inputData.m_uiDragStart;
// 			Vec2 nextUIPt = m_inputData.m_uiDragPositions[0];
// 			Vec2 uiLineVec = nextUIPt - uiPtToMove;
// 			float uiLineLength = uiLineVec.GetLength();
// 			if (uiLineLength > 0.0f)
// 			{
// 				Vec2 fwdNormal = uiLineVec.GetNormalized();
// 				catchUpSpeed = RangeMap_SmoothStart2(m_movementCatchupT, 0.0f, 1.0f, s_inputCatchupStartSpeed, s_inputCatchupMaxSpeed);
// 				uiPtToMove += fwdNormal * catchUpSpeed * deltaSeconds;
// 				Vec2 newFwdNormal = (nextUIPt - uiPtToMove).GetNormalized();
// 				if (DotProduct2D(newFwdNormal, fwdNormal) < 0.0f)
// 				{
// 					m_inputData.m_uiDragStart = nextUIPt;
// 					m_inputData.m_uiDragPositions.pop_front();
// 				}
// 				else
// 				{
// 					m_inputData.m_uiDragStart = uiPtToMove;
// 				}
// 			}
// 			else
// 			{
// 				m_inputData.m_uiDragPositions.pop_front();
// 			}
// 		}

// 		if((int) m_inputData.m_worldDragPositions.size() > 0)
// 		{
// 			Vec3 worldPtToMove = m_inputData.m_worldDragStart;
// 			Vec3 nextWorldPt = m_inputData.m_worldDragPositions[0];
// 			Vec3 worldLineVec = nextWorldPt - worldPtToMove;
// 			float worldLineLength = worldLineVec.GetLength();
// 			if (worldLineLength > 0.0f)
// 			{
// 				Vec3 fwdNormal = worldLineVec.GetNormalized();
// 				catchUpSpeed = RangeMap_SmoothStart2(m_movementCatchupT, 0.0f, 1.0f, s_inputCatchupStartSpeed, s_inputCatchupMaxSpeed);
// 				worldPtToMove += fwdNormal * catchUpSpeed * deltaSeconds;
// 				Vec3 newFwdNormal = (nextWorldPt - worldPtToMove).GetNormalized();
// 				if (DotProduct3D(newFwdNormal, fwdNormal) < 0.0f)
// 				{
// 					m_inputData.m_worldDragStart = nextWorldPt;
// 					m_inputData.m_worldDragPositions.pop_front();
// 				}
// 				else
// 				{
// 					m_inputData.m_worldDragStart = worldPtToMove;
// 				}
// 			}
// 			else
// 			{
// 				m_inputData.m_worldDragPositions.pop_front();
// 			}
// 		}
// 	}

// 	if (m_inputData.m_uiDragPositions.size() <= 0)
// 	{
// 		m_movementCatchupT = 0.0f;
// 	}

// 	if (m_inputData.m_worldDragPositions.size() <= 0)
// 	{
// 		m_movementCatchupT = 0.0f;
// 	}

//	m_player->SetPosition(m_inputData.m_worldDragStart);

// 	if (g_inputSystem->WasKeyJustPressed('L'))
// 	{
// 		s_inputCatchupStartSpeed *= 1.2f;
// 	}
// 
// 	if (g_inputSystem->WasKeyJustPressed('K'))
// 	{
// 		s_inputCatchupStartSpeed *= 0.8f;
// 	}
// 
// 	if (g_inputSystem->WasKeyJustPressed('M'))
// 	{
// 		s_inputRegistrationThreshold *= 1.2f;
// 	}
// 
// 	if (g_inputSystem->WasKeyJustPressed('N'))
// 	{
// 		s_inputRegistrationThreshold *= 0.8f;
// 	}
}


void Game::RenderLaunder()
{

	if (m_server == nullptr || !m_server->IsServerActive())
	{
		g_theRenderer->ClearScreen(Rgba8::CYAN);
		std::vector<Vertex_PCU> text;
		BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
		font->AddVertsForTextInAABB2(text, textBox, 800.0f, "LAUNCHER", Rgba8::MAGENTA);

		g_theRenderer->BeginCamera(m_uiCamera);
		Texture const& fontTexture = font->GetTexture();
		g_theRenderer->BindTexture(0, &fontTexture);
		g_theRenderer->DrawVertexArray((int) text.size(), text.data());
		g_theRenderer->EndCamera(m_uiCamera);
	}
	else
	{
// 		if (m_server)
// 		{
// 			m_server->Render();
// 		}
	}

//	g_theRenderer->BeginCamera(m_uiCamera);
//	{
//		g_theRenderer->SetDepthOptions(DepthTest::ALWAYS, false);

//		int numPts = (int) m_inputData.m_uiDragPositions.size();
// 		for (int i = 0; i < (numPts - 1); i++)
// 		{
// 			Vec2 pt1 = m_inputData.m_uiDragPositions[i];
// 			Vec2 pt2 = m_inputData.m_uiDragPositions[i+1];
// 			DrawLine(pt1, pt2 - pt1, 10.0f, Rgba8::BLUE);
// 		}

// 		if(m_debugRender)
// 		{
// 			for (int i = 0; i < numPts; i++)
// 			{
// 				Vec2 pt = m_inputData.m_uiDragPositions[i];
// 				DrawRing(pt, 10.0f, 5.0f, Rgba8::GREEN);
// 			}
// 		}
		//DrawRing(m_inputData.m_uiDragStart, 10.0f, 5.0f, Rgba8::MAGENTA);
		//DrawRing(m_inputData.m_uiDragEnd, 10.0f, 5.0f, Rgba8::RED);

		//DebugAddWorldSolidSphere()
//	}
//	g_theRenderer->EndCamera(m_uiCamera);

//	DebugRenderWorldToCamera(m_worldCamera);

}


void Game::SetGameState(GameState gameState)
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		DeInitAttractMode();	break;
		case GameState::GAME_STATE_LAUNCHER:	DeInitLauncher();		break;
		case GameState::NONE:					//fall through
		default: break;
	}
	m_gameState = gameState;
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		InitAttractMode();	break;
		case GameState::GAME_STATE_LAUNCHER:	InitLauncher();		break;
		case GameState::NONE:					//fall through
		default: break;
	}
}


void Game::SetGameTimeScale(float scale)
{
	m_gameClock->SetTimeScale((double) scale);
}


Clock* Game::GetClock() const
{
	return m_gameClock;
}


void Game::RenderGrid() const
{
	float lineLength = 1000.0f;
	float otherDims = 0.05f;

	std::vector<Vertex_PCU> gridVerts;

	int camGridX = 0;
	int camGridY = 0;
	for (int lineIndex = -100; lineIndex < 100; lineIndex++)
	{
		float xAxisGridPos = (float) (camGridX + lineIndex);
		float yAxisGridPos = (float) (camGridY + lineIndex);
		float xLineLength = abs(yAxisGridPos) + lineLength;
		float yLineLength = abs(xAxisGridPos) + lineLength;
		AABB3 lineAlongXAxis = AABB3(Vec3(xAxisGridPos, 0.0f, 0.0f), otherDims, xLineLength, otherDims);
		AABB3 lineAlongYAxis = AABB3(Vec3(0.0f, yAxisGridPos, 0.0f), yLineLength, otherDims, otherDims);

		if (xAxisGridPos == 0)
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongXAxis, Rgba8::WHITE);
		}
		else
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongXAxis, Rgba8::RED);
		}

		if (yAxisGridPos == 0)
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongYAxis, Rgba8::WHITE);
		}
		else
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongYAxis, Rgba8::GREEN);
		}
	}


	g_theRenderer->SetCullMode(CullMode::NONE);
	g_theRenderer->DrawVertexArray((int) gridVerts.size(), gridVerts.data());
}
