#include "Game/GameServer.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameLauncher.hpp"
#include "ThirdParty/ImGUI/implot.h"
#include "Engine/Networking/Client.hpp"
#include "Game/PlayerClient.hpp"
#include "Engine/Networking/UDPEndpoint.hpp"
#include "Engine/Networking/NetworkSystem.hpp"
#include "Engine/Networking/UDPDatagram.hpp"
#include "Game/NetworkPacket.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

constexpr double SLOW_TIME_SCALE = 0.2;
constexpr double FAST_TIME_SCALE = 5.0;

Rgba8 g_neonOrange = Rgba8(249, 87, 56, 255);
Rgba8 g_neonPink = Rgba8(234, 122, 244, 255);
Rgba8 g_neonCyan = Rgba8(59, 244, 251, 255);

struct NeonGlowShaderConstants
{
	Mat44 colorTransformationMat;
	float normalizedBlurRadius = 0.011f;
	float normalizedPerStepDistance = 0.0f;
	int blurNumOutwardSteps = 18;
	float blurThetaDeviationPerStep = 2.0f;
	float blurThetaDeviationWithinStep = 7.0f;
	float time = 0.0f;

//	int padding0;
	int padding1;
	int padding2;
};

NeonGlowShaderConstants g_neonGlowShaderConstants = {};

GameServer::GameServer(IPv4Address const& authServerAddr)
	:Server(authServerAddr)
	, m_isRemote(false)
	, m_authServerAddr(authServerAddr)
{
	m_numSamples = 200;
	m_windowSize = 50;
	m_exponentialFilter = 0.1f;
	m_numBins = 10;

	InitialiseBuffer(m_frames);
	InitialiseBuffer(m_frameTimes);
	InitialiseBuffer(m_simpleWindowedAvgs);
	InitialiseBuffer(m_recursiveWindowedAvgs);
	InitialiseBuffer(m_exponentialAvgs);

	static const float saturationRCoeff = 0.3086f;
	static const float saturationGCoeff = 0.6094f;
	static const float saturationBCoeff = 0.0820f;
	static float brightness = 2.151f;
	static float saturation = 1.075f;
	static const float brightnessMatValues[16] = {brightness, 0.0f, 0.0f, 0.0f,
													0.0f, brightness, 0.0f, 0.0f,
													0.0f, 0.0f, brightness, 0.0f,
													0.0f, 0.0f, 0.0f, 1.0f};
	static const Mat44 brightnessMat = Mat44(brightnessMatValues);

	static float a = (1.0f - saturation) * saturationRCoeff + saturation;
	static float b = (1.0f - saturation) * saturationRCoeff;
	static float c = (1.0f - saturation) * saturationRCoeff;
	static float d = (1.0f - saturation) * saturationGCoeff;
	static float e = (1.0f - saturation) * saturationGCoeff + saturation;
	static float f = (1.0f - saturation) * saturationGCoeff;
	static float g = (1.0f - saturation) * saturationBCoeff;
	static float h = (1.0f - saturation) * saturationBCoeff;
	static float i = (1.0f - saturation) * saturationBCoeff + saturation;
	static const float saturationMatValues[16] = {a, d, g, 0.0f,
											b, e, h, 0.0f,
											c, f, i, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f};
	const Mat44 saturationMat = Mat44(saturationMatValues);
	Mat44 transformMatrix = saturationMat;
	transformMatrix.Append(brightnessMat);
	g_neonGlowShaderConstants.colorTransformationMat = transformMatrix;
}


GameServer::GameServer(IPv4Address const& remoteServerAddr, IPv4Address const& authServerAddr)
	:Server(remoteServerAddr)
	, m_isRemote(true)
	, m_authServerAddr(authServerAddr)
	, m_remoteServerAddr(remoteServerAddr)
{
	m_numSamples = 200;
	m_windowSize = 50;
	m_exponentialFilter = 0.1f;
	m_numBins = 10;

	InitialiseBuffer(m_frames);
	InitialiseBuffer(m_frameTimes);
	InitialiseBuffer(m_simpleWindowedAvgs);
	InitialiseBuffer(m_recursiveWindowedAvgs);
	InitialiseBuffer(m_exponentialAvgs);
}


void GameServer::Startup()
{
	m_gameClock = new Clock();
	Server::Startup();
	m_isActive = true;

	m_uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	m_uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, m_uiCanvasSizeX, m_uiCanvasSizeY);
	
	bool startFromLobby = g_gameConfigBlackboard.GetValue("startFromLobby", true);
	GameServerState startState = startFromLobby ? SERVER_STATE_LOBBY : SERVER_STATE_GAME;
	SetServerState(startState);

	g_audioSystem->SetSoundPlaybackSpeed(g_gameLauncher->m_launcherBgMusicPlaybackId, 0.0f);
}


void GameServer::BeginFrame()
{
	m_gameUpdateMsgRcevd = false;
	while (m_isActive)
	{
		std::optional<UDPDatagram*> packet = m_endpoint->Receive();
		if (packet == std::nullopt)
		{
			break;
		}

		if (!m_isRemote)
		{
			ExtractMessages_AuthServer(packet.value());
		}
		else
		{
			ExtractMessages_RemoteServer(packet.value());
		}
	}

	if (m_isConnectedToOtherPlayer && !m_gameUpdateMsgRcevd)
	{
		g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Did not receive game update message"));
	}
}


void GameServer::Update()
{
	CheckInput();
	if (!m_isActive)
		return;

	float deltaSeconds = static_cast<float>(m_gameClock->GetFrameDeltaSeconds());
 	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);

	switch (m_state)
	{
	case SERVER_STATE_LOBBY:
		Update_Lobby(deltaSeconds);
		break;
	case SERVER_STATE_GAME:
		Update_Game(deltaSeconds);
		break;
	default:
		break;
	}
}


void GameServer::Render()
{
	if (!m_isActive)
		return;

	switch (m_state)
	{
	case SERVER_STATE_LOBBY:
		Render_Lobby();
		break;
	case SERVER_STATE_GAME:
		Render_Game();
		break;
	default:
		break;
	}
}


void GameServer::EndFrame()
{
	if (!m_isConnectedToOtherPlayer)
		return;

	if (m_state == SERVER_STATE_GAME)
	{
		if (m_isGamePaused)
			return;

		GameMessage message = {};
		message.m_type = GAME_MESSAGE_GAME_UPDATE;
		message.m_frameNumber = m_frameNumber;
	
		message.m_player1CurPos = m_entities[1]->m_curPosition;
		message.m_player1PrevPos = m_entities[1]->m_prevPos;
		message.m_player1Velocity = m_entities[1]->m_velocity;
		message.m_player1Score = m_entities[1]->m_score;
		
		message.m_player2CurPos = m_entities[2]->m_curPosition;
		message.m_player2PrevPos = m_entities[2]->m_prevPos;
		message.m_player2Velocity = m_entities[2]->m_velocity;
		message.m_player2Score = m_entities[2]->m_score;
		
		message.m_puckCurPos = m_entities[0]->m_curPosition;
		message.m_puckPrevPos = m_entities[0]->m_prevPos;
		message.m_puckVelocity = m_entities[0]->m_velocity;
		message.m_puckAngularVelocity = m_entities[0]->m_angularVelocityDegrees;
		message.m_puckOrientation = m_entities[0]->m_orientationDegrees;
		message.m_isPuckDead = m_entities[0]->m_isDead;
		message.m_latestScoringPlayerIndex = m_latestScoringPlayerIndex;
		message.m_isGameOver = m_isGameOver;

		IPv4Address senderAddr = m_bindingAddr;
		IPv4Address destAddr = m_isRemote ? m_authServerAddr : m_remoteServerAddr;
		UDPDatagram* gameUpdateDatagram = NetworkSystem::CreateDatagram(senderAddr, destAddr);
		AddMessageToDatagram(message, gameUpdateDatagram);
		SendDatagram(gameUpdateDatagram);
	}
}


void GameServer::SpawnEntity(Entity* entity)
{
	m_entities.push_back(entity);
}


void GameServer::UpdateDebugWindow()
{
	ImGui::Begin("DebugWindow");
	ImGui::Text(Stringf("Time Step: %.4f", m_physicsTimeStep).c_str());
	ImGui::Separator();
	ImGui::Text("Controls");
	ImGui::Text("F1: toggle this window");
	ImGui::Text("K/L: Decrease/Increase time step");
	ImGui::Text("F2: show frame stats");
	ImGui::End();
}


