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
#include "Game/Entity.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Window/Window.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/BufferParser.hpp"

RandomNumberGenerator* g_rng = nullptr;

Game::Game()
{

}


Game::~Game()
{
}


void Game::Startup()
{
	LoadAssets();

	g_rng = new RandomNumberGenerator();
	SetGameState(GameState::GAME_STATE_CONVEX_SCENE);
}


void Game::LoadAssets()
{
	//load audio
	g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");
}


void Game::ShutDown()
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_CONVEX_SCENE:		DeInitConvexScene();	break;
		case GameState::NONE:						//fall through
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
		m_gameClock.StepFrame();
	}

	//pause game
	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_gameClock.TogglePause();
	}

	//manipulate time scale
	if (g_inputSystem->IsKeyPressed('R'))
	{
		SetTimeScale(1.0f);
	}
	else if (g_inputSystem->IsKeyPressed('T'))
	{
		SetTimeScale(0.2f);
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		SetTimeScale(5.0f);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}


void Game::Update()
{
	CheckInputDeveloperCheats();
	float deltaSeconds = (float) m_gameClock.GetFrameDeltaSeconds();

	//check input
	switch (m_gameState)
	{
		case GameState::GAME_STATE_CONVEX_SCENE:		CheckInputConvexScene();	break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameState::GAME_STATE_CONVEX_SCENE:		UpdateConvexScene(deltaSeconds);	break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}

	UpdateCameras();
}


void Game::UpdateCameras()
{
	//does nothing
}


void Game::Render()
{

	switch (m_gameState)
	{
		case GameState::GAME_STATE_CONVEX_SCENE:		RenderConvexScene();	break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}
}


void Game::InitConvexScene()
{
	float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiCanvasSizeX, uiCanvasSizeY);

	m_shapesWithOutlineVBO = g_theRenderer->CreateDynamicVertexBuffer("shapesWithOutline");
	m_shapesWithoutOutlineVBO = g_theRenderer->CreateDynamicVertexBuffer("shapesWithoutOutline");
	m_shapeOutlineVBO = g_theRenderer->CreateDynamicVertexBuffer("shapeOutlines");

	m_numInvisibleRaycasts = g_gameConfigBlackboard.GetValue("startingRaycastCount", 1024);
	m_renderBSP = true;

	bool shouldReadFromFile = g_gameConfigBlackboard.GetValue("readSceneFromFile", false);
	if (shouldReadFromFile)
	{
		std::string sceneFilePath = g_gameConfigBlackboard.GetValue("sceneFilePath", "");
		ReadSceneFromFile(sceneFilePath);
		UpdateVertexBuffers();
	}
	else
	{
		float worldSizeX = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
		float worldSizeY = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
		float worldSizeHalfX = worldSizeX * 0.5f;
		float worldSizeHalfY = worldSizeY * 0.5f;
		m_worldOrthoBottomLeft = Vec2(-worldSizeHalfX, -worldSizeHalfY);
		m_worldOrthoTopRight = Vec2(worldSizeHalfX, worldSizeHalfY);
		m_worldCamera.SetOrthoView(m_worldOrthoBottomLeft, m_worldOrthoTopRight);

		m_numShapes = g_gameConfigBlackboard.GetValue("startingNumShapes", 8);
		RandomizeScene();
	}

	float width = m_worldOrthoTopRight.x - m_worldOrthoBottomLeft.x;
	m_worldLineWidth = width * 0.001f;
}


void Game::DeInitConvexScene()
{
	g_theRenderer->DestroyVertexBuffer(m_shapesWithOutlineVBO);
	g_theRenderer->DestroyVertexBuffer(m_shapesWithoutOutlineVBO);
	g_theRenderer->DestroyVertexBuffer(m_shapeOutlineVBO);
}


void Game::CheckInputConvexScene()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		IntVec2 mouseScreenPos = g_inputSystem->GetMousePosition();
		m_inputStart = RangeMap(mouseScreenPos.GetVec2(), Vec2::ZERO, g_window->GetWindowDims(), Vec2(m_worldOrthoBottomLeft.x, m_worldOrthoTopRight.y), Vec2(m_worldOrthoTopRight.x, m_worldOrthoBottomLeft.y));
		m_isInputActive = true;
	}

	if (g_inputSystem->IsMouseButtonPressed(0))
	{
		IntVec2 mouseScreenPos = g_inputSystem->GetMousePosition();
		m_inputEnd = RangeMap(mouseScreenPos.GetVec2(), Vec2::ZERO, g_window->GetWindowDims(), Vec2(m_worldOrthoBottomLeft.x, m_worldOrthoTopRight.y), Vec2(m_worldOrthoTopRight.x, m_worldOrthoBottomLeft.y));
	}

	if (g_inputSystem->IsMouseButtonUp(0))
	{
		m_isInputActive = false;
		m_testNode = nullptr;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR))
	{
		m_isInputLocked = !m_isInputLocked;
	}
	
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F11))
	{
		m_raycastUsingBSP = !m_raycastUsingBSP;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F10))
	{
		m_showBSPWindow = !m_showBSPWindow;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F9))
	{
		m_renderShapesAsBlob = !m_renderShapesAsBlob;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F7))
	{
		m_renderBSP = !m_renderBSP;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F6))
	{
		m_raycastScene = !m_raycastScene;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F5))
	{
		m_checkAgainstBoundingDisks = !m_checkAgainstBoundingDisks;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F4))
	{
		RandomizeScene();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		m_renderShapePlanes = !m_renderShapePlanes;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		m_displaySceneStats = !m_displaySceneStats;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_displayControls = !m_displayControls;
	}

	if (g_inputSystem->WasKeyJustPressed('M'))
	{
		m_numInvisibleRaycasts *= 2;
	}

	if (g_inputSystem->WasKeyJustPressed('N'))
	{
		m_numInvisibleRaycasts /= 2;
		m_numInvisibleRaycasts = m_numInvisibleRaycasts < 1 ? 1 : m_numInvisibleRaycasts;
	}

	if (g_inputSystem->WasKeyJustPressed('K'))
	{
		m_numShapes *= 2;
		m_convexShapes.reserve(m_numShapes);
		while (m_convexShapes.size() < m_numShapes)
		{
			ConvexShape convexShape = {};
			RandomizeConvexShape(convexShape);
			m_convexShapes.push_back(convexShape);
		}
		UpdateVertexBuffers();
	}

	if (g_inputSystem->WasKeyJustPressed('J'))
	{
		m_numShapes /= 2;
		m_numShapes = m_numShapes < 1 ? 1 : m_numShapes;
		while (m_convexShapes.size() > m_numShapes)
		{
			m_convexShapes.pop_back();
		}
		UpdateVertexBuffers();
	}

	if (g_inputSystem->WasKeyJustPressed('L'))
	{
		AddChildNodesToNode(m_bspTree.m_rootNode);
	}

	if (g_inputSystem->WasKeyJustPressed('Q'))
	{
		ClearBSPTree(m_bspTree.m_rootNode);
		m_bspTree.m_partitionPlanes.clear();
		m_bspTree.m_treeDepth = -1;
	}

	if (g_inputSystem->WasKeyJustPressed('E'))
	{
		ClearBSPTree(m_bspTree.m_rootNode);
		m_bspTree.m_partitionPlanes.clear();
		m_bspTree.m_treeDepth = -1;
		float timeBeforeBSPGeneration = (float) GetCurrentTimeSeconds();
		GenerateBSPTree(m_bspTree.m_rootNode);
		float timeAfterBSPGeneration = (float) GetCurrentTimeSeconds();
		m_bspTreeGenerationTimeSeconds = timeAfterBSPGeneration - timeBeforeBSPGeneration;
	}

	if (g_inputSystem->WasKeyJustPressed('X'))
	{
		m_maxBSPDepth++;
	}

	if (g_inputSystem->WasKeyJustPressed('Z'))
	{
		m_maxBSPDepth--;
	}

	if (g_inputSystem->WasKeyJustPressed('G'))
	{
		int partitionSchemeIndex = (int) m_partitionScheme;
		partitionSchemeIndex += 1;
		partitionSchemeIndex %= BSPPartitionScheme_COUNT;
		m_partitionScheme = (BSPPartitionScheme) partitionSchemeIndex;
	}

	if (g_inputSystem->WasKeyJustPressed('H'))
	{
		m_renderBoundingDisks = !m_renderBoundingDisks;
	}

	if (g_inputSystem->WasKeyJustPressed('S'))
	{
		WriteSceneToFile();
	}
}


