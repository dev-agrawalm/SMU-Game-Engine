#include "Game/GameLauncher.hpp"
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
#include "Engine/Core/Clock.hpp"

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

SoundID g_gameSounds[NUM_SOUNDS] = {};
float g_gameSoundVolumes[NUM_SOUNDS] = {};

GameLauncher::GameLauncher()
{

}


GameLauncher::~GameLauncher()
{
}


void GameLauncher::Startup()
{
	DebugRenderSetVisible();
	LoadAssets();

	//m_gameClock = new Clock();
	g_rng = new RandomNumberGenerator();
	SetGameState(GameLauncherState::GAME_STATE_ATTRACT);

	m_uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	m_uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, m_uiCanvasSizeX, m_uiCanvasSizeY);

	m_launcherClock = new Clock();
	m_launcherBgMusicPlaybackId = g_audioSystem->StartSound(g_gameSounds[MUSIC_LAUNCHER_BG_MUSIC], true, g_gameSoundVolumes[MUSIC_LAUNCHER_BG_MUSIC]);
}


void GameLauncher::LoadAssets()
{
	//load audio
	g_gameSounds[SFX_TABLE_HIT]					= g_audioSystem->CreateOrGetSound("Data/Audio/Buzz1.wav");
	g_gameSounds[SFX_SCORED_A_GOAL]				= g_audioSystem->CreateOrGetSound("Data/Audio/Goal.mp3");
	g_gameSounds[SFX_PADDLE_HIT_PUCK]			= g_audioSystem->CreateOrGetSound("Data/Audio/PaddlePuckContact.mp3");
	g_gameSounds[SFX_PUCK_RESET]				= g_audioSystem->CreateOrGetSound("Data/Audio/PuckReset.mp3"); //to be fixed
	g_gameSounds[SFX_PUCK_SLIDE_ON_TABLE]		= g_audioSystem->CreateOrGetSound("Data/Audio/tableSlide3.wav");
	g_gameSounds[MUSIC_GAME_BG_MUSIC]			= g_audioSystem->CreateOrGetSound("Data/Audio/GameBGMusic.wav");
	g_gameSounds[MUSIC_LAUNCHER_BG_MUSIC]		= g_audioSystem->CreateOrGetSound("Data/Audio/GameBGMusic.wav"); //to find new clip

	g_gameSoundVolumes[SFX_TABLE_HIT]			= 0.9f;
	g_gameSoundVolumes[SFX_SCORED_A_GOAL]		= 2.0f;
	g_gameSoundVolumes[SFX_PADDLE_HIT_PUCK]		= 2.0f;
	g_gameSoundVolumes[SFX_PUCK_RESET]			= 0.0f;
	g_gameSoundVolumes[SFX_PUCK_SLIDE_ON_TABLE] = 1.0f;
	g_gameSoundVolumes[MUSIC_GAME_BG_MUSIC]		= 0.8f;
	g_gameSoundVolumes[MUSIC_LAUNCHER_BG_MUSIC] = 1.0f;

	//load textures
	g_theRenderer->CreateOrGetTexture("Data/Images/TestUV.png");
	g_theRenderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
}


void GameLauncher::ShutDown()
{
	switch (m_gameState)
	{
		case GameLauncherState::GAME_STATE_ATTRACT:		DeInitAttractMode();	break;
		case GameLauncherState::GAME_STATE_LAUNCHER:	DeInitLauncher();		break;
		case GameLauncherState::NONE:					//fallthrough
		default: break;
	}

	delete g_rng;
	g_rng = nullptr;

	delete m_launcherClock;
	m_launcherClock = nullptr;

	g_audioSystem->StopSound(m_launcherBgMusicPlaybackId);
}