void GameServer::UpdatePerformanceStats(float deltaSeconds)
{
	ImGui::Begin("Performance Window");
	ImPlotFlags plotFlags = 0;
	plotFlags |= ImPlotFlags_Crosshairs;
	plotFlags |= ImPlotFlags_NoBoxSelect;
	if (ImPlot::BeginPlot("FPS", "Frame", "Time (seconds)", ImVec2(-1.0f, 0.0f), plotFlags, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit))
	{
		std::vector<float> frames = m_frames.GetBufferAsVector();
		std::vector<float> frameTimes = m_frameTimes.GetBufferAsVector();
		std::vector<float> simpleWindowedAvgs = m_simpleWindowedAvgs.GetBufferAsVector();
		std::vector<float> recursiveWindowedAvgs = m_recursiveWindowedAvgs.GetBufferAsVector();
		std::vector<float> exponentialAvgs = m_exponentialAvgs.GetBufferAsVector();

		ImPlot::PlotLine("Frame Time", frames.data(), frameTimes.data(), (int) frames.size());
		ImPlot::PlotLine("Windowed Avg Actual", frames.data(), simpleWindowedAvgs.data(), (int) frames.size());
		ImPlot::PlotLine("Windowed Avg Short", frames.data(), recursiveWindowedAvgs.data(), (int) frames.size());
		ImPlot::PlotLine("Exponential", frames.data(), exponentialAvgs.data(), (int) frames.size());
		ImPlot::EndPlot();
	}

	bool wasSampleSizeChanged = ImGui::SliderInt("Sample Size", &m_numSamples, 20, 500);
	ImGui::SliderInt("Window Size", &m_windowSize, 20, 500);
	ImGui::SliderFloat("Exponential Filter", &m_exponentialFilter, 0.01f, 1.0f);

	ImPlot::SetNextPlotLimits(0.0, 0.1, 0.0, (double) m_numSamples);
	if (ImPlot::BeginPlot("Frame Time Histogram", "Seconds", "Frames", ImVec2(-1.0f, 0.0f), plotFlags))
	{
		std::vector<float> frameTimes = m_frameTimes.GetBufferAsVector();
		ImPlot::PlotHistogram("Histogram", frameTimes.data(), (int) frameTimes.size(), m_numBins, false, false);
		ImPlot::EndPlot();
	}
	ImGui::SliderInt("Number of Bins", &m_numBins, 5, 50);

	ImGui::End();

	m_perfFrameCount += 1.0f;
	m_frames.Push(m_perfFrameCount);
	m_frameTimes.Push(deltaSeconds);

	float recursiveWindowedAvg = GetRecursiveWindowedAvgForCurrentFrame();
	m_recursiveWindowedAvgs.Push(recursiveWindowedAvg);

	float simpleWindowedAvg = GetSimpleWindowedAvgForCurrentFrame();
	m_simpleWindowedAvgs.Push(simpleWindowedAvg);

	float exponentialAvg = GetExponentialAvgForCurrentFrame();
	m_exponentialAvgs.Push(exponentialAvg);

	if (wasSampleSizeChanged)
	{
		ResizeBuffer(m_frameTimes, m_numSamples);
		ResizeBuffer(m_frames, m_numSamples);
		ResizeBuffer(m_simpleWindowedAvgs, m_numSamples);
		ResizeBuffer(m_recursiveWindowedAvgs, m_numSamples);
		ResizeBuffer(m_exponentialAvgs, m_numSamples);
	}
}


void GameServer::Shutdown()
{
	if (!m_isActive)
		ERROR_AND_DIE("Attemped to shutdown server when server was not active");

	if (m_isConnectedToOtherPlayer)
	{
		SendDisconnectMessageToOtherPlayer();
		return;
	}

	SetServerState(SERVER_STATE_NONE);
	Server::Shutdown();
	m_isActive = false;
	delete m_gameClock;
	m_gameClock = nullptr;

	g_audioSystem->SetSoundPlaybackSpeed(g_gameLauncher->m_launcherBgMusicPlaybackId, 1.0f);
}


void GameServer::SetServerState(GameServerState newState)
{
	switch (m_state)
	{
	case SERVER_STATE_LOBBY:
		Shutdown_Lobby();
		break;
	case SERVER_STATE_GAME:
		Shutdown_Game();
		break;
	default:
		break;
	}

	m_state = newState;

	switch (m_state)
	{
	case SERVER_STATE_LOBBY:
		Startup_Lobby();
		break;
	case SERVER_STATE_GAME:
		Startup_Game();
		break;
	default:
		break;
	}
}


void GameServer::TogglePause()
{
	GameMessage message = {};
	UDPDatagram* datagram = nullptr;
	if (m_isRemote)
	{
		if (m_pausingPlayerIndex == 1)
			return;
	
		if (m_isGamePaused)
		{
			message.m_type = GAME_MESSAGE_GAME_RESUME;
			m_pausingPlayerIndex = 0;
		}
		else
		{
			message.m_type = GAME_MESSAGE_GAME_PAUSE;
			m_pausingPlayerIndex = 2;
		}

		if (m_isConnectedToOtherPlayer)
		{
			datagram = NetworkSystem::CreateDatagram(m_remoteServerAddr, m_authServerAddr);
		}
	}
	else
	{
		if (m_pausingPlayerIndex == 2)
			return;

		if (m_isGamePaused)
		{
			message.m_type = GAME_MESSAGE_GAME_RESUME;
			m_pausingPlayerIndex = 0;
		}
		else
		{
			message.m_type = GAME_MESSAGE_GAME_PAUSE;
			m_pausingPlayerIndex = 1;
		}

		if (m_isConnectedToOtherPlayer)
		{
			datagram = NetworkSystem::CreateDatagram(m_authServerAddr, m_remoteServerAddr);
		}
	}

	if (m_isConnectedToOtherPlayer && datagram != nullptr)
	{
		AddMessageToDatagram(message, datagram);
		SendDatagram(datagram);
	}

	m_gameClock->TogglePause();
	m_isGamePaused = !m_isGamePaused;
	if (m_isGamePaused)
	{
		MouseConfig config = {};
		config.m_isHidden = false;
		config.m_isLocked = false;
		config.m_isRelative = false;
		config.m_priority = 51;
		g_inputSystem->PushMouseConfig(config);
	}
	else
	{
		g_inputSystem->PopMouseConfigOfPriority(51);
	}
}


void GameServer::Startup_Lobby()
{
	if (m_isRemote)
	{
		UDPDatagram* joinRequestDatagram = NetworkSystem::CreateDatagram(m_bindingAddr, m_authServerAddr);
		GameMessage joinRequestMessage = {};
		joinRequestMessage.m_type = GAME_MESSAGE_JOIN_REQUEST;
		AddMessageToDatagram(joinRequestMessage, joinRequestDatagram);
		joinRequestDatagram->m_debugString = "JoinRequestMessage";
		SendDatagram(joinRequestDatagram);
		g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Client join request sent to: %s", m_authServerAddr.GetAsString().c_str()));
	}
}


void GameServer::Update_Lobby(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void GameServer::Render_Lobby()
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	std::vector<Vertex_PCU> textVerts;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 titleTextBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(textVerts, titleTextBox, 800.0f, "LOBBY", Rgba8::MAGENTA);

	if (m_isConnectedToOtherPlayer && !m_isRemote)
	{
		AABB2 subtitleTextBox = AABB2(Vec2(900.0f, 100.0f), 1800.0f, 200.0f);
		font->AddVertsForTextInAABB2(textVerts, subtitleTextBox, 150.0f, "Press Space to start the game", Rgba8::YELLOW);
	}
	else if (m_isConnectedToOtherPlayer && m_isRemote)
	{
		AABB2 subtitleTextBox = AABB2(Vec2(900.0f, 100.0f), 1800.0f, 200.0f);
		font->AddVertsForTextInAABB2(textVerts, subtitleTextBox, 150.0f, "Wait for P1 to start the game", Rgba8::YELLOW);
	}

	g_theRenderer->BeginCamera(m_uiCamera);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void GameServer::Shutdown_Lobby()
{

}


void GameServer::Startup_Game()
{
	m_isGameOver = false;
	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = true;
	config.m_isRelative = false;
	config.m_priority = 50;
	g_inputSystem->PushMouseConfig(config);
	m_physicsTimer = Stopwatch(*m_gameClock, m_physicsTimeStep);

	m_worldCamera.DefineGameSpace(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));
	Mat44 persProjMat = Mat44::CreatePerspectiveProjectionMatrix(FOV_ANGLE_DEGREES, g_window->GetWindowConfig().m_aspectRatio, Z_NEAR, Z_FAR);
	m_worldCamera.SetProjectionMatrix(persProjMat);
	if (!m_isRemote)
	{
		Vec3 cameraPos = g_gameConfigBlackboard.GetValue("cameraPos1", Vec3::ZERO);
		Vec3 cameraOrientation = g_gameConfigBlackboard.GetValue("cameraOrientation1", Vec3::ZERO);
		EulerAngles camEulerAngles = EulerAngles(cameraOrientation.x, cameraOrientation.y, cameraOrientation.z);
		Mat44 orientationMat = camEulerAngles.GetAsMatrix_XFwd_YLeft_ZUp();
		Mat44 camModelMat = Mat44::CreateTranslation3D(cameraPos);
		camModelMat.Append(orientationMat);
		m_camModelMatrix = camModelMat;
		m_worldCamera.SetCameraModelMatrix(camModelMat);
	}
	else
	{
		Vec3 cameraPos = g_gameConfigBlackboard.GetValue("cameraPos2", Vec3::ZERO);
		Vec3 cameraOrientation = g_gameConfigBlackboard.GetValue("cameraOrientation2", Vec3::ZERO);
		EulerAngles camEulerAngles = EulerAngles(cameraOrientation.x, cameraOrientation.y, cameraOrientation.z);
		Mat44 orientationMath = camEulerAngles.GetAsMatrix_XFwd_YLeft_ZUp();
		Mat44 camModelMat = Mat44::CreateTranslation3D(cameraPos);
		camModelMat.Append(orientationMath);
		m_camModelMatrix = camModelMat;
		m_worldCamera.SetCameraModelMatrix(camModelMat);
	}

	Mat44 const& camModelMatrix = m_worldCamera.GetModelMatrix();
	m_playerAudioListener.m_forward = camModelMatrix.GetIBasis3D();
	m_playerAudioListener.m_up = camModelMatrix.GetKBasis3D();
	m_playerAudioListener.m_position = camModelMatrix.GetTranslation3D();
	g_audioSystem->UpdateListeners(1, &m_playerAudioListener);

	static float tableWidth = g_gameConfigBlackboard.GetValue("tableWidth", 32.0f);
	static float tableHeight = g_gameConfigBlackboard.GetValue("tableHeight", 128.0f);
	m_table = AirHockeyTable(tableWidth, tableHeight, Vec3::ZERO);

	m_puck = new Prop(Vec3(0.0f, 0.0f, 0.0f));
	m_entities.push_back(m_puck);
	if (m_isRemote)
	{
		Player* player1 = new Player();
		SpawnEntity(player1);
		PlayerClient* player2Client = new PlayerClient(this, IPv4Address(), IPv4Address(), false);
		player2Client->Startup();
		AddClient(player2Client);
	}
	else
	{
		PlayerClient* player1Client = new PlayerClient(this, IPv4Address(), IPv4Address(), true);
		player1Client->Startup();
		AddClient(player1Client);

		if (m_isConnectedToOtherPlayer)
		{
			Player* player2 = new Player();
			SpawnEntity(player2);
			UDPDatagram* gameStartMessage = NetworkSystem::CreateDatagram(m_bindingAddr, m_remoteServerAddr);
			GameMessage message = {};
			message.m_type = GAME_MESSAGE_GAME_START;
			AddMessageToDatagram(message, gameStartMessage);
			SendDatagram(gameStartMessage);
			g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Game start message sent to: %s", m_remoteServerAddr.GetAsString().c_str()));
		}
	}

	m_neonGlowShaderBuffer = g_theRenderer->CreateConstantBuffer(sizeof(NeonGlowShaderConstants));
	InitialiseTableVBO();

	m_gameBgMusicPlaybackId = g_audioSystem->StartSound(g_gameSounds[MUSIC_GAME_BG_MUSIC], true, g_gameSoundVolumes[MUSIC_GAME_BG_MUSIC]);
}