void Game::UpdateConvexScene(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	DebugAddScreenText("Press F1 to see controls", Vec2::ZERO, 0.0f, Vec2::ZERO, 12.0f, Rgba8::MAGENTA);

	if (m_isInputActive || m_isInputLocked)
	{
		Vec2 rayStart = m_inputStart;
		Vec2 ray = m_inputEnd - m_inputStart;
		float maxDistance = ray.GetLengthAndNormalise();
		m_raycastResult = RaycastScene(rayStart, ray, maxDistance);
	}

	if (m_raycastScene)
	{
		RandomlyRaycastScene();
	}

	if (m_displayControls)
	{
		ImGui::Begin("Controls");
		ImGui::Text(Stringf("SpaceBar: Lock input raycast").c_str());
		ImGui::Text(Stringf("F1: Toggle controls window").c_str());
		ImGui::Text(Stringf("F2: Toggle Stats window").c_str());
		ImGui::Text(Stringf("F3: Render planes for a convex shape (only 1 shape's planes will be rendered)").c_str());
		ImGui::Text(Stringf("F4: Randomize scene").c_str());
		ImGui::Text(Stringf("F5: Toggle raycast check vs bounding disks").c_str());
		ImGui::Text(Stringf("F6: Toggle invisible raycasts").c_str());
		ImGui::Text(Stringf("F7: Toggle Render bsp tree").c_str());
		ImGui::Text(Stringf("F9: Toggle rendering shapes as a single blob").c_str());
		ImGui::Text(Stringf("F10: Toggle bsp tree window").c_str());
		ImGui::Text(Stringf("F11: Toggle raycasting using bsp").c_str());
		ImGui::Text(Stringf("H: Render bounding disks").c_str());
		ImGui::Text(Stringf("N/M: Half/Double number of invisible raycasts").c_str());
		ImGui::Text(Stringf("J/K: Half/Double number of shapes in scene").c_str());
		ImGui::Text(Stringf("Z/X: Decrease/Increase max depth of bsp").c_str());
		ImGui::Text(Stringf("G: Toggle through bsp partitioning schemes").c_str());
		ImGui::Text(Stringf("L: Add children to the bsp tree").c_str());
		ImGui::Text(Stringf("Q: Clears bsp tree ").c_str());
		ImGui::Text(Stringf("E: Generate entire bsp tree ").c_str());
		ImGui::End();
	}

	if (m_displaySceneStats)
	{
		ImGui::Begin("Stats");
		ImGui::Text(Stringf("Number of shapes: %i", m_numShapes).c_str());
		ImGui::Text(Stringf("Number of invisible raycasts: %i", m_numInvisibleRaycasts).c_str());
		ImGui::Text(Stringf("Frame time: %.2f seconds or %.2f ms", deltaSeconds, deltaSeconds * 1000).c_str());
		//ImGui::Text(Stringf("Time taken to render shapes: %.2f seconds or %.2f ms", m_renderingShapesTimeSeconds, m_renderingShapesTimeSeconds * 1000).c_str());
		ImGui::Separator();
		ImGui::Text(Stringf("Are invisible raycasts active? : %s", m_raycastScene ? "true" : "false").c_str());
		ImGui::Text(Stringf("Check against bounding disks? : %s", m_checkAgainstBoundingDisks ? "true" : "false").c_str());
		ImGui::Text(Stringf("Time taken by invisible raycasts: %.2f seconds or %.2f ms", m_raycastingTimeSeconds, m_raycastingTimeSeconds * 1000).c_str());
		ImGui::Separator();
		ImGui::Text(Stringf("Raycasting using BSP? : %s", m_raycastUsingBSP ? "true" : "false").c_str());
		ImGui::Text(Stringf("BSP tree depth : %i", m_bspTree.m_treeDepth).c_str());
		ImGui::Text(Stringf("BSP tree max depth : %i", m_maxBSPDepth).c_str());
		ImGui::Text(Stringf("Time taken to generate bsp tree: %.2f seconds or %.2f ms", m_bspTreeGenerationTimeSeconds, m_bspTreeGenerationTimeSeconds * 1000).c_str());
		ImGui::Text(Stringf("BSP PartitioningScheme: %s", m_partitionScheme == BSPPartitionScheme_NAIVE ? "Naive" :
			m_partitionScheme == BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_1 ? "Scoring scheme 1" : 
			m_partitionScheme == BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_2 ? "Scoring scheme 2" : "Scoring scheme 3").c_str());
		ImGui::End();
	}

	if (m_showBSPWindow)
	{
		ImGui::Begin("BSP Tree");
		DisplayBSPTreeData(m_bspTree.m_rootNode);
		ImGui::End();
	}
}


void Game::RenderConvexScene()
{
	g_theRenderer->ClearScreen(Rgba8::WHITE);
	g_theRenderer->BeginCamera(m_worldCamera);
	g_theRenderer->BindShaderByName("Data/Shaders/Default");

	float renderingShapesStartTime = (float) GetCurrentTimeSeconds();
	if (m_renderShapesAsBlob)
	{
		g_theRenderer->DrawVertexBuffer(m_shapeOutlineVBO, m_numVertsForOutline);
		g_theRenderer->DrawVertexBuffer(m_shapesWithoutOutlineVBO, m_numVertsWithoutOutline);
	}
	else
	{
		g_theRenderer->DrawVertexBuffer(m_shapesWithOutlineVBO, m_numVertsWithOutline);
	}
	float renderingShapesEndTime = (float) GetCurrentTimeSeconds();
	m_renderingShapesTimeSeconds = renderingShapesEndTime - renderingShapesStartTime;

	if (m_renderBSP)
	{
		if (m_bspTree.m_rootNode != nullptr)
		{
			m_pointsOfIntersection.clear();
			RenderBSPNode(m_bspTree.m_rootNode, Rgba8::RED);

			for (int i = 0; i < m_pointsOfIntersection.size(); i++)
			{
				DrawRing(m_pointsOfIntersection[i], 10.0f, m_worldLineWidth, Rgba8::GREEN);
			}
		}
	}

	if (m_numShapes > 0 && m_renderShapePlanes)
	{
		std::vector<Plane2> const& planes = m_convexShapes[0].m_convexHull.m_unorderedPlanes;
		for (int i = 0; i < (int) planes.size(); i++)
		{
			Plane2 const& plane = planes[i];
			DrawPlane2(plane, m_worldLineWidth, Rgba8::GREY, Rgba8::MAGENTA);
		}
	}

	if (m_renderBoundingDisks)
	{
		for (int i = 0; i < m_numShapes; i++)
		{
			ConvexShape& shape = m_convexShapes[i];
			DrawRing(shape.m_boundingDiskCenter, shape.m_boundindDiskRadius, m_worldLineWidth * 0.5f, Rgba8::GREY);
		}
	}

	if (m_isInputActive || m_isInputLocked)
	{
		std::vector<Vertex_PCU> inputVerts;
		AddVertsForArrow2DToVector(inputVerts, m_inputStart, m_inputEnd, Rgba8::BLACK, m_worldLineWidth, ARROW_TIP_LENGTH, 0.0f);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->DrawVertexArray((int) inputVerts.size(), inputVerts.data());

		if (m_raycastResult.m_didHit)
		{
			DrawRaycast(m_raycastResult, m_worldLineWidth, Rgba8::GREEN, Rgba8::RED);
		}

// 		if (m_testNode)
// 		{
// 			DrawPlane2(m_testNode->m_parentNode->m_partitionPlane, WORLD_LINE_WIDTH, Rgba8::MAGENTA);
// 		}
	}

	g_theRenderer->EndCamera(m_worldCamera);
}


void Game::SetGameState(GameState gameState)
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_CONVEX_SCENE:		DeInitConvexScene();	break;
		case GameState::NONE:					//fall through
		default: break;
	}

	m_gameState = gameState;

	switch (m_gameState)
	{
		case GameState::GAME_STATE_CONVEX_SCENE:		InitConvexScene(); break;
		case GameState::NONE:					//fall through
		default: break;
	}
}


void Game::SetTimeScale(float timeScale)
{
	m_gameClock.SetTimeScale((double) timeScale);
}


BaseRaycastResult2D Game::RaycastVsConvexShape(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, ConvexShape const& convexShape)
{
	BaseRaycastResult2D result = {};
	if (m_checkAgainstBoundingDisks && convexShape.m_boundindDiskRadius > 0.0f)
	{
		BaseRaycastResult2D earlyOutCheck = RaycastVsDisk2D(rayStart, rayForwardNormal, rayDistance, convexShape.m_boundingDiskCenter, convexShape.m_boundindDiskRadius);
		if (!earlyOutCheck.m_didHit)
			return result;
	}

	result = RaycastVsConvexHull2(rayStart, rayForwardNormal, rayDistance, convexShape.m_convexHull);
	return result;
}