void GameLauncher::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void GameLauncher::CheckInputDeveloperCheats()
{
// 	if (g_inputSystem->IsKeyPressed('O'))
// 	{
// 		m_gameClock->StepFrame();
// 	}
// 
// 	if (g_inputSystem->WasKeyJustPressed('P'))
// 	{
// 		m_gameClock->TogglePause();
// 	}

// 	if (g_inputSystem->IsKeyPressed('R'))
// 	{
// 		SetGameTimeScale(SLOW_TIME_SCALE);
// 	}
// 	else if (g_inputSystem->IsKeyPressed('T'))
// 	{
// 		SetGameTimeScale(1.0f);
// 	}
// 	else if (g_inputSystem->IsKeyPressed('Y'))
// 	{
// 		SetGameTimeScale(FAST_TIME_SCALE);
// 	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}


void GameLauncher::Update()
{
	CheckInputDeveloperCheats();
// 	float deltaSeconds = static_cast<float>(m_gameClock->GetFrameDeltaSeconds());
// 	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);

	//check input
	switch (m_gameState)
	{
		case GameLauncherState::GAME_STATE_ATTRACT:		CheckInputAttractMode();	break;
		case GameLauncherState::GAME_STATE_LAUNCHER:	CheckInputLauncher();		break;
		case GameLauncherState::NONE:					//fallthrough case
		default: break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameLauncherState::GAME_STATE_ATTRACT:		UpdateAttractMode();	break;
		case GameLauncherState::GAME_STATE_LAUNCHER:	UpdateLauncher();		break;
		case GameLauncherState::NONE:					//fallthrough case
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


void GameLauncher::Render()
{
	switch (m_gameState)
	{
		case GameLauncherState::GAME_STATE_ATTRACT:		RenderAttractMode();	break;
		case GameLauncherState::GAME_STATE_LAUNCHER:	RenderLauncher();		break;
		case GameLauncherState::NONE: //fallthrough case
		default: break;
	}
}


void GameLauncher::InitAttractMode()
{
	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = false;
	config.m_isRelative = false;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);
}


void GameLauncher::DeInitAttractMode()
{
	g_inputSystem->PopMouseConfigOfPriority(0);
}


void GameLauncher::CheckInputAttractMode()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_START))
	{
		SetGameState(GameLauncherState::GAME_STATE_LAUNCHER);
	}
}


void GameLauncher::UpdateAttractMode()
{
}


void GameLauncher::RenderAttractMode()
{
 	g_theRenderer->ClearScreen(Rgba8::BLACK);

// 	std::vector<Vertex_PCU> text;
// 	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
// 	AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
// 	font->AddVertsForTextInAABB2(text, textBox, 800.0f, "ATTRACT", Rgba8::WHITE);

	static Vec2 uiOrthoBottomLeft = Vec2::ZERO;
	static Vec2 uiOrthoTopRight = Vec2(m_uiCanvasSizeX, m_uiCanvasSizeY);
	static AABB2 quad = AABB2(uiOrthoBottomLeft, uiOrthoTopRight);
	static Vec3 screenTextureVertPositions[4] =
	{
		Vec3(quad.m_mins.x, quad.m_mins.y),	//BL
		Vec3(quad.m_maxs.x, quad.m_mins.y),	//BR
		Vec3(quad.m_mins.x, quad.m_maxs.y),	//TL
		Vec3(quad.m_maxs.x, quad.m_maxs.y)	//TR
	};
	static Vec2 screenTextureVertUVs[4] =
	{
		Vec2(0.0f, 0.0f),	//BL
		Vec2(1.0f, 0.0f),	//BR
		Vec2(0.0f, 1.0f),	//TL
		Vec2(1.0f, 1.0f)	//TR
	};
	static Vertex_PCU screenTextureVerts[6] =
	{
		Vertex_PCU(screenTextureVertPositions[0], Rgba8::WHITE, screenTextureVertUVs[0]),
		Vertex_PCU(screenTextureVertPositions[1], Rgba8::WHITE, screenTextureVertUVs[1]),
		Vertex_PCU(screenTextureVertPositions[2], Rgba8::WHITE, screenTextureVertUVs[2]),

		Vertex_PCU(screenTextureVertPositions[1], Rgba8::WHITE, screenTextureVertUVs[1]),
		Vertex_PCU(screenTextureVertPositions[3], Rgba8::WHITE, screenTextureVertUVs[3]),
		Vertex_PCU(screenTextureVertPositions[2], Rgba8::WHITE, screenTextureVertUVs[2])
	};

	g_theRenderer->BeginCamera(m_uiCamera);
	static Texture* attractScreenTexture = g_theRenderer->CreateOrGetTexture("Data/Images/AttractScreen.jpg");
	g_theRenderer->BindTexture(0, attractScreenTexture);
	g_theRenderer->DrawVertexArray(6, screenTextureVerts);
	//g_theRenderer->DrawVertexArray((int) text.size(), text.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void GameLauncher::InitLauncher()
{
	//m_gameClock->Unpause();

	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = false;
	config.m_isRelative = false;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);
}


void GameLauncher::DeInitLauncher()
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