void GameServer::Update_Game(float deltaSeconds)
{
	ScopeTimer gameUpdateTime = ScopeTimer("Game Update Time");

	if (m_showDebugWindow)
	{
		UpdateDebugWindow();
	}

	if (m_showFrameStats)
	{
		UpdatePerformanceStats(deltaSeconds);
	}

	if (m_showShaderWindow)
	{
		UpdateShaderWindow();
	}

	for (int i = 0; i < (int) m_clients.size(); i++)
	{
		Client* client = m_clients[i];
		PlayerClient* pc = dynamic_cast<PlayerClient*>(client);
		if (pc)
		{
			pc->Update(deltaSeconds);
		}
	}

	if (m_isGamePaused)
	{
		g_audioSystem->SetSoundPlaybackVolume(m_gameBgMusicPlaybackId, 0.0f);
		return;
	}
	g_audioSystem->SetSoundPlaybackVolume(m_gameBgMusicPlaybackId, g_gameSoundVolumes[MUSIC_LAUNCHER_BG_MUSIC]);

	while (m_physicsTimer.CheckAndDecrement() && m_physicsTimeStep > 0.0f)
	{
		for (int i = 0; i < m_entities.size(); i++)
		{
			Entity* entity = m_entities[i];
			if (entity)
			{
				entity->Update(deltaSeconds);
			}
		}

		if(m_isGameOver)
			break;

		PushEntitiesOutOfEachOther();
		
		bool wereWallsHitLastFrame = m_tableEdgesGotHitFlag != 0;
		PushEntityOutOfWalls_Puck(m_entities[0]);
		bool wereWallsHitThisFrame = !wereWallsHitLastFrame && m_tableEdgesGotHitFlag != 0;
		if (wereWallsHitThisFrame)
		{
			m_edgeHitResetFrameCounter = EDGE_HIT_RESET_FRAME_COUNTER;
		}

		if (m_tableEdgesGotHitFlag != 0)
		{
			m_edgeHitResetFrameCounter--;
		}

		if (m_edgeHitResetFrameCounter <= 0)
		{
			m_tableEdgesGotHitFlag = 0;
		}
	}

	UpdateNeonShaderConstantBuffer();
	UpdateTableVertexBuffer();
}


void GameServer::UpdateNeonShaderConstantBuffer()
{
	g_neonGlowShaderConstants.normalizedPerStepDistance = g_neonGlowShaderConstants.normalizedBlurRadius / g_neonGlowShaderConstants.blurNumOutwardSteps;
	g_neonGlowShaderConstants.time = (float) GetCurrentTimeSeconds();
	m_neonGlowShaderBuffer->SetData(g_neonGlowShaderConstants);
}


void GameServer::UpdateTableVertexBuffer()
{
	Rgba8 southColor = IsEdgeHit(EDGE_SOUTH) ? Rgba8::WHITE : g_neonOrange;
	Rgba8 northColor = IsEdgeHit(EDGE_NORTH) ? Rgba8::WHITE : g_neonOrange;
	Rgba8 northEastColor = IsEdgeHit(EDGE_NORTH_EAST) ? Rgba8::WHITE : g_neonPink;
	Rgba8 southEastColor = IsEdgeHit(EDGE_SOUTH_EAST) ? Rgba8::WHITE : g_neonPink;
	Rgba8 northWestColor = IsEdgeHit(EDGE_NORTH_WEST) ? Rgba8::WHITE : g_neonCyan;
	Rgba8 southWestColor = IsEdgeHit(EDGE_SOUTH_WEST) ? Rgba8::WHITE : g_neonCyan;

	float time = (float) GetCurrentTimeSeconds();
	int randomSeed = g_rng->GetRandomIntInRange(0, 100);
	float southColorFlicker		= (float) fabs(Get1dPerlinNoise(time, 2.0f, 5, 0.5f, 2.0f, true, randomSeed)		* 0.2f) + 0.8f;
	float northColorFlicker		= (float) fabs(Get1dPerlinNoise(time, 2.0f, 5, 0.5f, 2.0f, true, randomSeed + 1)	* 0.2f) + 0.8f;
	float northEastColorFlicker = (float) fabs(Get1dPerlinNoise(time, 2.0f, 5, 0.5f, 2.0f, true, randomSeed + 3)	* 0.2f) + 0.8f;
	float southEastColorFlicker = (float) fabs(Get1dPerlinNoise(time, 2.0f, 5, 0.5f, 2.0f, true, randomSeed + 5)	* 0.2f) + 0.8f;
	float northWestColorFlicker = (float) fabs(Get1dPerlinNoise(time, 2.0f, 5, 0.5f, 2.0f, true, randomSeed + 7)	* 0.2f) + 0.8f;
	float southWestColorFlicker = (float) fabs(Get1dPerlinNoise(time, 2.0f, 5, 0.5f, 2.0f, true, randomSeed + 11)	* 0.2f) + 0.8f;
	southColor.ScaleRGB(southColorFlicker);
	northColor.ScaleRGB(northColorFlicker);
	northEastColor.ScaleRGB(northEastColorFlicker);
	southEastColor.ScaleRGB(southEastColorFlicker);
	northWestColor.ScaleRGB(northWestColorFlicker);
	southWestColor.ScaleRGB(southWestColorFlicker);

	TransformVertexArrayColor(m_numVertsPerEdge, m_tableVerts.data() + m_southEdgeVertsStartIndex, southColor);
	TransformVertexArrayColor(m_numVertsPerEdge, m_tableVerts.data() + m_northEdgeVertsStartIndex, northColor);
	TransformVertexArrayColor(m_numVertsPerEdge, m_tableVerts.data() + m_northEastEdgeVertsStartIndex, northEastColor);
	TransformVertexArrayColor(m_numVertsPerEdge, m_tableVerts.data() + m_southEastEdgeVertsStartIndex, southEastColor);
	TransformVertexArrayColor(m_numVertsPerEdge, m_tableVerts.data() + m_southWestEdgeVertsStartIndex, southWestColor);
	TransformVertexArrayColor(m_numVertsPerEdge, m_tableVerts.data() + m_northWestEdgeVertsStartIndex, northWestColor);
	m_tableVBO->CopyVertexArray(m_tableVerts.data(), m_totalNumVerts);
}


void GameServer::Render_Game()
{
	ScopeTimer gameUpdateTime = ScopeTimer("Game Render Time");

	g_theRenderer->ClearScreen(Rgba8::BLACK);
	Texture* defaultDepthStencil = g_theRenderer->GetDefaultDepthStencil();

	Texture* screenTexture = g_theRenderer->CreateOrGetRenderTargetView("ScreenTexture");
	GUARANTEE_OR_DIE(screenTexture, "Unable to create render target view for the screen texture");
	
	//draw to render target
	g_theRenderer->ClearRenderTargetView(screenTexture, Rgba8::BLACK);
	g_theRenderer->BeginCamera(m_worldCamera, screenTexture);
	{
		g_theRenderer->ClearDepth(defaultDepthStencil, 1.0f, 0);
		g_theRenderer->SetDepthOptions(DepthTest::LESS, true);
		RenderGameScene();
	}
	g_theRenderer->EndCamera(m_worldCamera);

	g_theRenderer->BeginCamera(m_uiCamera, screenTexture);
	{
		RenderGameUI();
	}
	g_theRenderer->EndCamera(m_uiCamera);

	//draw to backbuffer
	if (m_renderScene)
	{
		g_theRenderer->BeginCamera(m_worldCamera);
		{
			g_theRenderer->ClearDepth(defaultDepthStencil, 1.0f, 0);
			g_theRenderer->SetDepthOptions(DepthTest::LESS, true);
			RenderGameScene();
		}
		g_theRenderer->EndCamera(m_worldCamera);

		g_theRenderer->BeginCamera(m_uiCamera);
		{
			RenderGameUI();
		}
		g_theRenderer->BeginCamera(m_worldCamera);
	}

	g_theRenderer->BeginCamera(m_uiCamera);
	{
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
			Vec2(0.0f, 1.0f),	//BL
			Vec2(1.0f, 1.0f),	//BR
			Vec2(0.0f, 0.0f),	//TL
			Vec2(1.0f, 0.0f)	//TR
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

		if (m_renderScreenTexture)
		{
			static Texture* noiseTexture = g_theRenderer->CreateOrGetTexture("Data/Images/noiseTexture.png");
			g_theRenderer->BindTexture(1, noiseTexture);
			static Shader* shader = g_theRenderer->CreateOrGetShader("Data/Shaders/NeonBlurPostProcessShader");
			g_theRenderer->BindShader(shader);
			g_theRenderer->BindTexture(0, screenTexture);
			g_theRenderer->BindConstantBuffer(4, m_neonGlowShaderBuffer);
			g_theRenderer->SetSamplingMode(SamplingMode::LINEAR_WRAP);
			g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
			g_theRenderer->DrawVertexArray(6, screenTextureVerts);
		}
	}
	g_theRenderer->EndCamera(m_uiCamera);

	//debug rendering
	DebugRenderWorldToCamera(m_worldCamera);
}