void Game::WriteSceneToFile()
{
	const uint32_t CHUNK_SUB_HEADER_SIZE = 10;
	const uint32_t CHUNK_SUB_FOOTER_SIZE = 4;

	const uint8_t SCENE_INFO_CHUNK_ID = 0x01;
	const uint8_t CONVEX_POLY_CHUNK_ID = 0x02;
	const uint8_t CONVEX_HULL_CHUNK_ID = 0x03;
	const uint8_t BOUNDING_DISK_CHUNK_ID = 0x81;
	const uint8_t BSP_CHUNK_ID = 0x8b;

	const uint8_t ENDIAN_MODE = 1;

	std::vector<unsigned char> byteBuffer;
	BufferWriter writer = BufferWriter(&byteBuffer);
	writer.SetEndianMode(EndinanMode::ENDIAN_MODE_LITTLE);
	//header
	writer.AppendChar('G');
	writer.AppendChar('H');
	writer.AppendChar('C');
	writer.AppendChar('S');
	writer.AppendByte(30); //company ID
	writer.AppendByte(1); //major file version
	writer.AppendByte(1); //minor file version
	writer.AppendByte(ENDIAN_MODE); //Endian mode
	uint32_t tocOffsetLocation = writer.GetTotalBufferSize();
	writer.AppendUInt32(0); //ToC offset
	writer.AppendChar('S');
	writer.AppendChar('C');
	writer.AppendChar('H');
	writer.AppendChar('G');

	const uint32_t SCENE_INFO_CHUNK_SIZE = 34;
	const uint32_t SCENE_INFO_CHUNK_DATA_SIZE = 20;

	//scene info chunk
	uint32_t sceneInfoChunkStartPos = writer.GetTotalBufferSize();
	writer.AppendChar('G');
	writer.AppendChar('H');
	writer.AppendChar('C');
	writer.AppendChar('K');
	writer.AppendByte(SCENE_INFO_CHUNK_ID); //chunk id
	writer.AppendByte(ENDIAN_MODE); //chunk data endian mode
	writer.AppendUInt32(SCENE_INFO_CHUNK_DATA_SIZE); //chunk data size
	writer.AppendVec2(m_worldOrthoBottomLeft);
	writer.AppendVec2(m_worldOrthoTopRight);
	writer.AppendUInt32((uint32_t) m_convexShapes.size());
	writer.AppendChar('K');
	writer.AppendChar('C');
	writer.AppendChar('H');
	writer.AppendChar('G');

	//ConvexPolysChunk
	uint32_t convexPolyChunkStartPos = writer.GetTotalBufferSize();
	writer.AppendChar('G');
	writer.AppendChar('H');
	writer.AppendChar('C');
	writer.AppendChar('K');
	writer.AppendByte(CONVEX_POLY_CHUNK_ID); //chunk id
	writer.AppendByte(ENDIAN_MODE); //chunk data endian mode
	uint32_t convexPolyChunkDataSizeLocation = writer.GetTotalBufferSize();
	uint32_t convexPolyChunkDataSize = 0;
	writer.AppendUInt32(0); //chunk data size
	writer.AppendUInt32((uint32_t) m_convexShapes.size()); //num shapes
	convexPolyChunkDataSize += sizeof(uint32_t);
	for (int shapeIndex = 0; shapeIndex < m_convexShapes.size(); shapeIndex++)
	{
		ConvexShape& shape = m_convexShapes[shapeIndex];
		ConvexPoly2& poly = shape.m_convexPoly;
		uint8_t numPoints = (uint8_t) poly.GetNumPoints();
		writer.AppendByte(numPoints);
		convexPolyChunkDataSize += sizeof(uint8_t);
		for (int ptIndex = 0; ptIndex < numPoints; ptIndex++)
		{
			Vec2 pt = poly.GetPointAtIndex(ptIndex);
			writer.AppendVec2(pt);
			convexPolyChunkDataSize += sizeof(Vec2);
		}
	}
	writer.OverwriteUInt32(convexPolyChunkDataSize, convexPolyChunkDataSizeLocation);
	writer.AppendChar('K');
	writer.AppendChar('C');
	writer.AppendChar('H');
	writer.AppendChar('G');

	//ConvexHullChunk
	uint32_t convexHullChunkStartPos = writer.GetTotalBufferSize();
	writer.AppendChar('G');
	writer.AppendChar('H');
	writer.AppendChar('C');
	writer.AppendChar('K');
	writer.AppendByte(CONVEX_HULL_CHUNK_ID); //chunk id
	writer.AppendByte(ENDIAN_MODE); //chunk data endian mode
	uint32_t convexHullChunkDataSizeLocation = writer.GetTotalBufferSize();
	uint32_t convexHullChunkDataSize = 0;
	writer.AppendUInt32(0); //chunk data size
	writer.AppendUInt32((uint32_t) m_convexShapes.size()); //num shapes
	convexHullChunkDataSize += sizeof(uint32_t);
	for (int shapeIndex = 0; shapeIndex < m_convexShapes.size(); shapeIndex++)
	{
		ConvexShape& shape = m_convexShapes[shapeIndex];
		ConvexHull2& hull = shape.m_convexHull;
		uint8_t numPlanes = (uint8_t) hull.GetNumPlanes();
		writer.AppendByte(numPlanes);
		convexHullChunkDataSize += sizeof(uint8_t);
		for (int planeIndex = 0; planeIndex < numPlanes; planeIndex++)
		{
			Plane2 plane = hull.GetPlaneAtIndex(planeIndex);
			writer.AppendPlane2(plane);
			convexHullChunkDataSize += sizeof(Plane2);
		}
	}
	writer.OverwriteUInt32(convexHullChunkDataSize, convexHullChunkDataSizeLocation);
	writer.AppendChar('K');
	writer.AppendChar('C');
	writer.AppendChar('H');
	writer.AppendChar('G');

	//Bounding Disks
	uint32_t boundingDiskChunkStartPos = writer.GetTotalBufferSize();
	writer.AppendChar('G');
	writer.AppendChar('H');
	writer.AppendChar('C');
	writer.AppendChar('K');
	writer.AppendByte(BOUNDING_DISK_CHUNK_ID); //chunk id
	writer.AppendByte(ENDIAN_MODE); //chunk data endian mode
	uint32_t boundingDiskChunkDataSizeLocation = writer.GetTotalBufferSize();
	uint32_t boundingDiskChunkDataSize = 0;
	writer.AppendUInt32(0); //chunk data size
	writer.AppendUInt32((uint32_t) m_convexShapes.size()); //num shapes
	boundingDiskChunkDataSize += sizeof(uint32_t);
	for (int shapeIndex = 0; shapeIndex < m_convexShapes.size(); shapeIndex++)
	{
		ConvexShape& shape = m_convexShapes[shapeIndex];
		writer.AppendVec2(shape.m_boundingDiskCenter);
		boundingDiskChunkDataSize += sizeof(Vec2);
		writer.AppendFloat(shape.m_boundindDiskRadius);
		boundingDiskChunkDataSize += sizeof(float);
	}
	writer.OverwriteUInt32(boundingDiskChunkDataSize, boundingDiskChunkDataSizeLocation);
	writer.AppendChar('K');
	writer.AppendChar('C');
	writer.AppendChar('H');
	writer.AppendChar('G');

	//BSP2 tree
	uint32_t bspChunkStartPos = writer.GetTotalBufferSize();
	writer.AppendChar('G');
	writer.AppendChar('H');
	writer.AppendChar('C');
	writer.AppendChar('K');
	writer.AppendByte(BOUNDING_DISK_CHUNK_ID); //chunk id
	writer.AppendByte(ENDIAN_MODE); //chunk data endian mode
	uint32_t bspChunkDataSizeLocation = writer.GetTotalBufferSize();
	uint32_t bspChunkDataSize = 0;
	writer.AppendUInt32(0); //chunk data size
	//WriteBSPTreeToBuffer(writer, bspChunkDataSize);
	writer.OverwriteUInt32(bspChunkDataSize, bspChunkDataSizeLocation);
	writer.AppendChar('K');
	writer.AppendChar('C');
	writer.AppendChar('H');
	writer.AppendChar('G');

	//toc
	uint32_t tocOffset = writer.GetTotalBufferSize();
	writer.AppendChar('G');
	writer.AppendChar('H');
	writer.AppendChar('T');
	writer.AppendChar('C');
	writer.AppendByte(4); //num chunks

	//scene info chunk
	writer.AppendByte(SCENE_INFO_CHUNK_ID);
	writer.AppendUInt32(sceneInfoChunkStartPos);
	writer.AppendUInt32(SCENE_INFO_CHUNK_SIZE);

	//convex poly chunk
	writer.AppendByte(CONVEX_POLY_CHUNK_ID);
	writer.AppendUInt32(convexPolyChunkStartPos);
	writer.AppendUInt32(convexPolyChunkDataSize + CHUNK_SUB_FOOTER_SIZE + CHUNK_SUB_HEADER_SIZE);

	//convex hull chunk
	writer.AppendByte(CONVEX_HULL_CHUNK_ID);
	writer.AppendUInt32(convexHullChunkStartPos);
	writer.AppendUInt32(convexHullChunkDataSize + CHUNK_SUB_FOOTER_SIZE + CHUNK_SUB_HEADER_SIZE);

	//bounding disk chunk
	writer.AppendByte(BOUNDING_DISK_CHUNK_ID);
	writer.AppendUInt32(boundingDiskChunkStartPos);
	writer.AppendUInt32(boundingDiskChunkDataSize + CHUNK_SUB_FOOTER_SIZE + CHUNK_SUB_HEADER_SIZE);

	//bsp disk chunk
	writer.AppendByte(BSP_CHUNK_ID);
	writer.AppendUInt32(bspChunkStartPos);
	writer.AppendUInt32(bspChunkDataSize + CHUNK_SUB_FOOTER_SIZE + CHUNK_SUB_HEADER_SIZE);

	writer.AppendChar('C');
	writer.AppendChar('T');
	writer.AppendChar('H');
	writer.AppendChar('G');
	writer.OverwriteUInt32(tocOffset, tocOffsetLocation);

	WriteBufferToFile(byteBuffer, "Data/Manav.GHCS");
}


void Game::ReadSceneFromFile(std::string const& filePath)
{
	std::vector<uint8_t> byteBuffer;
	FileReadToBuffer(byteBuffer, filePath);
	BufferParser parser = BufferParser(byteBuffer.data(), (uint32_t) byteBuffer.size());
	char fileHeaderStartG = parser.ParseChar();
	char fileHeaderStartH = parser.ParseChar();
	char fileHeaderStartC = parser.ParseChar();
	char fileHeaderStartS = parser.ParseChar();
	uint8_t cohortNumber = parser.ParseByte();
	uint8_t majorVersion = parser.ParseByte();
	uint8_t minorVersion = parser.ParseByte();
	uint8_t endianModeByte = parser.ParseByte();
	EndinanMode fileEndianMode = endianModeByte == 2 ? EndinanMode::ENDIAN_MODE_BIG : EndinanMode::ENDIAN_MODE_LITTLE;
	parser.SetEndianMode(fileEndianMode);
	uint32_t tocOffset = parser.ParseUInt32();
	char fileHeaderEndS = parser.ParseChar();
	char fileHeaderEndC = parser.ParseChar();
	char fileHeaderEndH = parser.ParseChar();
	char fileHeaderEndG = parser.ParseChar();

	GUARANTEE_OR_DIE(fileHeaderStartG == 'G', "File header is incorrect");
	GUARANTEE_OR_DIE(fileHeaderStartH == 'H', "File header is incorrect");
	GUARANTEE_OR_DIE(fileHeaderStartC == 'C', "File header is incorrect");
	GUARANTEE_OR_DIE(fileHeaderStartS == 'S', "File header is incorrect");
	GUARANTEE_OR_DIE(cohortNumber == 30, "File header is incorrect");
	GUARANTEE_OR_DIE(majorVersion == 1, "File header is incorrect");
	GUARANTEE_OR_DIE(minorVersion == 1, "File header is incorrect");
	GUARANTEE_OR_DIE(fileHeaderEndS == 'S', "File header is incorrect");
	GUARANTEE_OR_DIE(fileHeaderEndC == 'C', "File header is incorrect");
	GUARANTEE_OR_DIE(fileHeaderEndH == 'H', "File header is incorrect");
	GUARANTEE_OR_DIE(fileHeaderEndG == 'G', "File header is incorrect");
	GUARANTEE_OR_DIE(tocOffset < (uint32_t) byteBuffer.size(), "File header is incorrect");

	const uint32_t TOC_CHUNK_DATA_SIZE = 9;
	parser.JumpToOffset(tocOffset);
	char tocHeaderStartG = parser.ParseChar();
	char tocHeaderStartH = parser.ParseChar();
	char tocHeaderStartT = parser.ParseChar();
	char tocHeaderStartC = parser.ParseChar();
	uint8_t numChunks = parser.ParseByte();
	uint32_t tocChunkDataOffset = tocOffset + 5;
	uint32_t tocEndOffset = tocChunkDataOffset + numChunks * TOC_CHUNK_DATA_SIZE;
	parser.JumpToOffset(tocEndOffset);
	char tocHeaderEndC = parser.ParseChar();
	char tocHeaderEndT = parser.ParseChar();
	char tocHeaderEndH = parser.ParseChar();
	char tocHeaderEndG = parser.ParseChar();
	GUARANTEE_OR_DIE(tocHeaderStartG == 'G', "ToC is incorrect");
	GUARANTEE_OR_DIE(tocHeaderStartH == 'H', "ToC is incorrect");
	GUARANTEE_OR_DIE(tocHeaderStartT == 'T', "ToC is incorrect");
	GUARANTEE_OR_DIE(tocHeaderStartC == 'C', "ToC is incorrect");
	GUARANTEE_OR_DIE(tocHeaderEndC == 'C', "ToC is incorrect");
	GUARANTEE_OR_DIE(tocHeaderEndT == 'T', "ToC is incorrect");
	GUARANTEE_OR_DIE(tocHeaderEndH == 'H', "ToC is incorrect");
	GUARANTEE_OR_DIE(tocHeaderEndG == 'G', "ToC is incorrect");
	GUARANTEE_RECOVERABLE(numChunks > 0, "ToC contains data for 0 or less chunks");
	parser.JumpToOffset(tocChunkDataOffset);
	
	const uint32_t SCENE_INFO_CHUNK_SIZE = 34;
	const uint8_t SCENE_INFO_CHUNK_ID = 0x01;
	const uint8_t CONVEX_POLY_CHUNK_ID = 0x02;
	const uint8_t CONVEX_HULL_CHUNK_ID = 0x03;
	const uint8_t BOUNDING_DISK_CHUNK_ID = 0x81;

	uint32_t sceneInfoChunkOffset = FindChunkOffsetForChunkId(parser, tocChunkDataOffset, numChunks, SCENE_INFO_CHUNK_ID);
	GUARANTEE_OR_DIE(sceneInfoChunkOffset != 0xffffffff, "Unable to find scene info chunk in file");
	uint32_t sceneInfoChunkSize = FindChunkSizeForChunkId(parser, tocChunkDataOffset, numChunks, SCENE_INFO_CHUNK_ID);
	GUARANTEE_OR_DIE(sceneInfoChunkSize == SCENE_INFO_CHUNK_SIZE, Stringf("Scene chunk info size if iscorrect. Size in file: %i", sceneInfoChunkSize));
	ParseChunk(parser, sceneInfoChunkOffset, SCENE_INFO_CHUNK_ID, sceneInfoChunkSize);

	uint32_t convexPolyChunkOffset = FindChunkOffsetForChunkId(parser, tocChunkDataOffset, numChunks, CONVEX_POLY_CHUNK_ID);
	GUARANTEE_OR_DIE(convexPolyChunkOffset != 0xffffffff, "Unable to find convex poly chunk in file");
	uint32_t convexPolyChunkSize = FindChunkSizeForChunkId(parser, tocChunkDataOffset, numChunks, CONVEX_POLY_CHUNK_ID);
	ParseChunk(parser, convexPolyChunkOffset, CONVEX_POLY_CHUNK_ID, convexPolyChunkSize);

	uint32_t convexHullChunkOffset = FindChunkOffsetForChunkId(parser, tocChunkDataOffset, numChunks, CONVEX_HULL_CHUNK_ID);
	if (convexHullChunkOffset != 0xffffffff)
	{
		uint32_t convexHullChunkSize = FindChunkSizeForChunkId(parser, tocChunkDataOffset, numChunks, CONVEX_HULL_CHUNK_ID);
		ParseChunk(parser, convexHullChunkOffset, CONVEX_HULL_CHUNK_ID, convexHullChunkSize);
	}
	else
	{
		ERROR_RECOVERABLE("Unable to find convex hull chunks so generating hulls using convex polys");
		for (int i = 0; i < m_numShapes; i++)
		{
			ConvexShape& shape = m_convexShapes[i];
			shape.m_convexHull = shape.m_convexPoly.GetAsConvexHull();
		}
	}

	uint32_t boundingDiskChunkOffset = FindChunkOffsetForChunkId(parser, tocChunkDataOffset, numChunks, BOUNDING_DISK_CHUNK_ID);
	if (boundingDiskChunkOffset != 0xffffffff)
	{
		uint32_t boundingDiskChunkSize = FindChunkSizeForChunkId(parser, tocChunkDataOffset, numChunks, BOUNDING_DISK_CHUNK_ID);
		ParseChunk(parser, boundingDiskChunkOffset, BOUNDING_DISK_CHUNK_ID, boundingDiskChunkSize);
	}
}