void GameLauncher::CheckInputLauncher()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		if (m_server == nullptr || !m_server->IsServerActive())
			SetGameState(GameLauncherState::GAME_STATE_ATTRACT);
	}

	if (g_inputSystem->WasKeyJustPressed('M') && m_server == nullptr)
	{
		IPv4Address authServer = IPv4Address(51110, 127, 0, 0, 1);
		m_server = new GameServer(authServer);
		m_server->Startup();
	}

	if (g_inputSystem->WasKeyJustPressed('N') && m_server == nullptr)
	{
		IPv4Address remoteServer = IPv4Address(51111, 127, 0, 0, 1);
		IPv4Address authServer = IPv4Address(51110, 127, 0, 0, 1);
		m_server = new GameServer(remoteServer, authServer);
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


void GameLauncher::UpdateLauncher()
{
	if (m_server && m_server->IsServerActive())
	{
		m_server->BeginFrame();
		m_server->Update();
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
				Strings ipElements = SplitStringOnDelimiter(s_selectedIP, '.');
				if (ipElements.size() > 0 && _stricmp(m_serverPortStr, "") != 0)
				{
					int b1 = atoi(ipElements[0].c_str());
					int b2 = atoi(ipElements[1].c_str());
					int b3 = atoi(ipElements[2].c_str());
					int b4 = atoi(ipElements[3].c_str());
					int port = atoi(m_serverPortStr);
					IPv4Address serverAddr = IPv4Address((uint16_t) port, (uint8_t) b1, (uint8_t) b2, (uint8_t) b3, (uint8_t) b4);
					if (m_server)
					{
						delete m_server;
						m_server = nullptr;
					}
					
					m_server = new GameServer(serverAddr);
					m_server->Startup();
					g_console->AddLine(DevConsole::MINOR_INFO, Stringf("Auth Server started: %s", m_server->GetDescription().c_str()));
				}
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

					if (m_server)
					{
						delete m_server;
						m_server = nullptr;
					}

					m_server = new GameServer(clientAddr, serverAddr);
					m_server->Startup();
					g_console->AddLine(DevConsole::MINOR_INFO, Stringf("Remote Server started: %s", m_server->GetDescription().c_str()));
				}
			}
		}

		ImGui::Separator();
		ImGui::InputText("Server IPv4 Address", m_serverIpAddrStr, 16);
		ImGui::InputText("Server Port", m_serverPortStr, 16);

		ImGui::End();
	}

	float deltaSeconds = static_cast<float>(m_launcherClock->GetFrameDeltaSeconds());
	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);
	m_otherPlayerQuitTextFadeoutDuration -= deltaSeconds;
}


void GameLauncher::RenderLauncher()
{
	if (m_server == nullptr || !m_server->IsServerActive())
	{
		g_theRenderer->ClearScreen(Rgba8::BLACK);
		std::vector<Vertex_PCU> textVerts;
		BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
		font->AddVertsForTextInAABB2(textVerts, textBox, 800.0f, "LAUNCHER", Rgba8::WHITE);

		if (m_otherPlayerQuitTextFadeoutDuration > 0.0f)
		{
			AABB2 subtitleTextBox = AABB2(Vec2(900.0f, 100.0f), 1800.0f, 200.0f);
			Rgba8 textColor = Rgba8::YELLOW;
			textColor.a = FloatToByte(m_otherPlayerQuitTextFadeoutDuration);
			font->AddVertsForTextInAABB2(textVerts, subtitleTextBox, 150.0f, "The other player quit the game.", textColor);
		}

		g_theRenderer->BeginCamera(m_uiCamera);
		Texture const& fontTexture = font->GetTexture();
		g_theRenderer->BindTexture(0, &fontTexture);
		g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
		g_theRenderer->EndCamera(m_uiCamera);
	}
}


void GameLauncher::SetGameState(GameLauncherState gameState)
{
	switch (m_gameState)
	{
		case GameLauncherState::GAME_STATE_ATTRACT:		DeInitAttractMode();	break;
		case GameLauncherState::GAME_STATE_LAUNCHER:	DeInitLauncher();		break;
		case GameLauncherState::NONE:					//fall through
		default: break;
	}
	m_gameState = gameState;
	switch (m_gameState)
	{
		case GameLauncherState::GAME_STATE_ATTRACT:		InitAttractMode();	break;
		case GameLauncherState::GAME_STATE_LAUNCHER:	InitLauncher();		break;
		case GameLauncherState::NONE:					//fall through
		default: break;
	}
}


// void GameLauncher::SetGameTimeScale(float scale)
// {
// 	m_gameClock->SetTimeScale((double) scale);
// }
// 
// 
// Clock* GameLauncher::GetClock() const
// {
// 	return m_gameClock;
// }


void GameLauncher::RenderGrid() const
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