void GameServer::RenderGameScene()
{
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, nullptr);
	for (int i = 0; i < m_entities.size(); i++)
	{
		m_entities[i]->Render();
	}
	g_theRenderer->DrawVertexBuffer(m_tableVBO, m_totalNumVerts);
}


void GameServer::RenderGameUI()
{
	Vec2 uiOrthoBottomLeft = Vec2::ZERO;
	Vec2 uiOrthoTopRight = Vec2(m_uiCanvasSizeX, m_uiCanvasSizeY);
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	std::vector<Vertex_PCU> textVerts;
	Vec2 scoreTextBoxDimensions = Vec2(300.0f, 300.0f);
	if (m_entities[1])
	{
		Rgba8 textColor = m_isRemote ? Rgba8::RED : Rgba8::GREEN;
		Vec2 scoreTextBoxCenter = m_isRemote ? Vec2(1660.0f, 500.0f) : Vec2(200.0f, 500.0f);
		AABB2 player1ScoreTextBox = AABB2(scoreTextBoxCenter, scoreTextBoxDimensions.x, scoreTextBoxDimensions.y);
		font->AddVertsForTextInAABB2(textVerts, player1ScoreTextBox, 50.0f, Stringf("Player1\n\nScore: %i", m_entities[1]->m_score), textColor);
	}

	if (m_isConnectedToOtherPlayer && m_entities[2])
	{
		Rgba8 textColor = m_isRemote ? Rgba8::GREEN : Rgba8::RED;
		Vec2 scoreTextBoxCenter = m_isRemote ? Vec2(200.0f, 500.0f) : Vec2(1660.0f, 500.0f);
		AABB2 player2ScoreTextBox = AABB2(scoreTextBoxCenter, scoreTextBoxDimensions.x, scoreTextBoxDimensions.y);
		font->AddVertsForTextInAABB2(textVerts, player2ScoreTextBox, 50.0f, Stringf("Player2\n\nScore: %i", m_entities[2]->m_score), textColor);
	}

	if (m_isGameOver)
	{
		std::string text = Stringf("Game Over\nPlayer %i WON!!", m_latestScoringPlayerIndex);
		Vec2 uiCenter = (uiOrthoTopRight - uiOrthoBottomLeft) * 0.5f;
		font->AddVertsForTextInAABB2(textVerts, AABB2(uiCenter + Vec2(0.0f, 150.0f), 700.0f, 900.0f), 800.0f, text, Rgba8::WHITE, 1.0f, BitmapFont::ALIGNED_CENTER);
	}
	else if (m_entities[0]->m_isDead)
	{
		std::string text = Stringf("Player %i: Press Spacebar to revive the puck", 3 - m_latestScoringPlayerIndex);
		Vec2 uiCenter = (uiOrthoTopRight - uiOrthoBottomLeft) * 0.5f;
		font->AddVertsForTextInAABB2(textVerts, AABB2(uiCenter + Vec2(0.0f, 150.0f), 700.0f, 450.0f), 400.0f, text, Rgba8::GREEN, 1.0f, BitmapFont::ALIGNED_CENTER);
	}

	g_theRenderer->BindShader(nullptr);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
	
	textVerts.clear();
	if (m_isGamePaused)
	{
		AABB2 screenQuad = AABB2(uiOrthoBottomLeft, uiOrthoTopRight);
		std::vector<Vertex_PCU> quadVerts;
		Rgba8 quadColor = Rgba8(0, 0, 0, 150);
		AddVertsForAABB2ToVector(quadVerts, screenQuad, quadColor);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->DrawVertexArray((int) quadVerts.size(), quadVerts.data());

		AABB2 textBox = AABB2((uiOrthoTopRight - uiOrthoBottomLeft) * 0.5f, m_uiCanvasSizeX, m_uiCanvasSizeY);
		char const* pauseInformation = m_pausingPlayerIndex == 1 ? "Player 1" : "Player 2";
		font->AddVertsForTextInAABB2(textVerts, textBox, 200.0f, Stringf("Game Paused"), Rgba8::WHITE);
		font->AddVertsForTextInAABB2(textVerts, textBox, 50.0f, Stringf("Paused by %s", pauseInformation), Rgba8::WHITE, 1.0f, Vec2(0.5f, 0.35f));
		
		g_theRenderer->BindTexture(0, &fontTexture);
		g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
	}
}


void GameServer::Shutdown_Game()
{
	for (int i = 0; i < (int) m_entities.size(); i++)
	{
		Entity* entity = m_entities[i];
		if (entity)
		{
			delete entity;
			entity = nullptr;
		}
	}
	m_entities.clear();

	g_inputSystem->PopMouseConfigOfPriority(50);

	g_theRenderer->DestroyConstantBuffer(m_neonGlowShaderBuffer);
	g_theRenderer->DestroyVertexBuffer(m_tableVBO);

	g_audioSystem->StopSound(m_gameBgMusicPlaybackId);
}


void GameServer::RevivePuckForPlayer(int playerIndex)
{
	if (m_entities[0]->m_isDead /*&& m_latestScoringPlayerIndex != playerIndex*/ && !m_isGameOver)
	{
		g_audioSystem->StartSound(g_gameSounds[SFX_PUCK_RESET], false, g_gameSoundVolumes[SFX_PUCK_RESET]);
		m_entities[0]->m_isDead = false;
		m_entities[0]->m_velocity = Vec3::ZERO;
		m_entities[0]->m_angularVelocityDegrees = 0.0f;
		m_entities[0]->m_orientationDegrees = 0.0f;
		Vec3 revivalPos = playerIndex == 1 ? Vec3(0.0f, -3.0f, 0.0f) : Vec3(0.0f, 3.0f, 0.0f);
		m_entities[0]->m_curPosition = revivalPos;
		m_entities[0]->m_prevPos = revivalPos;
		m_entities[0]->m_neonTrailPositions.Clear();
	}
}


void GameServer::ExtractMessages_AuthServer(UDPDatagram* datagram)
{
	GameMessage message = {};
	ReadMessageFromDatagram(message, datagram);

	switch (message.m_type)
	{
	case GAME_MESSAGE_JOIN_REQUEST:
	{
		if (m_isConnectedToOtherPlayer)
			return;

		IPv4Address messageSourceAddr = datagram->m_source;
		UDPDatagram* joinAcceptDatagram = NetworkSystem::CreateDatagram(m_bindingAddr, messageSourceAddr);
		GameMessage acceptMessage = {};
		acceptMessage.m_type = GAME_MESSAGE_JOIN_ACCEPT;
		AddMessageToDatagram(acceptMessage, joinAcceptDatagram);
		joinAcceptDatagram->m_debugString = "JoinAcceptMessage";
		SendDatagram(joinAcceptDatagram);
		g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Client join request received from: %s", messageSourceAddr.GetAsString().c_str()));
		g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Client join accept sent to: %s", messageSourceAddr.GetAsString().c_str()));
		m_remoteServerAddr = messageSourceAddr;
		m_isConnectedToOtherPlayer = true;
		break;
	}
	case GAME_MESSAGE_GAME_UPDATE:
	{
		m_entities[2]->m_curPosition = message.m_player2CurPos;
		m_entities[2]->m_prevPos = message.m_player2PrevPos;
		m_entities[2]->m_velocity = message.m_player2Velocity;
		m_gameUpdateMsgRcevd = true;
		if (m_entities[0]->m_isDead && m_latestScoringPlayerIndex == 1)
		{
			m_entities[0]->m_isDead = message.m_isPuckDead;
			m_entities[0]->m_curPosition = message.m_puckCurPos;
			m_entities[0]->m_prevPos = m_entities[0]->m_curPosition;
			m_entities[0]->m_orientationDegrees = message.m_puckOrientation;
			m_entities[0]->m_velocity = message.m_puckVelocity;
			m_entities[0]->m_angularVelocityDegrees = message.m_puckAngularVelocity;
		}
		if (!m_isGameOver)
		{
			m_isGameOver = message.m_isGameOver;
			if (m_isGameOver)
			{
				m_latestScoringPlayerIndex = message.m_latestScoringPlayerIndex;
			}
		}
		break;
	}
	case GAME_MESSAGE_GAME_PAUSE:
	{
		if (m_isGamePaused)
			return;

		PauseGame();
		break;
	}
	case GAME_MESSAGE_GAME_RESUME:
	{
		if (!m_isGamePaused)
			return;

		UnpauseGame();
		break;
	}
	case GAME_MESSAGE_SERVER_DISCONNECT_ACK:
	{
		m_isConnectedToOtherPlayer = false;
		Shutdown();
		break;
	}
	case GAME_MESSAGE_SERVER_DISCONNECT:
	{
		GameMessage ackMessage = {};
		ackMessage.m_type = GAME_MESSAGE_SERVER_DISCONNECT_ACK;
		UDPDatagram* ackMessageDatagram = NetworkSystem::CreateDatagram(m_authServerAddr, m_remoteServerAddr);
		AddMessageToDatagram(ackMessage, ackMessageDatagram);
		SendDatagram(ackMessageDatagram);
		m_isConnectedToOtherPlayer = false;
		Shutdown();
		g_gameLauncher->m_otherPlayerQuitTextFadeoutDuration = 2.0f;
		break;
	}
	default:
		break;
	}
}