void Game::RandomizeScene()
{
	m_convexShapes.clear();
	m_convexShapes.reserve(m_numShapes);
	for (int i = 0; i < m_numShapes; i++)
	{
		ConvexShape randomShape = {};
		RandomizeConvexShape(randomShape);
		m_convexShapes.push_back(randomShape);
	}

	UpdateVertexBuffers();
}


void Game::UpdateVertexBuffers()
{
	std::vector<Vertex_PCU> convexPolyWithOutlineVerts;
	std::vector<Vertex_PCU> convexPolyWithoutOutlineVerts;
	std::vector<Vertex_PCU> convexPolyOutlineVerts;
	Rgba8 color = Rgba8::CYAN;
	Rgba8 transparentColor = Rgba8(color.r, color.g, color.b, 100);
	for (int i = 0; i < m_numShapes; i++)
	{
		ConvexShape const& shape = m_convexShapes[i];
		AddVertsForConvexPoly2ToVector(convexPolyWithOutlineVerts, shape.m_convexPoly, transparentColor);
		AddVertsForConvexPoly2ToVector(convexPolyWithoutOutlineVerts, shape.m_convexPoly, color);
		AddVertsForConvexPoly2BorderToVector(convexPolyWithOutlineVerts, shape.m_convexPoly, Rgba8::BLACK, BORDER_LINE_WIDTH);
		AddVertsForConvexPoly2BorderToVector(convexPolyOutlineVerts, shape.m_convexPoly, Rgba8::BLACK, BORDER_LINE_WIDTH * 2.0f);
	}
	m_numVertsWithOutline = (int) convexPolyWithOutlineVerts.size();
	m_shapesWithOutlineVBO->CopyVertexArray(convexPolyWithOutlineVerts.data(), m_numVertsWithOutline);

	m_numVertsWithoutOutline = (int) convexPolyWithoutOutlineVerts.size();
	m_shapesWithoutOutlineVBO->CopyVertexArray(convexPolyWithoutOutlineVerts.data(), m_numVertsWithoutOutline);

	m_numVertsForOutline = (int) convexPolyOutlineVerts.size();
	m_shapeOutlineVBO->CopyVertexArray(convexPolyOutlineVerts.data(), m_numVertsForOutline);
}


void Game::RandomizeConvexShape(ConvexShape& out_shape)
{
	out_shape.m_convexPoly.ClearAllPoints();

	float minRadius = g_gameConfigBlackboard.GetValue("randomPolyMinRadius", 1.0f);
	float maxRadius = g_gameConfigBlackboard.GetValue("randomPolyMaxRadius", 7.0f);;
	float radius = g_rng->GetRandomFloatInRange(minRadius, maxRadius);
	out_shape.m_boundindDiskRadius = radius;
	float worldFraction = 0.9f;
	Vec2 boundingDiskCenter = g_rng->GetRandomPositionInAABB2(m_worldOrthoBottomLeft * worldFraction, m_worldOrthoTopRight * worldFraction);
	out_shape.m_boundingDiskCenter = boundingDiskCenter;

	float minThetaDegrees = g_gameConfigBlackboard.GetValue("randomPolyMinThetaDegrees", 10.0f);
	float maxThetaDegrees = g_gameConfigBlackboard.GetValue("randomPolyMaxThetaDegrees", 150.0f);
	float startThetaDegrees = g_rng->GetRandomFloatInRange(minThetaDegrees, maxThetaDegrees);
	Vec2 startPt = boundingDiskCenter + Vec2::MakeFromPolarDegrees(startThetaDegrees, radius);
	out_shape.m_convexPoly.AddPoint(startPt);
	float nextthetaDegrees = startThetaDegrees + g_rng->GetRandomFloatInRange(minThetaDegrees, maxThetaDegrees);
	while (nextthetaDegrees - startThetaDegrees < 350.0f)
	{
		Vec2 nextPt = boundingDiskCenter + Vec2::MakeFromPolarDegrees(nextthetaDegrees, radius);
		out_shape.m_convexPoly.AddPoint(nextPt);
		nextthetaDegrees += g_rng->GetRandomFloatInRange(minThetaDegrees, maxThetaDegrees);
	}

	out_shape.m_convexHull = out_shape.m_convexPoly.GetAsConvexHull();
}


void Game::RandomizePlane(Plane2& plane)
{
	float randomOrientation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	Vec2 randomNormal = Vec2::MakeFromPolarDegrees(randomOrientation, 1.0f);
	float minDist = g_gameConfigBlackboard.GetValue("minDistFromOrigin", 5.0f);
	float maxDist = g_gameConfigBlackboard.GetValue("maxDistFromOrigin", 500.0f);
	float randomDistFromOrigin = g_rng->GetRandomFloatInRange(minDist, maxDist);

	plane.m_fwdNormal = randomNormal;
	plane.m_distanceFromOrigin = randomDistFromOrigin;
}


BaseRaycastResult2D Game::RaycastScene(Vec2 const& rayStart, Vec2 const& rayFwdNormal, float maxDistance)
{
	if (m_raycastUsingBSP)
		return RaycastScene_WithBSP(rayStart, rayFwdNormal, maxDistance);

	return RaycastScene_WithoutBSP(rayStart, rayFwdNormal, maxDistance);
}


BaseRaycastResult2D Game::RaycastScene_WithoutBSP(Vec2 const& rayStart, Vec2 const& rayFwdNormal, float maxDistance)
{
	BaseRaycastResult2D result = {};
	float nearestHitDistance = 99999.0f;
	for (int i = 0; i < m_numShapes; i++)
	{
		ConvexShape const& shape = m_convexShapes[i];
		BaseRaycastResult2D raycastResult = RaycastVsConvexShape(rayStart, rayFwdNormal, maxDistance, shape);
		if (raycastResult.m_didHit && raycastResult.m_impactDistance < nearestHitDistance)
		{
			result = raycastResult;
			nearestHitDistance = raycastResult.m_impactDistance;
		}
	}
	return result;
}