void GameServer::ExtractMessages_RemoteServer(UDPDatagram* datagram)
{
	GameMessage message = {};
	ReadMessageFromDatagram(message, datagram);
	switch (message.m_type)
	{
	case GAME_MESSAGE_JOIN_ACCEPT:
	{
		IPv4Address messageSource = datagram->m_source;
		g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Client join accept received from: %s", messageSource.GetAsString().c_str()));
		m_isConnectedToOtherPlayer = true;
		break;
	}
	case GAME_MESSAGE_GAME_START:
	{
		SetServerState(SERVER_STATE_GAME);
		break;
	}
	case GAME_MESSAGE_GAME_UPDATE:
	{
		m_entities[0]->m_curPosition = message.m_puckCurPos;
		m_entities[0]->m_prevPos = message.m_puckPrevPos;
		m_entities[0]->m_velocity = message.m_puckVelocity;
		m_entities[0]->m_orientationDegrees = message.m_puckOrientation;
		m_entities[0]->m_angularVelocityDegrees = message.m_puckAngularVelocity;
		m_entities[0]->m_isDead = message.m_isPuckDead;
		m_entities[1]->m_curPosition = message.m_player1CurPos;
		m_entities[1]->m_prevPos = message.m_player1PrevPos;
		m_entities[1]->m_velocity = message.m_player1Velocity;
		m_entities[1]->m_score = message.m_player1Score;

		m_entities[2]->m_score = message.m_player2Score;

		m_latestScoringPlayerIndex = message.m_latestScoringPlayerIndex;
		if (!m_isGameOver)
		{
			m_isGameOver = message.m_isGameOver;
			if (m_isGameOver)
			{
				m_latestScoringPlayerIndex = message.m_latestScoringPlayerIndex;
			}
		}
		
		m_gameUpdateMsgRcevd = true;
		break;
	}
	case GAME_MESSAGE_GAME_PAUSE:
	{
		if (m_isGamePaused)
			return;

		PauseGame();
		break;
	}
	case GAME_MESSAGE_GAME_RESUME:
	{
		if (!m_isGamePaused)
			return;

		UnpauseGame();
		break;
	}
	case GAME_MESSAGE_SERVER_DISCONNECT:
	{
		GameMessage ackMessage = {};
		ackMessage.m_type = GAME_MESSAGE_SERVER_DISCONNECT_ACK;
		UDPDatagram* ackMessageDatagram = NetworkSystem::CreateDatagram(m_remoteServerAddr, m_authServerAddr);
		AddMessageToDatagram(ackMessage, ackMessageDatagram);
		SendDatagram(ackMessageDatagram);
		m_isConnectedToOtherPlayer = false;
		Shutdown();
		g_gameLauncher->m_otherPlayerQuitTextFadeoutDuration = 2.0f;
		break;
	}
	case GAME_MESSAGE_SERVER_DISCONNECT_ACK:
	{
		m_isConnectedToOtherPlayer = false;
		Shutdown();
		break;
	}
	default:
		break;
	}
}


void GameServer::UpdateShaderWindow()
{
	float blurRadius = g_neonGlowShaderConstants.normalizedBlurRadius;
	int numSteps = g_neonGlowShaderConstants.blurNumOutwardSteps;
	float thetaDeviationPerStep = g_neonGlowShaderConstants.blurThetaDeviationPerStep;
	float thetaDeviationWithinStep = g_neonGlowShaderConstants.blurThetaDeviationWithinStep;
	Mat44 transformMatrix = Mat44();

	static int selectedMatrixIndex = 0;
	static float brightness = 1.0f;
	static float saturation = 0.0f;
	ImGui::Begin("ShaderWindows");
	if (ImGui::Button("Load values from xml"))
	{
		XmlDocument shaderValuesXml;
		shaderValuesXml.LoadFile("Data/XMLData/ShaderData.xml");
		XmlElement* rootElement = nullptr;
		rootElement = shaderValuesXml.RootElement();
		blurRadius = ParseXmlAttribute(*rootElement, "blurRadius", blurRadius);
		numSteps = ParseXmlAttribute(*rootElement, "numSections", numSteps);
		thetaDeviationPerStep = ParseXmlAttribute(*rootElement, "numSamplesPerSection", thetaDeviationPerStep);
		brightness = ParseXmlAttribute(*rootElement, "brightness", brightness);
		saturation = ParseXmlAttribute(*rootElement, "saturation", saturation);
	}

	ImGui::SliderFloat("Gaussian Blur Normalized Radius", &blurRadius, 0.0f, 1.0f);
	ImGui::SliderInt("Gaussian Blur Num Steps", &numSteps, 0, 20);
	ImGui::SliderFloat("Gaussian Blur ThetaDeviationPerStep", &thetaDeviationPerStep, 0, 50);
	ImGui::SliderFloat("Gaussian Blur ThetaDeviationWithinStep", &thetaDeviationWithinStep, 0, 50);
	ImGui::SliderFloat("Brightness", &brightness, -100.0f, 100.0f);
	ImGui::SliderFloat("Saturation", &saturation, -100.0f, 100.0f);

	static int savedTextFrameCountdown = 0;
	if (ImGui::Button("Save values to disk"))
	{
		XmlDocument shaderValuesXml;
		XmlElement* rootElement = nullptr;
		rootElement = shaderValuesXml.NewElement("NeonShader");
		shaderValuesXml.InsertFirstChild(rootElement);

		rootElement->SetAttribute("blurRadius", blurRadius);
		rootElement->SetAttribute("numSteps", numSteps);
		rootElement->SetAttribute("thetaDeviationPerStep", thetaDeviationPerStep);
		rootElement->SetAttribute("thetaDeviationWithinStep", thetaDeviationWithinStep);
		rootElement->SetAttribute("brightness", brightness);
		rootElement->SetAttribute("saturation", saturation);

		XmlError result = shaderValuesXml.SaveFile("Data/XMLData/ShaderData.xml");
		GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to save shader data to xml. TinyXml Error Code %i", result));
		savedTextFrameCountdown = 120;
	}

	if (savedTextFrameCountdown > 0)
	{
		ImGui::Text("Data Saved");
	}
	savedTextFrameCountdown--;
	ImGui::End();

	static const Mat44 identity = Mat44();

	static const float saturationRCoeff = 0.3086f;
	static const float saturationGCoeff = 0.6094f;
	static const float saturationBCoeff = 0.0820f;
	static const float bwMatValues[16] = {saturationRCoeff, saturationGCoeff, saturationBCoeff, 0.0f,
											saturationRCoeff, saturationGCoeff, saturationBCoeff, 0.0f,
											saturationRCoeff, saturationGCoeff, saturationBCoeff, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f};
	static const Mat44 bwTransformMat = Mat44(bwMatValues);

	const float brightnessMatValues[16] = {brightness, 0.0f, 0.0f, 0.0f,
													0.0f, brightness, 0.0f, 0.0f,
													0.0f, 0.0f, brightness, 0.0f,
													0.0f, 0.0f, 0.0f, 1.0f};
	const Mat44 brightnessMat = Mat44(brightnessMatValues);

	float a = (1.0f - saturation) * saturationRCoeff + saturation;
	float b = (1.0f - saturation) * saturationRCoeff;
	float c = (1.0f - saturation) * saturationRCoeff;
	float d = (1.0f - saturation) * saturationGCoeff;
	float e = (1.0f - saturation) * saturationGCoeff + saturation;
	float f = (1.0f - saturation) * saturationGCoeff;
	float g = (1.0f - saturation) * saturationBCoeff;
	float h = (1.0f - saturation) * saturationBCoeff;
	float i = (1.0f - saturation) * saturationBCoeff + saturation;
	const float saturationMatValues[16] = {a, d, g, 0.0f,
											b, e, h, 0.0f,
											c, f, i, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f};
	const Mat44 saturationMat = Mat44(saturationMatValues);

	transformMatrix = saturationMat;
	transformMatrix.Append(brightnessMat);
	g_neonGlowShaderConstants.colorTransformationMat = transformMatrix;
	g_neonGlowShaderConstants.normalizedBlurRadius = blurRadius;
	g_neonGlowShaderConstants.blurNumOutwardSteps = numSteps;
	g_neonGlowShaderConstants.blurThetaDeviationPerStep = thetaDeviationPerStep;
	g_neonGlowShaderConstants.blurThetaDeviationWithinStep = thetaDeviationWithinStep;
}


void GameServer::CheckInput()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_showDebugWindow = !m_showDebugWindow;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		m_showFrameStats = !m_showFrameStats;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		g_debugMode = !g_debugMode;
	}

	if (g_inputSystem->WasKeyJustPressed('L'))
	{
		m_physicsTimeStep *= 1.2f;
		m_physicsTimer.SetDuration(m_physicsTimeStep);
	}

	if (g_inputSystem->WasKeyJustPressed('K'))
	{
		m_physicsTimeStep *= 0.8f;
		m_physicsTimer.SetDuration(m_physicsTimeStep);
	}

	if (g_inputSystem->WasKeyJustPressed('O'))
	{
		m_gameClock->StepFrame();
	}

	if (g_inputSystem->WasKeyJustPressed('U'))
	{
		m_gameClock->Unpause();
	}

	if (g_inputSystem->IsKeyPressed('R'))
	{
		m_gameClock->SetTimeScale(SLOW_TIME_SCALE);
	}
	else if (g_inputSystem->IsKeyPressed('T'))
	{
		m_gameClock->SetTimeScale(1.0);
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		m_gameClock->SetTimeScale(FAST_TIME_SCALE);
	}
	
	if (m_state == SERVER_STATE_LOBBY)
	{
		if (!m_isRemote && g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) && m_isConnectedToOtherPlayer)
		{
			SetServerState(SERVER_STATE_GAME);
 		}
	}
	
	if (m_state == SERVER_STATE_GAME)
	{
		if (g_inputSystem->WasKeyJustPressed('P'))
		{
			TogglePause();
		}
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		Shutdown();
	}

	//intended to force a crash in the exe
	if (g_inputSystem->WasKeyJustPressed('J'))
	{
		Entity* e = nullptr;
		e->m_curPosition += Vec3::ZERO;
	}

	if(g_inputSystem->WasKeyJustPressed(KEYCODE_F9))
	{
		m_renderScene = !m_renderScene;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F10))
	{
		m_renderScreenTexture = !m_renderScreenTexture;
	}

// 	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F11))
// 	{
// 		m_renderScreenTextureAdditive = !m_renderScreenTextureAdditive;
// 	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F4) && !m_isRemote)
	{
		m_showShaderWindow = !m_showShaderWindow;
	}
}


void GameServer::SendDisconnectMessageToOtherPlayer()
{
	UDPDatagram* datagram = nullptr;
	if (m_isRemote)
	{
		datagram = NetworkSystem::CreateDatagram(m_remoteServerAddr, m_authServerAddr);
	}
	else
	{
		datagram = NetworkSystem::CreateDatagram(m_authServerAddr, m_remoteServerAddr);
	}
	GameMessage disconnectMessage = {};
	disconnectMessage.m_type = GAME_MESSAGE_SERVER_DISCONNECT;
	AddMessageToDatagram(disconnectMessage, datagram);
	SendDatagram(datagram);
}


void GameServer::PushEntitiesOutOfWalls()
{
	float tableWidth = m_table.m_width;
	float tableHeight = m_table.m_height;
	Vec2 tableMins = Vec2(-tableWidth, -tableHeight) * 0.5f;
	Vec2 tableMaxs = Vec2(tableWidth, tableHeight) * 0.5f;
	for (int i = 0; i < (int) m_entities.size(); i++)
	{
		Entity*& entity = m_entities[i];
		if (entity == nullptr || entity->m_isDead)
			continue;

		if (entity->m_type == ENTITY_TYPE_PUCK)
		{
			bool wereWallsHitLastFrame = m_tableEdgesGotHitFlag != 0;
			PushEntityOutOfWalls_Puck(entity);
			bool wereWallsHitThisFrame = !wereWallsHitLastFrame && m_tableEdgesGotHitFlag != 0;
			if (wereWallsHitThisFrame)
			{
				m_edgeHitResetFrameCounter = EDGE_HIT_RESET_FRAME_COUNTER;
			}
			return;
		}

		Vec3 originalEntityPos = entity->GetPosition();
		Vec2 positionXY = originalEntityPos.GetVec2();
		Vec2 correctedPosXY = positionXY;
		float physicsRadius = entity->GetPhysicsRadius();
		if (positionXY.x - physicsRadius < tableMins.x)
		{
			correctedPosXY.x = tableMins.x + physicsRadius;
		}
		if (positionXY.x + physicsRadius > tableMaxs.x)
		{
			correctedPosXY.x = tableMaxs.x - physicsRadius;
		}

		if (positionXY.y - physicsRadius < tableMins.y)
		{
			correctedPosXY.y = tableMins.y + physicsRadius;
		}
		if (positionXY.y + physicsRadius > tableMaxs.y)
		{
			correctedPosXY.y = tableMaxs.y - physicsRadius;
		}
		Vec3 correctPos = correctedPosXY.GetVec3();
		entity->SetPosition(correctPos);

		if (entity->m_bouncesOffWalls && correctPos != originalEntityPos)
		{
			Vec3 currentEntityPos = correctPos;
			Vec3 impactNormal = (currentEntityPos - originalEntityPos).GetNormalized();
			Vec3 newVelocity = Vec3::ZERO;
			float newAngularVelocity = 0.0f;
			Vec3 currentVelocity = entity->m_velocity;
			float currentAngularVelocity = entity->m_angularVelocityDegrees;
			BounceDiskOffSurface(WALL_SURFACE_ELASTICITY, WALL_SURFACE_FRICTION, impactNormal, physicsRadius, currentVelocity, currentAngularVelocity, newVelocity, newAngularVelocity);
			entity->SetVelocity(newVelocity);
			entity->SetAngularVelocityDegrees(newAngularVelocity);
		}
	}
}


void GameServer::PushEntityOutOfWalls_Puck(Entity* puck)
{
	if (puck->m_isDead)
		return;

	float tableWidth = m_table.m_width;
	float tableHeight = m_table.m_height;
	Vec2 tableMins = Vec2(-tableWidth, -tableHeight) * 0.5f;
	Vec2 tableMaxs = Vec2(tableWidth, tableHeight) * 0.5f;

	Vec3 originalEntityPos = puck->GetPosition();
	Vec2 positionXY = originalEntityPos.GetVec2();
	Vec2 correctedPosXY = positionXY;
	float physicsRadius = puck->GetPhysicsRadius();
	//south
	if (positionXY.x - physicsRadius < tableMins.x)
	{
		correctedPosXY.x = tableMins.x + physicsRadius;
		m_tableEdgesGotHitFlag += EDGE_SOUTH;
	}

	//north
	if (positionXY.x + physicsRadius > tableMaxs.x)
	{
		correctedPosXY.x = tableMaxs.x - physicsRadius;
		m_tableEdgesGotHitFlag += EDGE_NORTH;
	}

	float goalWidth = g_gameConfigBlackboard.GetValue("goalWidth", 3.0f);
	float goalWidthHalf = goalWidth * 0.5f;
	//east
	float physicsRadiusScale = 0.75f;
	if (positionXY.y - physicsRadius < tableMins.y)
	{
		correctedPosXY.y = tableMins.y + physicsRadius;
		if ((positionXY.x - physicsRadius * physicsRadiusScale) < -goalWidthHalf)
		{
			m_tableEdgesGotHitFlag += EDGE_SOUTH_EAST;
		}
		else if ((positionXY.x + physicsRadius * physicsRadiusScale) > goalWidthHalf)
		{
			m_tableEdgesGotHitFlag += EDGE_NORTH_EAST;
		}
		else if (m_isConnectedToOtherPlayer)
		{
			PlayerScoredPoints(2, 1);
		}
	}
	//west
	if (positionXY.y + physicsRadius > tableMaxs.y)
	{
		correctedPosXY.y = tableMaxs.y - physicsRadius;
		if ((positionXY.x - physicsRadius * physicsRadiusScale) < -goalWidthHalf)
		{
			m_tableEdgesGotHitFlag += EDGE_SOUTH_WEST;
		}
		else if ((positionXY.x + physicsRadius * physicsRadiusScale) > goalWidthHalf)
		{
			m_tableEdgesGotHitFlag += EDGE_NORTH_WEST;
		}
		else
		{
			PlayerScoredPoints(1, 1);
		}
	}
	Vec3 correctPos = correctedPosXY.GetVec3();
	puck->SetPosition(correctPos);

	if (puck->m_bouncesOffWalls && correctPos != originalEntityPos)
	{
		//scored a goal
		if (!puck->m_isDead)
		{
			g_audioSystem->PlaySoundAt(g_gameSounds[SFX_TABLE_HIT], puck->m_curPosition, Vec3::ZERO, g_gameSoundVolumes[SFX_TABLE_HIT]);
		}
		Vec3 currentEntityPos = correctPos;
		Vec3 impactNormal = (currentEntityPos - originalEntityPos).GetNormalized();
		Vec3 newVelocity = Vec3::ZERO;
		float newAngularVelocity = 0.0f;
		Vec3 currentVelocity = puck->m_velocity;
		float currentAngularVelocity = puck->m_angularVelocityDegrees;
		BounceDiskOffSurface(WALL_SURFACE_ELASTICITY, WALL_SURFACE_FRICTION, impactNormal, physicsRadius, currentVelocity, currentAngularVelocity, newVelocity, newAngularVelocity);
		puck->SetVelocity(newVelocity);
		puck->SetAngularVelocityDegrees(newAngularVelocity);
	}
}