BaseRaycastResult2D Game::RaycastScene_WithBSP(Vec2 const& rayStart, Vec2 const& rayFwdNormal, float maxDistance)
{
	struct PointOfIntersectionData
	{
	public:
		Plane2 m_intersectingPlane;
		Vec2 m_pointOfIntersection;
		float m_distanceToIntersection = 0.0f;
		bool m_enteringPlaneNode = false;
		BSPNode* m_partitionNode = nullptr;
	};
	static PointOfIntersectionData enteringPoi = {};
	enteringPoi = {};

	BaseRaycastResult2D result = {};
	result.m_startPos = rayStart;
	result.m_forwardNormal = rayFwdNormal;
	result.m_maxDistance = maxDistance;

	BSPNode* node = m_bspTree.GetNodeForPoint(rayStart);
	if (node == nullptr)
	{
		static float backwardCorrectionLength = 0.5f;
		node = m_bspTree.GetNodeForPoint(rayStart - rayFwdNormal * backwardCorrectionLength);
	}

	Vec2 startingPoint = rayStart;
	float remainingDistance = maxDistance;
	while (node)
	{
		int numShapes = node->m_numShapesInsideNode;
		std::vector<ConvexShape> const& shapes = node->m_shapesInNode;
		float nearestHitDistance = 999'999.0f;
		for (int i = 0; i < numShapes; i++)
		{
			ConvexShape const& shape = shapes[i];
			BaseRaycastResult2D raycast = RaycastVsConvexShape(rayStart, rayFwdNormal, maxDistance, shape);
			if (raycast.m_didHit && raycast.m_impactDistance < nearestHitDistance)
			{
				result = raycast;
				nearestHitDistance = raycast.m_impactDistance;
			}
		}

		if (result.m_didHit)
			break;

		if (remainingDistance <= 0.0f)
			break;

		BSPNode* parentNode = node->m_parentNode;
		std::vector<PointOfIntersectionData> pointsOfIntersection;
		while (parentNode != nullptr)
		{
			//find intersection bw raycast and plane
			Plane2 plane = parentNode->m_partitionInfo.m_partitionPlane;
			if (!ArePlanesVirtuallyEqual(plane, enteringPoi.m_intersectingPlane))
			{
				BaseRaycastResult2D raycast = RaycastVsPlane2(startingPoint, rayFwdNormal, remainingDistance, plane);
				if (raycast.m_didHit)
				{
					PointOfIntersectionData poiData = {};
					poiData.m_intersectingPlane = plane;
					poiData.m_distanceToIntersection = raycast.m_impactDistance;
					poiData.m_pointOfIntersection = raycast.m_impactPoint;
					poiData.m_enteringPlaneNode = raycast.m_impactNormal == plane.m_fwdNormal;
					poiData.m_partitionNode = parentNode;
					pointsOfIntersection.push_back(poiData);
				}
			}
			parentNode = parentNode->m_parentNode;
		}

		if (pointsOfIntersection.size() == 0)
			break;

		float nearestIntersectionDist = 999'999.0f;
		for (int i = 0; i < (int) pointsOfIntersection.size(); i++)
		{
			PointOfIntersectionData const poi = pointsOfIntersection[i];
			if (poi.m_distanceToIntersection < nearestIntersectionDist)
			{
				enteringPoi = poi;
				nearestIntersectionDist = poi.m_distanceToIntersection;
			}
		}

		BSPNode* enteringNode = nullptr;
		if (enteringPoi.m_enteringPlaneNode)
		{
			enteringNode = enteringPoi.m_partitionNode->m_backNode;
		}
		else
		{
			enteringNode = enteringPoi.m_partitionNode->m_frontNode;
		}
		while (enteringNode->m_backNode && enteringNode->m_frontNode)
		{
			Plane2 enteringNodeParitionPlane = enteringNode->m_partitionInfo.m_partitionPlane;
			if (enteringNodeParitionPlane.IsPointInFront(enteringPoi.m_pointOfIntersection))
			{
				enteringNode = enteringNode->m_frontNode;
			}
			else
			{
				enteringNode = enteringNode->m_backNode;
			}
		}

		startingPoint = enteringPoi.m_pointOfIntersection;
		remainingDistance -= enteringPoi.m_distanceToIntersection;
		node = enteringNode;
	}

	return result;
}


void Game::RandomlyRaycastScene()
{
	struct RaycastStruct
	{
	public:
		RaycastStruct(Vec2 const& rayStart, Vec2 const& rayFwdNormal, float maxDistance) : m_rayStart(rayStart), m_rayFwdNormal(rayFwdNormal), m_maxDistance(maxDistance) {}
	public:
		Vec2 m_rayStart;
		Vec2 m_rayFwdNormal;
		float m_maxDistance = 0.0f;
	};

	std::vector<RaycastStruct> randomRaycasts;
	randomRaycasts.reserve(m_numInvisibleRaycasts);
	for (int i = 0; i < m_numInvisibleRaycasts; i++)
	{
		Vec2 randomRaycastStart = g_rng->GetRandomPositionInAABB2(m_worldOrthoBottomLeft, m_worldOrthoTopRight);
		float randomOrientation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
		Vec2 randomFwdNormal = Vec2::MakeFromPolarDegrees(randomOrientation, 1.0f);
		float randomMaxDistance = g_rng->GetRandomFloatInRange(20.0f, 5000.0f);
		randomRaycasts.emplace_back(randomRaycastStart, randomFwdNormal, randomMaxDistance);
	}

	float raycastStartTime = (float) GetCurrentTimeSeconds();
	for (int i = 0; i < m_numInvisibleRaycasts; i++)
	{
		RaycastStruct const& raycast = randomRaycasts[i];
		RaycastScene(raycast.m_rayStart, raycast.m_rayFwdNormal, raycast.m_maxDistance);
	}
	float raycastEndTime = (float) GetCurrentTimeSeconds();
	m_raycastingTimeSeconds = raycastEndTime - raycastStartTime;
}


void Game::GenerateBSPTree(BSPNode*& node)
{
	if (node == nullptr)
	{
		node = new BSPNode();
		node->m_numShapesInsideNode = m_numShapes;
		node->m_shapesInNode = m_convexShapes;
	}

	if (node->m_noMorePartitionsPossible)
		return;

	AddChildNodesToNode(node);
	//check if children were added
	//if added then generate the tree further with them
	if (node->m_backNode && node->m_frontNode)
	{
		GenerateBSPTree(node->m_frontNode);
		GenerateBSPTree(node->m_backNode);
	}
}


void Game::AddChildNodesToNode(BSPNode*& bspNode)
{
	if (bspNode == nullptr)
	{
		bspNode = new BSPNode();
		bspNode->m_numShapesInsideNode = m_numShapes;
		bspNode->m_shapesInNode = m_convexShapes;
	}
	
	if (bspNode->m_backNode == nullptr && bspNode->m_frontNode == nullptr)
	{
		if (bspNode->m_noMorePartitionsPossible)
			return;

		AddPartitionToNode(bspNode);
	}
	else
	{
		AddChildNodesToNode(bspNode->m_frontNode);
		AddChildNodesToNode(bspNode->m_backNode);
	}
}


BSPPartitionStruct Game::GetBestPartition(std::vector<BSPPartitionStruct> const& partitionData, BSPNode* bspNode)
{
	BSPPartitionStruct bestPartition = {};
	//Scheme 1
	switch (m_partitionScheme)
	{
	case BSPPartitionScheme_NAIVE:
	{
		bool allPlanesIntersectShapes = true;
		for (int i = 0; i < (int) partitionData.size(); i++)
		{
			BSPPartitionStruct const& partitionInfo = partitionData[i];
			int numIntersectionShapes = partitionInfo.m_numIntersectingShapes;
			if (numIntersectionShapes == 0 && !IsPlaneInBSPTree(partitionInfo.m_partitionPlane))
			{
				allPlanesIntersectShapes = false;
				break;
			}
		}

		if (allPlanesIntersectShapes)
		{
			//find partition with least intersections
			int leastIntersectionCount = 100'000;
			for (int i = 0; i < (int) partitionData.size(); i++)
			{
				BSPPartitionStruct const& partitionInfo = partitionData[i];
				if (IsPlaneInBSPTree(partitionInfo.m_partitionPlane) || IsPartitionOutsideNode(partitionInfo, bspNode))
				{
					continue;
				}

				if (partitionInfo.m_numIntersectingShapes < leastIntersectionCount)
				{
					bestPartition = partitionInfo;
					leastIntersectionCount = partitionInfo.m_numIntersectingShapes;
				}
			}
		}
		else
		{
			bool foundBestPartition = false;
			for (int i = 0; i < (int) partitionData.size(); i++)
			{
				BSPPartitionStruct const& partitionInfo = partitionData[i];
				if (IsPlaneInBSPTree(partitionInfo.m_partitionPlane) || IsPartitionOutsideNode(partitionInfo, bspNode))
				{
					continue;
				}

				if (partitionInfo.m_numIntersectingShapes == 0)
				{
					if (!foundBestPartition)
					{
						bestPartition = partitionInfo;
					}

					int numShapesBehind = partitionInfo.m_numShapesBehind;
					int numShapesInFront = partitionInfo.m_numShapesInFront;
					if (numShapesBehind == 0 || numShapesInFront == 0)
					{
						bestPartition = partitionInfo;
						foundBestPartition = true;
						break;
					}

					int differentInShapeCount = numShapesBehind - numShapesInFront;
					if (differentInShapeCount >= -2 && differentInShapeCount <= 2)
					{
						bestPartition = partitionInfo;
						foundBestPartition = true;
					}
				}
			}
		}
		break;
	}
	case BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_1:
	{
		std::vector<BSPPartitionStruct> uniquePartitions;
		for (int i = 0; i < (int) partitionData.size(); i++)
		{
			BSPPartitionStruct partition = partitionData[i];
			if(IsPlaneInBSPTree(partition.m_partitionPlane) || IsPartitionOutsideNode(partition, bspNode))
				continue;
			
			bool isPartitionUnique = true;
			for (int j = 0; j < (int) uniquePartitions.size(); j++)
			{
				BSPPartitionStruct& uniquePartition = uniquePartitions[j];
				if (uniquePartition.m_partitionPlane == partition.m_partitionPlane)
				{
					uniquePartition.m_partitionScore++;
					isPartitionUnique = false;
					break;
				}
			}
			
			if (!isPartitionUnique)
				continue;
			
			bool areObjectsAllOnOneSide = partition.m_numShapesInFront == 0 || partition.m_numShapesBehind == 0;
			bool areObjectsSplitEvenly = abs(partition.m_numShapesInFront - partition.m_numShapesBehind) <= 5.0f;
			bool areObjectsDivided = abs(partition.m_numShapesInFront - partition.m_numShapesBehind) > 0.0f;
			int partitionScore = areObjectsSplitEvenly * 5 + areObjectsDivided * 5 + areObjectsAllOnOneSide * 1 - partition.m_numIntersectingShapes * 2;
			partition.m_partitionScore = partitionScore;
			uniquePartitions.push_back(partition);
		}

		int highestScore = 0;
		for (int i = 0; i < (int) uniquePartitions.size(); i++)
		{
			BSPPartitionStruct partition = uniquePartitions[i];
			if (partition.m_partitionScore > highestScore && !IsPlaneInBSPTree(partition.m_partitionPlane))
			{
				bestPartition = partition;
				highestScore = partition.m_partitionScore;
			}
		}
		break;
	}
	case BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_2:
	{
		std::vector<BSPPartitionStruct> uniquePartitions;
		for (int i = 0; i < (int) partitionData.size(); i++)
		{
			BSPPartitionStruct partition = partitionData[i];
			if (IsPlaneInBSPTree(partition.m_partitionPlane) || IsPartitionOutsideNode(partition, bspNode))
				continue;

			bool isPartitionUnique = true;
			for (int j = 0; j < (int) uniquePartitions.size(); j++)
			{
				BSPPartitionStruct& uniquePartition = uniquePartitions[j];
				if (uniquePartition.m_partitionPlane == partition.m_partitionPlane)
				{
					uniquePartition.m_partitionScore++;
					isPartitionUnique = false;
					break;
				}
			}

			if (!isPartitionUnique)
				continue;

			int objectDivisionCount = (int) abs(partition.m_numShapesInFront - partition.m_numShapesBehind);
			int partitionScore =  (5 - objectDivisionCount) * 10 - partition.m_numIntersectingShapes * 2;
			partition.m_partitionScore = partitionScore;
			uniquePartitions.push_back(partition);
		}

		int highestScore = -999'999'999;
		for (int i = 0; i < (int) uniquePartitions.size(); i++)
		{
			BSPPartitionStruct partition = uniquePartitions[i];
			if (partition.m_partitionScore > highestScore && !IsPlaneInBSPTree(partition.m_partitionPlane))
			{
				bestPartition = partition;
				highestScore = partition.m_partitionScore;
			}
		}
		break;
	}
	case BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_3:
	{
		std::vector<BSPPartitionStruct> uniquePartitions;
		int numShapes = bspNode->m_numShapesInsideNode;
		for (int i = 0; i < (int) partitionData.size(); i++)
		{
			BSPPartitionStruct partition = partitionData[i];
			if (IsPlaneInBSPTree(partition.m_partitionPlane) || IsPartitionOutsideNode(partition, bspNode))
				continue;

			bool isPartitionUnique = true;
			for (int j = 0; j < (int) uniquePartitions.size(); j++)
			{
				BSPPartitionStruct& uniquePartition = uniquePartitions[j];
				if (uniquePartition.m_partitionPlane == partition.m_partitionPlane)
				{
					uniquePartition.m_partitionScore++;
					isPartitionUnique = false;
					break;
				}
			}

			if (!isPartitionUnique)
				continue;

			int objectDivisionCount = (int) abs(partition.m_numShapesInFront - partition.m_numShapesBehind);
			int partitionScore =  (numShapes + 1 - objectDivisionCount) * 10 - partition.m_numIntersectingShapes * 2;
			partition.m_partitionScore = partitionScore;
			uniquePartitions.push_back(partition);
		}

		int highestScore = 10 - numShapes * 2 - 1;
		for (int i = 0; i < (int) uniquePartitions.size(); i++)
		{
			BSPPartitionStruct partition = uniquePartitions[i];
			if (partition.m_partitionScore > highestScore && !IsPlaneInBSPTree(partition.m_partitionPlane))
			{
				bestPartition = partition;
				highestScore = partition.m_partitionScore;
			}
		}
		break;
	}

	case BSPPartitionScheme_COUNT:
		break;
	}

	return bestPartition;
}