void GameServer::PushEntitiesOutOfEachOther()
{
	for (int i = 0; i < (int) m_entities.size(); i++)
	{
		Entity*& entity1 = m_entities[i];
		for (int j = i + 1; j < (int) m_entities.size(); j++)
		{
			Entity*& entity2 = m_entities[j];
			if (entity1 && entity2 && entity1 != entity2)
			{
				if(entity1->m_isDead || entity2->m_isDead)
					continue;

				Vec2 entity1Pos = entity1->GetPosition().GetVec2();
				Vec2 entity2Pos = entity2->GetPosition().GetVec2();
				float entity1Radius = entity1->GetPhysicsRadius();
				float entity2Radius = entity2->GetPhysicsRadius();
				bool isEntity1Static = entity1->m_pushesEntity && !entity1->m_pushedByEntity;
				bool isEntity2Static = entity2->m_pushesEntity && !entity2->m_pushedByEntity;
				bool didCollide = false;
				if (!isEntity1Static && isEntity2Static)
				{
					didCollide = PushDiskOutOfDisk2D(entity1Pos, entity1Radius, entity2Pos, entity2Radius);
				}
				else if (!isEntity2Static && isEntity1Static)
				{
					didCollide = PushDiskOutOfDisk2D(entity2Pos, entity2Radius, entity1Pos, entity1Radius);
				}
				else
				{
					didCollide = PushDisksOutOfEachOther2D(entity1Pos, entity1Radius, entity2Pos, entity2Radius);
				}
				entity1->SetPosition(entity1Pos.GetVec3());
				entity2->SetPosition(entity2Pos.GetVec3());

				Entity* puck = nullptr;
				Entity* paddle = nullptr;
				if (entity1->m_type == ENTITY_TYPE_PLAYER && entity2->m_type == ENTITY_TYPE_PUCK)
				{
					puck = entity2;
					paddle = entity1;
				}

				if (entity1->m_type == ENTITY_TYPE_PUCK && entity2->m_type == ENTITY_TYPE_PLAYER)
				{
					puck = entity1;
					paddle = entity2;
				}

				if (puck && paddle)
				{
					if (didCollide)
					{
						Vec3 puckPos = puck->GetPosition();
						Vec3 paddlePos = paddle->GetPosition();
						Vec3 impactNormal = (puckPos - paddlePos).GetNormalized();
						float puckRadius = puck->GetPhysicsRadius();
						Vec3 puckVelocity = puck->m_velocity;
						Vec3 paddleVelocity = paddle->m_velocity;
						float currentAngularVelocity = puck->m_angularVelocityDegrees;
						Vec3 newPuckVelocity = Vec3::ZERO;
						float newPuckAngularVelocity = 0.0f;
						bool reverseNormalVelocity = DotProduct3D(puckVelocity, impactNormal) <= 0.0f;
						if (!reverseNormalVelocity)
						{
							paddleVelocity *= PADDLE_FORCE_IN_SAME_DIRECTION;
						}
						Vec3 puckRelativeVelocity = puckVelocity - paddleVelocity;
						//g_console->AddLine(DevConsole::MINOR_INFO, 
						//	Stringf("Paddle Velocity: %.2f, %.2f | Puck Velocity: %.2f, %.2f | Impact Normal: %.2f, %.2f | Puck Relative Velocity: %.2f, %.2f | Reverse Normal Vel: %i",
						//	paddle->m_velocity.x, paddle->m_velocity.y, puck->m_velocity.x, puck->m_velocity.y, impactNormal.x, impactNormal.y, puckRelativeVelocity.x, puckRelativeVelocity.y, reverseNormalVelocity));
						BounceDiskOffSurface(PADDLE_SURFACE_ELASTICITY, PADDLE_SURFACE_FRICTION, impactNormal, puckRadius, puckRelativeVelocity, currentAngularVelocity, newPuckVelocity, newPuckAngularVelocity, reverseNormalVelocity);
						newPuckVelocity += paddle->m_velocity;
						//g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("New Puck Velocity: %.2f, %.2f",
						//	newPuckVelocity.x, newPuckVelocity.y));
						puck->SetVelocity(newPuckVelocity);
						puck->SetAngularVelocityDegrees(newPuckAngularVelocity);

						g_audioSystem->PlaySoundAt(g_gameSounds[SFX_PADDLE_HIT_PUCK], paddle->m_curPosition, puck->m_velocity, g_gameSoundVolumes[SFX_PADDLE_HIT_PUCK]);
					}
				}
			}
		}
	}
}


void GameServer::AddMessageToDatagram(GameMessage const& message, UDPDatagram* datagram)
{
	BufferWriter writer = BufferWriter(datagram->m_buffer.data(), DATAGRAM_BUFFER_MAX_SIZE);
	writer.AppendUInt32((uint32_t) message.m_type);
	writer.AppendInt(message.m_frameNumber);

	writer.AppendVec3(message.m_player1CurPos);
	writer.AppendVec3(message.m_player1PrevPos);
	writer.AppendVec3(message.m_player1Velocity);
	writer.AppendInt(message.m_player1Score);
	
	writer.AppendVec3(message.m_player2CurPos);
	writer.AppendVec3(message.m_player2PrevPos);
	writer.AppendVec3(message.m_player2Velocity);
	writer.AppendInt(message.m_player2Score);
	
	writer.AppendVec3(message.m_puckCurPos);
	writer.AppendVec3(message.m_puckPrevPos);
	writer.AppendVec3(message.m_puckVelocity);
	writer.AppendFloat(message.m_puckAngularVelocity);
	writer.AppendFloat(message.m_puckOrientation);
	writer.AppendInt(message.m_isPuckDead);
	writer.AppendInt(message.m_latestScoringPlayerIndex);
	writer.AppendInt(message.m_isGameOver);
}


void GameServer::ReadMessageFromDatagram(GameMessage& out_message, UDPDatagram* datagram)
{
	BufferParser parser = BufferParser(datagram->m_buffer.data(), DATAGRAM_BUFFER_MAX_SIZE);
	out_message.m_type = (GameMessageType) parser.ParseUInt32();
	out_message.m_frameNumber = parser.ParseInt();

	out_message.m_player1CurPos = parser.ParseVec3();
	out_message.m_player1PrevPos = parser.ParseVec3();
	out_message.m_player1Velocity = parser.ParseVec3();
	out_message.m_player1Score = parser.ParseInt();
	
	out_message.m_player2CurPos = parser.ParseVec3();
	out_message.m_player2PrevPos = parser.ParseVec3();
	out_message.m_player2Velocity = parser.ParseVec3();
	out_message.m_player2Score = parser.ParseInt();
	
	out_message.m_puckCurPos = parser.ParseVec3();
	out_message.m_puckPrevPos = parser.ParseVec3();
	out_message.m_puckVelocity = parser.ParseVec3();
	out_message.m_puckAngularVelocity = parser.ParseFloat();
	out_message.m_puckOrientation = parser.ParseFloat();
	out_message.m_isPuckDead = parser.ParseInt();

	out_message.m_latestScoringPlayerIndex = parser.ParseInt();
	out_message.m_isGameOver = parser.ParseInt();
}


bool GameServer::IsEdgeHit(TableEdge edge)
{
	return (m_tableEdgesGotHitFlag & edge) != 0;
}


void GameServer::PlayerScoredPoints(int playerIndex, int wonPts)
{
	if (g_gameConfigBlackboard.GetValue("shouldPuckDie", true) && !m_isGameOver)
	{
		g_audioSystem->PlaySoundAt(g_gameSounds[SFX_SCORED_A_GOAL], m_entities[0]->m_curPosition, Vec3::ZERO, g_gameSoundVolumes[SFX_SCORED_A_GOAL]);
		m_entities[playerIndex]->m_score += wonPts;
		m_entities[0]->m_isDead = true;
		m_latestScoringPlayerIndex = playerIndex;

		if (m_entities[playerIndex]->m_score >= NUM_PTS_FOR_VICTORY)
		{
			m_isGameOver = true;
		}
	}
}


Mat44 GameServer::GetCameraModelMatrix() const
{
	return m_camModelMatrix;
}


bool GameServer::IsServerActive() const
{
	return m_isActive;
}


std::string GameServer::GetDescription() const
{
	if (m_isRemote)
	{
		return Stringf("Remote Server | Is Remote: %i | RemoteServerAddr: %s | Is Active: %i | AuthServerAddress: %s", 
			m_isRemote, m_bindingAddr.GetAsString().c_str(), m_isActive, m_authServerAddr.GetAsString().c_str());
	}

	return Stringf("Auth Server | Is Remote: %i | ServerAddr: %s | Is Active: %i", m_isRemote, m_bindingAddr.GetAsString().c_str(), m_isActive);
}


bool GameServer::IsGamePaused()
{
	return m_isGamePaused;
}


bool GameServer::IsServerRemote()
{
	return m_isRemote;
}


AirHockeyTable GameServer::GetTable()
{
	return m_table;
}


float GameServer::GetSimpleWindowedAvgForCurrentFrame()
{
	float windowedAvg = 0.0f;
	int frameCount = 0;
	int back = m_frameTimes.GetBackIndex();
	int front = m_frameTimes.GetFrontIndex();

	if (m_windowSize <= 0)
		return 0.0f;

	if (back < front)
	{
		while (back >= 0)
		{
			float frameTime = m_frameTimes.GetElementAtIndex(back);
			windowedAvg += frameTime;
			back--;
			frameCount++;

			if (frameCount >= m_windowSize)
			{
				windowedAvg /= frameCount;
				return windowedAvg;
			}
		}

		back = m_frameTimes.GetSize() - 1;
	}

	while (back >= front)
	{
		float frameTime = m_frameTimes.GetElementAtIndex(back);
		windowedAvg += frameTime;
		back--;
		frameCount++;

		if (frameCount >= m_windowSize)
		{
			windowedAvg /= frameCount;
			return windowedAvg;
		}
	}

	return windowedAvg / frameCount;
}