void Game::AddPartitionToNode(BSPNode*& bspNode)
{
	if (bspNode->m_nodeDepth >= m_maxBSPDepth)
	{
		bspNode->m_noMorePartitionsPossible = true;
		return;
	}

	int numShapesInNode = bspNode->m_numShapesInsideNode;
	if (numShapesInNode != 0)
	{
		CreatePartitionInNode(bspNode);

		if (bspNode->m_noMorePartitionsPossible)
			return;

		int newDepth = bspNode->m_nodeDepth + 1;

		std::vector<ConvexShape> const& shapesInNode = bspNode->m_shapesInNode;
		Plane2 partitionPlane = bspNode->m_partitionInfo.m_partitionPlane;
		BSPNode* frontNode = new BSPNode();
		frontNode->m_parentNode = bspNode;
		frontNode->m_isFrontNode = true;
		AddShapesInFrontOfPlaneToVector(frontNode->m_shapesInNode, partitionPlane, shapesInNode);
		AddShapesIntersectionPlaneToVector(frontNode->m_shapesInNode, partitionPlane, shapesInNode);
		frontNode->m_numShapesInsideNode = (int) frontNode->m_shapesInNode.size();
		frontNode->m_nodeDepth = newDepth;
		bspNode->m_frontNode = frontNode;

		BSPNode* backNode = new BSPNode();
		backNode->m_parentNode = bspNode;
		AddShapesBehindPlaneToVector(backNode->m_shapesInNode, partitionPlane, shapesInNode);
		AddShapesIntersectionPlaneToVector(backNode->m_shapesInNode, partitionPlane, shapesInNode);
		backNode->m_numShapesInsideNode = (int) backNode->m_shapesInNode.size();
		backNode->m_nodeDepth = newDepth;
		bspNode->m_backNode = backNode;

		if (newDepth > m_bspTree.m_treeDepth)
		{
			m_bspTree.m_treeDepth = newDepth;
		}
	}
}


void Game::AddShapesInFrontOfPlaneToVector(std::vector<ConvexShape>& vector, Plane2 const& plane, std::vector<ConvexShape> const& refShapes)
{
	for (int i = 0; i < (int) refShapes.size(); i++)
	{
		ConvexShape const& shape = refShapes[i];
		if (IsShapeInFrontOfPlane(plane, shape))
		{
			vector.push_back(shape);
		}
	}
}


void Game::AddShapesBehindPlaneToVector(std::vector<ConvexShape>& vector, Plane2 const& plane, std::vector<ConvexShape> const& refShapes)
{
	for (int i = 0; i < (int) refShapes.size(); i++)
	{
		ConvexShape const& shape = refShapes[i];
		if (IsShapeBehindPlane(plane, shape))
		{
			vector.push_back(shape);
		}
	}
}


void Game::AddShapesIntersectionPlaneToVector(std::vector<ConvexShape>& vector, Plane2 const& plane, std::vector<ConvexShape> const& refShapes)
{
	for (int i = 0; i < (int) refShapes.size(); i++)
	{
		ConvexShape const& shape = refShapes[i];
		if (IsShapeIntersectinPlane(plane, shape))
		{
			vector.push_back(shape);
		}
	}
}


bool Game::IsShapeInFrontOfPlane(Plane2 const& plane, ConvexShape const& shape)
{
	ConvexPoly2 poly = shape.m_convexPoly;
	for (int i = 0; i < poly.GetNumPoints(); i++)
	{
		Vec2 pt = poly.GetPointAtIndex(i);
		if (plane.IsPointBehind(pt))
		{
			return false;
		}
	}

	return true;
}


bool Game::IsShapeBehindPlane(Plane2 const& plane, ConvexShape const& shape)
{
	ConvexPoly2 poly = shape.m_convexPoly;
	for (int i = 0; i < poly.GetNumPoints(); i++)
	{
		Vec2 pt = poly.GetPointAtIndex(i);
		if (plane.IsPointInFront(pt))
		{
			return false;
		}
	}

	return true;
}


bool Game::IsShapeIntersectinPlane(Plane2 const& plane, ConvexShape const& shape)
{
	bool hasPointsInFrontOfPlane = false;
	bool hasPointsBehindPlane = false;
	ConvexPoly2 poly = shape.m_convexPoly;
	for (int i = 0; i < poly.GetNumPoints(); i++)
	{
		Vec2 pt = poly.GetPointAtIndex(i);
		if (plane.IsPointInFront(pt))
		{
			hasPointsInFrontOfPlane = true;
		}

		if (plane.IsPointBehind(pt))
		{
			hasPointsBehindPlane = true;
		}
	}

	return hasPointsBehindPlane && hasPointsInFrontOfPlane;
}


void Game::CalculateBSPTreeDepth(BSPNode* node, int& out_treeDepth)
{
	//this function is wrong and will give the wrong answer

	if (node == nullptr)
		return;

	if (node->m_frontNode != nullptr)
	{
		CalculateBSPTreeDepth(node->m_frontNode, out_treeDepth);
	}
	else if (node->m_backNode != nullptr)
	{
		CalculateBSPTreeDepth(node->m_backNode, out_treeDepth);
	}

	if (node->m_parentNode != nullptr)
		out_treeDepth++;
}


void Game::ClearBSPTree(BSPNode*& node)
{
	if (node == nullptr)
		return;

	if (node->m_backNode == nullptr && node->m_frontNode == nullptr)
	{
		delete node;
		node = nullptr;
	}
	else
	{
		ClearBSPTree(node->m_backNode);
		ClearBSPTree(node->m_frontNode);
		delete node;
		node = nullptr;
	}
}


void Game::GoUpTheTreeAndFindPointsOfIntersectionBetweenPartitionPlanes(std::vector<Vec2>& pointsOfIntersection, BSPNode* node)
{
	Plane2 nodePartitionPlane = node->m_partitionInfo.m_partitionPlane;
	BSPNode* parentNode = node->m_parentNode;
	while (parentNode != nullptr)
	{
		Plane2 parentPartitionPlane = parentNode->m_partitionInfo.m_partitionPlane;
		Vec2 poi;
		if (parentPartitionPlane.GetPointOfIntersection(nodePartitionPlane, poi))
		{
			pointsOfIntersection.push_back(poi);
		}

		parentNode = parentNode->m_parentNode;
	}
}


bool Game::IsPointOffScreen(Vec2 const& pt1)
{
	return pt1.x < m_worldOrthoBottomLeft.x || pt1.y < m_worldOrthoBottomLeft.y || pt1.x > m_worldOrthoTopRight.x || pt1.y > m_worldOrthoTopRight.y;
}


bool Game::ArePlanesVirtuallyEqual(Plane2 const& plane1, Plane2 const& plane2)
{
	return plane1 == plane2 || plane1 == -plane2;
}