void GameServer::InitialiseBuffer(CircularBuffer<float>& buffer)
{
	buffer = CircularBuffer<float>(m_numSamples);
}


void GameServer::ResizeBuffer(CircularBuffer<float>& buffer, int newSize)
{
	buffer.Resize(newSize);
}


float GameServer::GetRecursiveWindowedAvgForCurrentFrame()
{
	if (m_windowSize <= 0)
		return 0.0f;

	if (m_simpleWindowedAvgs.IsEmpty())
	{
		return 0.0f;
	}

	float windowedAvgPrevFrame = m_simpleWindowedAvgs.GetBackElement();
	float frameTime_n = m_frameTimes.GetBackElement();

	float frameTime_n_minus_m = m_frameTimes.GetElementFromBack(m_windowSize);

	return windowedAvgPrevFrame + (frameTime_n - frameTime_n_minus_m) / m_windowSize;
}

float GameServer::GetExponentialAvgForCurrentFrame()
{
	float alpha = m_exponentialFilter;
	float one_minus_alpha = 1 - alpha;
	float previousExponentialAvg = 0.0f;
	if (!m_exponentialAvgs.IsEmpty())
	{
		previousExponentialAvg = m_exponentialAvgs.GetBackElement();
	}
	return m_frameTimes.GetBackElement() * alpha + previousExponentialAvg * one_minus_alpha;
}


void GameServer::InitialiseTableVBO()
{
	if (m_tableVBO == nullptr)
	{
		m_tableVBO = g_theRenderer->CreateDynamicVertexBuffer();
	}

	static float tableWidth = m_table.m_width;
	static float tableHalfWidth = tableWidth * 0.5f;
	static float tableHeight = m_table.m_height;
	static float tableHalfHeight = tableHeight * 0.5f;
	static Vec3 tableCenter = m_table.m_center;
	static Vec3 tableBottomLeft = tableCenter + Vec3(-tableHalfWidth, -tableHalfHeight, 0.0f);
	static Vec3 tableBottomRight = tableCenter + Vec3(tableHalfWidth, -tableHalfHeight, 0.0f);
	static Vec3 tableTopLeft = tableCenter + Vec3(-tableHalfWidth, tableHalfHeight, 0.0f);
	static Vec3 tableTopRight = tableCenter + Vec3(tableHalfWidth, tableHalfHeight, 0.0f);
	static float goalWidth = g_gameConfigBlackboard.GetValue("goalWidth", 3.0f);
	static float deadZoneHeight = g_gameConfigBlackboard.GetValue("deadZoneHeight", 0.0f);
	static float goalWidthHalf = goalWidth * 0.5f;
	static Vec3 southEdgeStart = tableTopLeft;
	static Vec3 southEdgeEnd = tableBottomLeft;
	static Vec3 northEdgeStart = tableTopRight;
	static Vec3 northEdgeEnd = tableBottomRight;
	static Vec3 northWestEdgeStart = tableTopRight;
	static Vec3 northWestEdgeEnd = tableCenter + Vec3(goalWidthHalf, tableHalfHeight, 0.0f);
	static Vec3 southWestEdgeStart = tableTopLeft;
	static Vec3 southWestEdgeEnd = tableCenter + Vec3(-goalWidthHalf, tableHalfHeight, 0.0f);
	static Vec3 southEastEdgeStart = tableBottomLeft;
	static Vec3 southEastEdgeEnd = tableCenter + Vec3(-goalWidthHalf, -tableHalfHeight, 0.0f);
	static Vec3 northEastEdgeStart = tableBottomRight;
	static Vec3 northEastEdgeEnd = tableCenter + Vec3(goalWidthHalf, -tableHalfHeight, 0.0f);
	static Vec3 player1BoundaryLeft = tableCenter + Vec3(-tableHalfWidth, -deadZoneHeight * 0.5f, 0.0f);
	static Vec3 player1BoundaryRight = tableCenter + Vec3(tableHalfWidth, -deadZoneHeight * 0.5f, 0.0f);
	static Vec3 player2BoundaryLeft = tableCenter + Vec3(-tableHalfWidth, deadZoneHeight * 0.5f, 0.0f);
	static Vec3 player2BoundaryRight = tableCenter + Vec3(tableHalfWidth, deadZoneHeight * 0.5f, 0.0f);
	static Rgba8 southColor = g_neonOrange;
	static Rgba8 northColor = g_neonOrange;
	static Rgba8 northEastColor = g_neonPink;
	static Rgba8 southEastColor = g_neonPink;
	static Rgba8 southWestColor = g_neonCyan;
	static Rgba8 northWestColor = g_neonCyan;
	static float lineThickness = 0.125f;

	m_tableVerts.reserve(500);
	//boundary of the table
	m_southEdgeVertsStartIndex = 0;
	AddVertsForLine3D(m_tableVerts, southEdgeStart, southEdgeEnd, southColor, lineThickness);
	m_numVertsPerEdge = (int) m_tableVerts.size();
	m_northEdgeVertsStartIndex = (int) m_tableVerts.size();
	AddVertsForLine3D(m_tableVerts, northEdgeStart, northEdgeEnd, northColor, lineThickness);
	m_northEastEdgeVertsStartIndex = (int) m_tableVerts.size();
	AddVertsForLine3D(m_tableVerts, northEastEdgeStart, northEastEdgeEnd, northEastColor, lineThickness);
	m_southEastEdgeVertsStartIndex = (int) m_tableVerts.size();
	AddVertsForLine3D(m_tableVerts, southEastEdgeStart, southEastEdgeEnd, southEastColor, lineThickness);
	m_southWestEdgeVertsStartIndex = (int) m_tableVerts.size();
	AddVertsForLine3D(m_tableVerts, southWestEdgeStart, southWestEdgeEnd, southWestColor, lineThickness);
	m_northWestEdgeVertsStartIndex = (int) m_tableVerts.size();
	AddVertsForLine3D(m_tableVerts, northWestEdgeStart, northWestEdgeEnd, northWestColor, lineThickness);

	//play area boundary
	AddVertsForLine3D(m_tableVerts, player1BoundaryLeft, player1BoundaryRight, Rgba8::GREY, 0.125f);
	AddVertsForLine3D(m_tableVerts, player2BoundaryLeft, player2BoundaryRight, Rgba8::GREY, 0.125f);

	m_totalNumVerts = (int) m_tableVerts.size();
	m_tableVBO->CopyVertexArray(m_tableVerts.data(), m_totalNumVerts);
}


void GameServer::PauseGame()
{
	m_isGamePaused = true;
	m_pausingPlayerIndex = m_isRemote ? 1 : 2;
	m_gameClock->Pause();
	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = false;
	config.m_isRelative = false;
	config.m_priority = 51;
	g_inputSystem->PushMouseConfig(config);
}


void GameServer::UnpauseGame()
{
	m_isGamePaused = false;
	m_pausingPlayerIndex = 0;
	m_gameClock->Unpause();
	g_inputSystem->PopMouseConfigOfPriority(51);
}


void GameServer::BounceDiskOffSurface(float surfaceElasticity, float surfaceFriction, Vec3 const& impactNormal, float diskRadius, Vec3 const& in_linearVelocity, float in_angularVelocity, Vec3& out_linearVelocity, float& out_angularVelocity, bool reverseNormalVelocity /*= true*/)
{
	const float degrees360 = DEGREES_360;
	const float degreesOneBy360 = 1.0f / degrees360;

	Vec3 currentVelocityAlongNormal = GetProjectedOnto3D(in_linearVelocity, impactNormal);
	float impactNormalElasticityCoeff = surfaceElasticity;
	Vec3 newVelocityAlongNormal = currentVelocityAlongNormal * impactNormalElasticityCoeff;
	if (reverseNormalVelocity)
	{
		newVelocityAlongNormal *= -1.0f;
	}

	Vec3 currentVelocityAlongTangent = (in_linearVelocity - currentVelocityAlongNormal);
	float currentAngularVelocityDegrees = in_angularVelocity;
	
	Vec3 velDueTranslation_pointOfImpact = currentVelocityAlongTangent;
	float speedDueRotation_pointOfImpact = 2 * PI * diskRadius * currentAngularVelocityDegrees * degreesOneBy360;
 	Vec3 velDueRotation_pointOfImpact = speedDueRotation_pointOfImpact * impactNormal.GetRotatedAboutZDegrees(-90.0f);

	Vec3 impulseDueTranslation = -velDueTranslation_pointOfImpact;
	Vec3 impulseDueRotation = -velDueRotation_pointOfImpact;
	Vec3 netImpulse = (impulseDueRotation + impulseDueTranslation) * surfaceFriction;

	Vec3 newVelocityAlongTangent = currentVelocityAlongTangent + netImpulse;
	Vec3 newAngularVelocity = velDueRotation_pointOfImpact + netImpulse;
	float newAngularSpeed = newAngularVelocity.GetLength();
	float newAngularVelocityDegrees = newAngularSpeed * DEGREES_360 / (2 * PI * diskRadius);
	if (DotProduct3D(newAngularVelocity, currentVelocityAlongTangent) < 0.0f)
	{
		newAngularVelocityDegrees *= -1.0f;
	}

	out_linearVelocity = newVelocityAlongTangent + newVelocityAlongNormal;
	out_angularVelocity = newAngularVelocityDegrees;
}