void Game::CreatePartitionInNode(BSPNode*& bspNode)
{
	std::vector<ConvexShape> const& shapes = bspNode->m_shapesInNode;
	std::vector<BSPPartitionStruct> partitionData;
	for (int shapeIndex = 0; shapeIndex < (int) shapes.size(); shapeIndex++)
	{
		ConvexShape const& shape = shapes[shapeIndex];
		ConvexHull2 const& convexHull = shape.m_convexHull;
		for (int planeIndex = 0; planeIndex < convexHull.m_unorderedPlanes.size(); planeIndex++)
		{
			Plane2 const& plane = convexHull.m_unorderedPlanes[planeIndex];
			BSPPartitionStruct partitionInfo = {};
			partitionInfo.m_refShape = shape;
			partitionInfo.m_partitionPlane = plane;
			
			std::vector<ConvexShape> shapesInFrontOfPlane;
			AddShapesInFrontOfPlaneToVector(shapesInFrontOfPlane, plane, shapes);
			partitionInfo.m_numShapesInFront = (int) shapesInFrontOfPlane.size();

			std::vector<ConvexShape> shapesBehindPlane;
			AddShapesBehindPlaneToVector(shapesBehindPlane, plane, shapes);
			partitionInfo.m_numShapesBehind = (int) shapesBehindPlane.size();
			
			std::vector<ConvexShape> shapesIntersectingPlane;
			AddShapesIntersectionPlaneToVector(shapesIntersectingPlane, plane, shapes);
			partitionInfo.m_numIntersectingShapes = (int) shapesIntersectingPlane.size();

			partitionData.push_back(partitionInfo);
		}
	}

	BSPPartitionStruct bestPartition = GetBestPartition(partitionData, bspNode);
	Plane2 bestPartitionPlane = bestPartition.m_partitionPlane;
	if (bestPartitionPlane.m_fwdNormal == Vec2::ZERO && bestPartitionPlane.m_distanceFromOrigin == 0.0f)
	{
		bspNode->m_noMorePartitionsPossible = true;
		return;
	}


	//bspNode->m_partitionPlane = bestPartition.m_partitionPlane;
	bspNode->m_partitionInfo = bestPartition;
	m_bspTree.m_partitionPlanes.push_back(bestPartition.m_partitionPlane);
	//bspNode->m_partitionPlaneRefShape = bestPartition.m_refShape;
}


bool Game::IsPlaneInBSPTree(Plane2 const& plane)
{
	for (int i = 0; i < (int) m_bspTree.m_partitionPlanes.size(); i++)
	{
		if (ArePlanesVirtuallyEqual(plane, m_bspTree.m_partitionPlanes[i]))
		{
			return true;
		}
	}

	return false;
}


bool Game::IsPartitionOutsideNode(BSPPartitionStruct const& partitionInfo, BSPNode* node)
{
	Plane2 partitionPlane = partitionInfo.m_partitionPlane;
	ConvexShape refShape = partitionInfo.m_refShape;
	Vec2 nearestPtOnPlane = GetNearestPointOnPlane2(refShape.m_boundingDiskCenter, partitionPlane);
	BSPNode* childNode = node;
	BSPNode* parentNode = node->m_parentNode;
	while (parentNode != nullptr)
	{
		Plane2 parentPartition = parentNode->m_partitionInfo.m_partitionPlane;
		if (childNode->m_isFrontNode)
		{
			if (!parentPartition.IsPointInFront(nearestPtOnPlane))
			{
				return true;
			}
		}
		else
		{
			if (!parentPartition.IsPointBehind(nearestPtOnPlane))
			{
				return true;
			}
		}

		childNode = parentNode;
		parentNode = parentNode->m_parentNode;
	}

	return false;
}


void Game::DisplayBSPTreeData(BSPNode* bspNode)
{
	if (bspNode == nullptr)
		return;

	int numShapes = bspNode->m_numShapesInsideNode;
	int numShapesInFront = bspNode->m_partitionInfo.m_numShapesInFront;
	int numShapesBehind = bspNode->m_partitionInfo.m_numShapesBehind;
	int numShapesIntersecting = bspNode->m_partitionInfo.m_numIntersectingShapes;
	std::string nodeDescription = Stringf("Num Shapes: %i | Num Shapes in front: %i | Num Shapes behind: %i | Num Shapes intersecting: %i", numShapes, numShapesInFront, numShapesBehind, numShapesIntersecting);
	bool isLeafNode = bspNode->m_backNode == nullptr && bspNode->m_frontNode == nullptr;

	if (bspNode->m_parentNode == nullptr)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode(Stringf("RootNode | %s", nodeDescription.c_str()).c_str()))
		{
			if (ImGui::TreeNode(Stringf("Is Leaf Node: %s", isLeafNode ? "True" : "False").c_str()))
			{
				ImGui::TreePop();
			}
			if (ImGui::TreeNode(Stringf("Plane: %.2fx, %.2fy, %.2fd", 
				bspNode->m_partitionInfo.m_partitionPlane.m_fwdNormal.x, bspNode->m_partitionInfo.m_partitionPlane.m_fwdNormal.y, bspNode->m_partitionInfo.m_partitionPlane.m_distanceFromOrigin).c_str()))
			{
				ImGui::TreePop();
			}
			DisplayBSPTreeData(bspNode->m_frontNode);
			DisplayBSPTreeData(bspNode->m_backNode);
			ImGui::TreePop();
		}
	}
	else
	{
		if (bspNode->m_isFrontNode)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode(Stringf("FrontNode | %s", nodeDescription.c_str()).c_str()))
			{
				if (ImGui::TreeNode(Stringf("Is Leaf Node: %s", isLeafNode ? "True" : "False").c_str()))
				{
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(Stringf("Plane: %.2fx, %.2fy, %.2fd",
					bspNode->m_partitionInfo.m_partitionPlane.m_fwdNormal.x, bspNode->m_partitionInfo.m_partitionPlane.m_fwdNormal.y, bspNode->m_partitionInfo.m_partitionPlane.m_distanceFromOrigin).c_str()))
				{
					ImGui::TreePop();
				}
				DisplayBSPTreeData(bspNode->m_frontNode);
				DisplayBSPTreeData(bspNode->m_backNode);
				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode(Stringf("BackNode | %s", nodeDescription.c_str()).c_str()))
			{
				if (ImGui::TreeNode(Stringf("Is Leaf Node: %s", isLeafNode ? "True" : "False").c_str()))
				{
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(Stringf("Plane: %.2fx, %.2fy, %.2fd",
					bspNode->m_partitionInfo.m_partitionPlane.m_fwdNormal.x, bspNode->m_partitionInfo.m_partitionPlane.m_fwdNormal.y, bspNode->m_partitionInfo.m_partitionPlane.m_distanceFromOrigin).c_str()))
				{
					ImGui::TreePop();
				}
				DisplayBSPTreeData(bspNode->m_frontNode);
				DisplayBSPTreeData(bspNode->m_backNode);
				ImGui::TreePop();
			}
		}
	}
}


uint32_t Game::FindChunkOffsetForChunkId(BufferParser& parser, uint32_t tocChunkDataOffset, int numChunks, uint8_t refChunkId)
{
	parser.JumpToOffset(tocChunkDataOffset);
	for (int chunkIndex = 0; chunkIndex < numChunks; chunkIndex++)
	{
		uint8_t chunkId = parser.ParseByte();
		uint32_t chunkStartOffset = parser.ParseUInt32();
		uint32_t chunkSize = parser.ParseUInt32();
		UNUSED(chunkSize);
		if (chunkId == refChunkId)
			return chunkStartOffset;
	}

	return 0xffffffff;
}


uint32_t Game::FindChunkSizeForChunkId(BufferParser& parser, uint32_t tocChunkDataOffset, int numChunks, uint8_t refChunkId)
{
	parser.JumpToOffset(tocChunkDataOffset);
	for (int chunkIndex = 0; chunkIndex < numChunks; chunkIndex++)
	{
		uint8_t chunkId = parser.ParseByte();
		uint32_t chunkStartOffset = parser.ParseUInt32();
		UNUSED(chunkStartOffset);
		uint32_t chunkSize = parser.ParseUInt32();
		if (chunkId == refChunkId)
			return chunkSize;
	}

	return 0xffffffff;
}


void Game::ParseChunk(BufferParser& parser, uint32_t chunkStartOffset, uint8_t chunkIdToParse, uint32_t totalChunkSize)
{
	parser.JumpToOffset(chunkStartOffset);

	const uint32_t CHUNK_SUB_HEADER_SIZE = 10;
	const uint32_t CHUNK_FOOTER_SIZE = 4;
	const uint32_t TOTAL_CHUNK_PADDING = CHUNK_SUB_HEADER_SIZE + CHUNK_FOOTER_SIZE;
	
	char chunkSubHeaderStartG = parser.ParseChar();
	char chunkSubHeaderStartH = parser.ParseChar();
	char chunkSubHeaderStartC = parser.ParseChar();
	char chunkSubHeaderStartK = parser.ParseChar();
	uint8_t chunkId = parser.ParseByte();
	uint8_t chunkEndianModeByte = parser.ParseByte();
	EndinanMode chunkEndianMode = chunkEndianModeByte == 2 ? EndinanMode::ENDIAN_MODE_BIG : EndinanMode::ENDIAN_MODE_LITTLE;
	uint32_t chunkDataSize = parser.ParseUInt32();
	parser.MoveReadPointerForward(chunkDataSize);
	char chunkFooterEndK = parser.ParseChar();
	char chunkFooterEndC = parser.ParseChar();
	char chunkFooterEndH = parser.ParseChar();
	char chunkFooterEndG = parser.ParseChar();
	GUARANTEE_OR_DIE(totalChunkSize - chunkDataSize == TOTAL_CHUNK_PADDING, Stringf("Size of chunk in toc does not match size of chunk in chunk. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkId == chunkIdToParse, Stringf("Chunk id found in toc is different from chunk id found in chunk. Chunk id in toc: %x, chunk id in chunk: %x", chunkIdToParse, chunkId));
	GUARANTEE_OR_DIE(chunkSubHeaderStartG == 'G',  Stringf("Chunk Sub header is incorrect. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkSubHeaderStartH == 'H', Stringf("Chunk Sub header is incorrect. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkSubHeaderStartC == 'C',  Stringf("Chunk Sub header is incorrect. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkSubHeaderStartK == 'K', Stringf("Chunk Sub header is incorrect. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkFooterEndK == 'K', Stringf("Chunk footer is incorrect. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkFooterEndC == 'C', Stringf("Chunk footer is incorrect. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkFooterEndH == 'H', Stringf("Chunk footer is incorrect. Chunk id: %x", chunkId));
	GUARANTEE_OR_DIE(chunkFooterEndG == 'G', Stringf("Chunk footer is incorrect. Chunk id: %x", chunkId));
	parser.MoveReadPointerBackward(CHUNK_FOOTER_SIZE + chunkDataSize);
	parser.SetEndianMode(chunkEndianMode);

	switch (chunkId)
	{
	case 0x01:
	{
		AABB2 worldBounds = parser.ParseAABB2();
		m_numShapes = parser.ParseUInt32();
		m_worldOrthoBottomLeft = worldBounds.m_mins;
		m_worldOrthoTopRight = worldBounds.m_maxs;
		m_worldCamera.SetOrthoView(m_worldOrthoBottomLeft, m_worldOrthoTopRight);
		break;
	}
	case 0x02:
	{
		uint32_t numShapes = parser.ParseUInt32();
		GUARANTEE_OR_DIE(numShapes == (uint32_t) m_numShapes, Stringf("Number of shapes in scene info do not match number of shapes in convex poly chunk"));
		bool isVectorEmpty = (int) m_convexShapes.size() <= 0;
		for (int shapeIndex = 0; shapeIndex < m_numShapes; shapeIndex++)
		{
			ConvexPoly2 convexPoly;
			uint8_t numVerts = parser.ParseByte();
			for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)
			{
				Vec2 vertPos = parser.ParseVec2();
				convexPoly.AddPoint(vertPos);
			}

			if (isVectorEmpty)
			{
				ConvexShape shape = {};
				shape.m_convexPoly = convexPoly;
				m_convexShapes.push_back(shape);
			}
			else
			{
				m_convexShapes[shapeIndex].m_convexPoly = convexPoly;
			}
		}
		break;
	}
	case 0x03:
	{
		uint32_t numShapes = parser.ParseUInt32();
		GUARANTEE_OR_DIE(numShapes == (uint32_t) m_numShapes, Stringf("Number of shapes in scene info do not match number of shapes in convex hull chunk"));
		bool isVectorEmpty = (int) m_convexShapes.size() <= 0;
		for (int shapeIndex = 0; shapeIndex < m_numShapes; shapeIndex++)
		{
			ConvexHull2 convexHull;
			uint8_t numPlanes = parser.ParseByte();
			for (int planeIndex = 0; planeIndex < numPlanes; planeIndex++)
			{
				Plane2 plane = parser.ParsePlane2();
				convexHull.m_unorderedPlanes.push_back(plane);
			}
		
			if (isVectorEmpty)
			{
				ConvexShape shape = {};
				shape.m_convexHull = convexHull;
				m_convexShapes.push_back(shape);
			}
			else
			{
				m_convexShapes[shapeIndex].m_convexHull = convexHull;
			}
		}
		break;
	}
	case 0x81:
	{
		uint32_t numShapes = parser.ParseUInt32();
		GUARANTEE_OR_DIE(numShapes == (uint32_t) m_numShapes, Stringf("Number of shapes in scene info do not match number of shapes in convex hull chunk"));
		bool isVectorEmpty = (int) m_convexShapes.size() <= 0;
		for (int shapeIndex = 0; shapeIndex < m_numShapes; shapeIndex++)
		{
			Vec2 boundingDiskCenter = parser.ParseVec2();
			float boundingDiskRadius = parser.ParseFloat();

			if (isVectorEmpty)
			{
				ConvexShape shape = {};
				shape.m_boundindDiskRadius = boundingDiskRadius;
				shape.m_boundingDiskCenter = boundingDiskCenter;
				m_convexShapes.push_back(shape);
			}
			else
			{
				m_convexShapes[shapeIndex].m_boundindDiskRadius = boundingDiskRadius;
				m_convexShapes[shapeIndex].m_boundingDiskCenter = boundingDiskCenter;
			}
		}
		break;
	}
	default:
		break;
	}
}


void Game::WriteBSPTreeToBuffer(BufferWriter& writer, uint32_t& out_byteSizeOfTree)
{
	writer.AppendByte((uint8_t) m_bspTree.m_treeDepth);
	writer.AppendByte((uint8_t) m_bspTree.m_numLeafNodes);
	writer.AppendByte((uint8_t) m_bspTree.m_numNodes);
	out_byteSizeOfTree += 3;
	BSPNode* rootNode = m_bspTree.m_rootNode;
	uint8_t rootNodeIndex = 0;
	WriteBSPNodeToBuffer(writer, rootNode, rootNodeIndex, out_byteSizeOfTree);
}


void Game::WriteBSPNodeToBuffer(BufferWriter& writer, BSPNode* node, uint8_t& nodeIndex, uint32_t& out_byteSizeOfNode)
{
	if (node->m_parentNode == nullptr)
	{
		writer.AppendByte(0xff);
		out_byteSizeOfNode += 1;
	}
	else
	{
		writer.AppendByte(nodeIndex - 1);
		out_byteSizeOfNode += 1;
	}

	if (node->m_frontNode == nullptr)
	{

	}
}


void Game::RenderBSPNode(BSPNode* node, Rgba8 const& partitionColor)
{
	if (node == nullptr)
		return;

	Rgba8 fwdNormalColor = Rgba8::MAGENTA;
	float width = m_worldLineWidth;
	if (node->m_frontNode && node->m_backNode)
	{
		if (node->m_parentNode == nullptr)
		{
			DrawPlane2(node->m_partitionInfo.m_partitionPlane, width, partitionColor, fwdNormalColor, true);
		}
		else
		{
			Plane2 partitionPlane = node->m_partitionInfo.m_partitionPlane;
			Vec2 planeFwdNormal = partitionPlane.m_fwdNormal;

			//DrawPlane2(node->m_partitionPlane, width, partitionColor);

 			std::vector<Vec2> pointsOfIntersection;
 			GoUpTheTreeAndFindPointsOfIntersectionBetweenPartitionPlanes(pointsOfIntersection, node);
			int numPtsOfIntersection = (int) pointsOfIntersection.size();
			if (numPtsOfIntersection > 0)
			{
				Vec2 refPt = node->m_partitionInfo.m_refShape.m_boundingDiskCenter;
				Vec2 nearestPt1;
				float nearestDistance1 = 999999.0f;
				int nearestPt1Index = 0;
				for (int i = 0; i < numPtsOfIntersection; i++)
				{
					Vec2 poi = pointsOfIntersection[i];
					float distBwPts = (poi - refPt).GetLength();
					if (distBwPts < nearestDistance1)
					{
						nearestPt1 = poi;
						nearestDistance1 = distBwPts;
						nearestPt1Index = i;
					}
				}

				if (numPtsOfIntersection > 1)
				{
					Vec2 arrowStart = GetNearestPointOnPlane2(node->m_partitionInfo.m_refShape.m_boundingDiskCenter, partitionPlane);
					Vec2 nearestPt2;
					float nearestDistance2 = 999'999.0f;
					for (int i = 0; i < numPtsOfIntersection; i++)
					{
						Vec2 poi = pointsOfIntersection[i];
						if (poi != nearestPt1)
						{
							Vec2 a = nearestPt1 - arrowStart;
							Vec2 b = poi - arrowStart;
							if (DotProduct2D(a, b) >= 0.0f)
							{
								continue;
							}

							float distBwPts = (poi - refPt).GetLength();
							if (distBwPts < nearestDistance2)
							{
								nearestPt2 = poi;
								nearestDistance2 = distBwPts;
							}
						}
					}

					if (nearestDistance2 == 999999.0f)
					{
						int nearestPt2Index = numPtsOfIntersection - 1 - nearestPt1Index;
						if (nearestPt2Index == nearestPt1Index)
						{
							nearestPt2Index--;
						}
						nearestPt2 = pointsOfIntersection[nearestPt2Index];
					}

//  				m_pointsOfIntersection.push_back(nearestPt1);
// 					m_pointsOfIntersection.push_back(nearestPt2);
					Vec2 a = nearestPt1 - arrowStart;
					Vec2 b = nearestPt2 - arrowStart;
					if (DotProduct2D(a, b) < 0.0f)
					{
						DrawLine(nearestPt1, nearestPt2 - nearestPt1, width, partitionColor);
					}
					else
					{
						Vec2 aNormal = a.GetNormalized();
						Vec2 pt2 = arrowStart - aNormal * 1000.0f;
						DrawLine(nearestPt1, pt2 - nearestPt1, width, partitionColor);
					}
					Vec2 arrowEnd = arrowStart + partitionPlane.m_fwdNormal * ARROW_LENGTH;
					DrawArrow(arrowStart, arrowEnd, fwdNormalColor, m_worldLineWidth, ARROW_TIP_LENGTH);
				}
				else
				{
					BSPNode* parentNode = node->m_parentNode;
					Plane2 parentPartitionPlane = parentNode->m_partitionInfo.m_partitionPlane;
					Vec2 parentPlaneFwdNormal = parentPartitionPlane.m_fwdNormal;
					Vec2 planeVecNormal = planeFwdNormal.GetRotated90Degrees();
					if ( (node->m_isFrontNode && DotProduct2D(planeVecNormal, parentPlaneFwdNormal) < 0.0f )
						|| (!node->m_isFrontNode && DotProduct2D(planeVecNormal, parentPlaneFwdNormal) > 0.0f))
					{
						planeVecNormal *= -1.0f;
					}

					//m_pointsOfIntersection.push_back(nearestPt1);
					Vec2 pt1 = nearestPt1;
					Vec2 pt2 = pt1 + planeVecNormal * 100000.0f;
					DrawLine(pt1, pt2 - pt1, width, partitionColor);
					Vec2 arrowStart = GetNearestPointOnPlane2(node->m_partitionInfo.m_refShape.m_boundingDiskCenter, partitionPlane);
					Vec2 arrowEnd = arrowStart + partitionPlane.m_fwdNormal * ARROW_LENGTH;
					DrawArrow(arrowStart, arrowEnd, fwdNormalColor, m_worldLineWidth, ARROW_TIP_LENGTH);
				}
			}
			else
			{
				DrawPlane2(partitionPlane, width, partitionColor, fwdNormalColor, true);
			}
		}

		Rgba8 childNodeParitionColor;
		if (partitionColor == Rgba8::RED)
		{
			childNodeParitionColor = Rgba8::BLUE;
		}
		else if (partitionColor == Rgba8::BLUE)
		{
			childNodeParitionColor = Rgba8::GREEN;
		}
		else if (partitionColor == Rgba8::GREEN)
		{
			childNodeParitionColor = Rgba8::GREY;
		}
		else if (partitionColor == Rgba8::GREY)
		{
			childNodeParitionColor = Rgba8::RED;
		}

		RenderBSPNode(node->m_frontNode, childNodeParitionColor);
		RenderBSPNode(node->m_backNode, childNodeParitionColor);
	}
}
