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
#include "Engine/Window/Window.hpp"
#include <vector>
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"

RandomNumberGenerator* g_rng = nullptr;

bool SetZoomSpeed(EventArgs& args)
{
	float zoomSpeed = args.GetValue("zoom", 1.0f);
	g_game->SetOrthoZoomSpeed(zoomSpeed);
	return true;
}


Game::Game()
{

}


Game::~Game()
{
}


void Game::Startup()
{
	g_eventSystem->SubscribeEventCallbackFunction("SetZoomSpeed", SetZoomSpeed);

	DebugRenderSetVisible();

	MouseConfig mouseConfig;
	mouseConfig.m_isHidden = true;
	mouseConfig.m_isLocked = true;
	mouseConfig.m_isRelative = true;
	mouseConfig.m_priority = 1;
	g_inputSystem->PushMouseConfig(mouseConfig);

	g_rng = new RandomNumberGenerator();
	m_fov = g_gameConfigBlackboard.GetValue("fov", m_fov);
	m_zNear = g_gameConfigBlackboard.GetValue("zNear", m_zNear);
	m_zFar = g_gameConfigBlackboard.GetValue("zFar", m_zFar);
	m_raycast3DLength = g_gameConfigBlackboard.GetValue("raycast3DLength", 0.0f);

	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
	m_orthoBottomLeft = Vec2(0.0f, 0.0f);
	m_orthoTopRight = Vec2(worldX, worldY);
	m_orthoZoomSpeed = g_gameConfigBlackboard.GetValue("orthoZoomSpeed", 0.0f);

	Vec2 worldCenter = Vec2(worldX, worldY) * 0.5f;
	m_referencePoint		= worldCenter;
	m_referencePointRadius	= 0.8f;
	m_referencePointSpeed	= 15.0f;

	m_raycast2DStart = worldCenter - Vec2(40.0f, 0.0f);
	m_raycast2DEnd = worldCenter + Vec2(40.0f, 0.0f);

	m_nearestPointColor		= Rgba8::YELLOW;
	m_shapeColor			= Rgba8(34, 65, 115, 255);
	m_highlightedShapeColor	= Rgba8(64, 122, 214, 255);

	RandomizeShapesForNearestPoint2D();
	UpdateVertexVectorNearestPoint2D();
	RandomizeDisksForRaycast2D();
	RandomizeAABB2sForRaycast();
	RandomizeShapes3D();
	RandomizeLineSegment2sForRaycast();

	m_pachinko2D.Init();
	m_gameMode = GAME_MODE_RAYCASTS_AABB2;
	SetGameMode(m_gameMode);
}



void Game::ShutDown()
{
	g_eventSystem->UnsubscribeEventCallbackFunction("SetZoomSpeed", SetZoomSpeed);

	g_inputSystem->PopMouseConfigOfPriority(1);

	delete g_rng;
	g_rng = nullptr;
}


void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void Game::CheckDebugCheatsInput()
{
	if (g_inputSystem->WasKeyJustPressed('Y'))
	{
		SetTimeScale(4.0f);
	}
	else if (g_inputSystem->WasKeyJustPressed('T'))
	{
		SetTimeScale(0.2f);
	}
	else if (g_inputSystem->WasKeyJustPressed('R'))
	{
		SetTimeScale(1.0f);
	}

	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_clock.TogglePause();
	}

	if (g_inputSystem->WasKeyJustPressed('O'))
	{
		m_clock.StepFrame();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}


void Game::CheckInput(float deltaSeconds)
{
	switch (m_gameMode)
	{
		case GAME_MODE_NEAREST_POINT_2D:
		{
			if (g_inputSystem->IsKeyPressed('W'))
			{
				m_referencePoint += Vec2(0.0f, 1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('A'))
			{
				m_referencePoint += Vec2(-1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('S'))
			{
				m_referencePoint += Vec2(0.0f, -1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('D'))
			{
				m_referencePoint += Vec2(1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
			{
				RandomizeShapesForNearestPoint2D();
			}
			break;
		}
		case GAME_MODE_RAYCASTS_Disk2D:
		{
			if (g_inputSystem->IsKeyPressed('W'))
			{
				m_raycast2DStart += Vec2(0.0f, 1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('A'))
			{
				m_raycast2DStart += Vec2(-1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('S'))
			{
				m_raycast2DStart += Vec2(0.0f, -1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('D'))
			{
				m_raycast2DStart += Vec2(1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('I'))
			{
				m_raycast2DEnd += Vec2(0.0f, 1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('J'))
			{
				m_raycast2DEnd += Vec2(-1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('K'))
			{
				m_raycast2DEnd += Vec2(0.0f, -1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('L'))
			{
				m_raycast2DEnd += Vec2(1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
			{
				RandomizeDisksForRaycast2D();
			}
			break;
		}
		case GAME_MODE_RAYCASTS_AABB2:
		{
			if (g_inputSystem->IsKeyPressed('W'))
			{
				m_raycast2DStart += Vec2(0.0f, 1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('A'))
			{
				m_raycast2DStart += Vec2(-1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('S'))
			{
				m_raycast2DStart += Vec2(0.0f, -1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('D'))
			{
				m_raycast2DStart += Vec2(1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('I'))
			{
				m_raycast2DEnd += Vec2(0.0f, 1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('J'))
			{
				m_raycast2DEnd += Vec2(-1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('K'))
			{
				m_raycast2DEnd += Vec2(0.0f, -1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('L'))
			{
				m_raycast2DEnd += Vec2(1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
			{
				RandomizeAABB2sForRaycast();
			}
			break;
		}
		case GAME_MODE_RAYCASTS_LINESEGMENT2:
		{
			if (g_inputSystem->IsKeyPressed('W'))
			{
				m_raycast2DStart += Vec2(0.0f, 1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('A'))
			{
				m_raycast2DStart += Vec2(-1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('S'))
			{
				m_raycast2DStart += Vec2(0.0f, -1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('D'))
			{
				m_raycast2DStart += Vec2(1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('I'))
			{
				m_raycast2DEnd += Vec2(0.0f, 1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('J'))
			{
				m_raycast2DEnd += Vec2(-1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('K'))
			{
				m_raycast2DEnd += Vec2(0.0f, -1.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('L'))
			{
				m_raycast2DEnd += Vec2(1.0f, 0.0f) * m_referencePointSpeed * deltaSeconds;
			}

			if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
			{
				RandomizeLineSegment2sForRaycast();
			}
			break;
		}
		case GAME_MODE_3D:
		{
			float movementSpeed = 10.0f;

			Vec3 xAxis = Vec3(1.0f, 0.0f, 0.0f);
			Vec3 yAxis = Vec3(0.0f, 1.0f, 0.0f);
			Vec3 zAxis = Vec3(0.0f, 0.0f, 1.0f);

			Vec3 camForward;
			Vec3 camLeft;
			Vec3 camUp;
			m_persCamAngles.GetAsVectors_XFwd_YLeft_ZUp(camForward, camLeft, camUp);

			Vec3 translation = Vec3(0.0f, 0.0f, 0.0f);
			if (g_inputSystem->IsKeyPressed('A'))
			{
				translation += Vec3(camLeft.x, camLeft.y, 0.0f).GetNormalized() * movementSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('D'))
			{
				translation += -Vec3(camLeft.x, camLeft.y, 0.0f).GetNormalized() * movementSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('W'))
			{
				translation += Vec3(camForward.x, camForward.y, 0.0f).GetNormalized() * movementSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('S'))
			{
				translation += -Vec3(camForward.x, camForward.y, 0.0f).GetNormalized() * movementSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('E'))
			{
				translation += zAxis * movementSpeed * deltaSeconds;
			}

			if (g_inputSystem->IsKeyPressed('Q'))
			{
				translation += zAxis * -movementSpeed * deltaSeconds;
			}
			m_persCamPosition += translation;
			if (m_isObjectPickedUp && m_pickedUpObject != nullptr)
			{
				m_pickedUpObject->Translate(translation);
				Mat44 translationMat = Mat44::CreateTranslation3D(translation);
				if (m_pickedUpObject->m_isWire)
				{
					TransformVertexArray3D(m_pickedUpObject->m_vertCount, &m_wire3dShapesVerts[m_pickedUpObject->m_vertsStartIndex], translationMat);
				}
				else
				{
					TransformVertexArray3D(m_pickedUpObject->m_vertCount, &m_solid3dShapesVerts[m_pickedUpObject->m_vertsStartIndex], translationMat);
				}
			}

			if (g_inputSystem->WasMouseButtonJustPressed(0))
			{
				if (m_impactShape3D != nullptr)
				{
					m_isObjectPickedUp = !m_isObjectPickedUp;
					if (m_isObjectPickedUp)
					{
						m_pickedUpObject = m_impactShape3D;
					}
				}
				else if (m_isObjectPickedUp)
				{
					m_isObjectPickedUp = false;
					m_pickedUpObject = nullptr;
				}
			}

			if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR))
			{
				if (m_isRefPointParked)
					UnParkReferencePointNearestPoint3D();
				else
					ParkReferencePointNearestPoint3D();
			}


			if (g_inputSystem->WasKeyJustPressed(KEYCODE_CTRL))
			{
				m_persCamPosition = Vec3(0.0f, 0.0f, 0.0f);
				m_persCamAngles = EulerAngles();
			}

			if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
			{
				RandomizeShapes3D();
			}

			float sensitivity = 0.1f;
			IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
			m_persCamAngles.m_yaw += -mouseDelta.x  * sensitivity;
			m_persCamAngles.m_pitch += mouseDelta.y  * sensitivity;
			m_persCamAngles.m_pitch = Clamp(m_persCamAngles.m_pitch, -89.9f, 89.9f);
			break;
		}
		case NUM_GAME_MODES:
			break;
		default:
			break;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F7))
	{
		CycleGameMode();
	}
	
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F6))
	{
		CycleGameMode(false);
	}

	if (g_inputSystem->IsKeyPressed('B') && m_camMode == CAMERA_MODE_ORTHOGRAPHIC)
	{
		Vec2 dims = m_orthoTopRight - m_orthoBottomLeft;
		Vec2 newDims = dims * m_orthoZoomSpeed;
		Vec2 difference = newDims - dims;
		m_orthoBottomLeft += difference * 0.5f;
		m_orthoTopRight -= difference * 0.5f;
	}

	if (g_inputSystem->IsKeyPressed('V') && m_camMode == CAMERA_MODE_ORTHOGRAPHIC)
	{
		Vec2 dims = m_orthoTopRight - m_orthoBottomLeft;
		Vec2 newDims = dims / m_orthoZoomSpeed;
		Vec2 difference = dims - newDims;
		m_orthoBottomLeft -= difference * 0.5f;
		m_orthoTopRight += difference * 0.5f;
	}
}


void Game::Update()
{
	CheckDebugCheatsInput();
	float deltaSeconds = (float) m_clock.GetFrameDeltaSeconds();

	CheckInput(deltaSeconds);
	UpdateCamera();

	m_verts.clear();
	switch (m_gameMode)
	{
		case GAME_MODE_NEAREST_POINT_2D:
		{
			UpdateVertexVectorNearestPoint2D();
			break;
		}
		case GAME_MODE_RAYCASTS_Disk2D:
		{
			RaycastAllDisks2D();
			UpdateVertexVectorRaycastVDisk2D();
			break;
		}
		case GAME_MODE_RAYCASTS_AABB2:
		{
			RaycastAllAABB2s();
			UpdateVertexVectorRaycastVAABB2();
			break;
		}
		case GAME_MODE_RAYCASTS_LINESEGMENT2:
		{
			RaycastAllLineSegment2s();
			UpdateVertexVectorRaycastVLineSegment2();
			break;
		}
		case GAME_MODE_BILLIARDS_2D: 
		{
			m_billiards2D.Update(deltaSeconds);
		}
		case GAME_MODE_PACHINKO_2D:
		{
			m_pachinko2D.Update(deltaSeconds);
		}
		case GAME_MODE_3D:
		{
			DebugAddWorldBasis(Mat44(), 0.0f);

			Vec3 fwd, left, up;
			m_persCamAngles.GetAsVectors_XFwd_YLeft_ZUp(fwd, left, up);
			float fwdOffset = 0.5f;
			Vec3 inFrontOfPlayer = m_persCamPosition + fwd * fwdOffset;
			float basisLength = 0.01f;
			float basisThickness = 0.003f;
			DebugAddWorldLine(inFrontOfPlayer, inFrontOfPlayer + Vec3(1.0f, 0.0f, 0.0f) * basisLength, Rgba8::RED, basisThickness, 0.0f, DebugRenderMode::ALWAYS);
			DebugAddWorldLine(inFrontOfPlayer, inFrontOfPlayer + Vec3(0.0f, 1.0f, 0.0f) * basisLength, Rgba8::GREEN, basisThickness, 0.0f, DebugRenderMode::ALWAYS);
			DebugAddWorldLine(inFrontOfPlayer, inFrontOfPlayer + Vec3(0.0f, 0.0f, 1.0f) * basisLength, Rgba8::BLUE, basisThickness, 0.0f, DebugRenderMode::ALWAYS);

			UpdateNearestPointVerts3D();
			CheckAndUpdateVertAlphasBasedOnOverlaps();
			RaycastAll3DShapes();

			if (m_isObjectPickedUp)
			{
				Rgba8 pickedColor = Rgba8(255, 0, 0);
				if (m_pickedUpObject->m_isWire)
				{
					TransformVertexArrayColor(m_pickedUpObject->m_vertCount, &m_wire3dShapesVerts[m_pickedUpObject->m_vertsStartIndex], pickedColor);
				}
				else
				{
					pickedColor = pickedColor * m_pickedUpObject->m_color;
					TransformVertexArrayColor(m_pickedUpObject->m_vertCount, &m_solid3dShapesVerts[m_pickedUpObject->m_vertsStartIndex], pickedColor);
				}
			}
			break;
		}
		case NUM_GAME_MODES:
		default:
			break;
	}

	DisplayGameModeAndControls(m_gameMode);
}


void Game::UpdateCamera()
{
	switch (m_camMode)
	{
		case CAMERA_MODE_ORTHOGRAPHIC:
		{
			m_renderCamera.SetOrthoView(m_orthoBottomLeft, m_orthoTopRight);
			break;
		}
		case CAMERA_MODE_PERSPECTIVE:
		{
			m_renderCamera.DefineGameSpace(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));
			Mat44 persMatrix = Mat44::CreatePerspectiveProjectionMatrix(m_fov, g_window->GetWindowConfig().m_aspectRatio, m_zNear, m_zFar);
			m_renderCamera.SetProjectionMatrix(persMatrix);
			Mat44 camModelMat = Mat44::CreateTranslation3D(m_persCamPosition);
			Mat44 rotation = m_persCamAngles.GetAsMatrix_XFwd_YLeft_ZUp();
			camModelMat.Append(rotation);
			m_renderCamera.SetCameraModelMatrix(camModelMat);
			break;
		}
		case NUM_CAMERA_MODES:
		default:
			break;
	}
}


void Game::ToggleCameraMode()
{
	if (m_camMode == CAMERA_MODE_ORTHOGRAPHIC)
	{
		m_camMode = CAMERA_MODE_PERSPECTIVE;
	}
	else
	{
		m_camMode = CAMERA_MODE_ORTHOGRAPHIC;
	}
}


void Game::SetGameMode(GameMode newMode)
{
	m_gameMode = newMode;
	if (m_gameMode < GAME_MODE_3D)
	{
		SetCameraMode(CAMERA_MODE_ORTHOGRAPHIC);

		float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
		float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
		m_orthoBottomLeft = Vec2(0.0f, 0.0f);
		m_orthoTopRight = Vec2(worldX, worldY);
	}
	if (m_gameMode >= (int) GAME_MODE_3D)
	{
		SetCameraMode(CAMERA_MODE_PERSPECTIVE);
	}
}


void Game::CycleGameMode(bool goForward /*= true*/)
{
	int gameMode = (int) m_gameMode;
	gameMode += goForward ? 1 : -1;
	if (gameMode < 0)
		gameMode = (int) NUM_GAME_MODES - 1;
	else
		gameMode %= (int) NUM_GAME_MODES;

	SetGameMode((GameMode) gameMode);
}


void Game::SetOrthoZoomSpeed(float zoomSpeed)
{
	m_orthoZoomSpeed = zoomSpeed;
}


Clock const& Game::GetClock() const
{
	return m_clock;
}


void Game::DisplayGameModeAndControls(GameMode mode)
{
	float cursorY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	DebugAddScreenText("F6/F7: cycle through modes ", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
	
	cursorY -= 14.0f;
	switch (mode)
	{
		case GAME_MODE_NEAREST_POINT_2D:
			DebugAddScreenText("Game Mode: Nearest Point 2D", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
			break;
		case GAME_MODE_RAYCASTS_Disk2D:
			DebugAddScreenText("Game Mode: Raycast v Disks 2D", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
			break;
		case  GAME_MODE_RAYCASTS_AABB2:
			DebugAddScreenText("Game Mode: Raycast v AABB2", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
		break;
		case  GAME_MODE_RAYCASTS_LINESEGMENT2:
			DebugAddScreenText("Game Mode: Raycast v LineSegment2", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
			break;
		case GAME_MODE_BILLIARDS_2D:
			DebugAddScreenText("Game Mode: Billiards2D", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
			break;
		case GAME_MODE_PACHINKO_2D:
			DebugAddScreenText("Game Mode: Pachinko2D", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
			break;
		case GAME_MODE_3D:
			DebugAddScreenText("Game Mode: 3D", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
			break;
		case NUM_GAME_MODES:
		default:
			DebugAddScreenText("Invalid Game Mode", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::YELLOW);
	}
	
	//controls
	cursorY -= 14.0f;
	switch (mode)
	{
		case GAME_MODE_NEAREST_POINT_2D:
			DebugAddScreenText("WASD: move reference point | F8: Randomize shapes", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			break;
		case GAME_MODE_RAYCASTS_Disk2D:
			DebugAddScreenText("WASD: move raycast start | IJKL: move raycast end | F8: Randomize disks", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			break;
		case  GAME_MODE_RAYCASTS_AABB2:
			DebugAddScreenText("WASD: move raycast start | IJKL: move raycast end | F8: Randomize aabb2s", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			break;
		case  GAME_MODE_RAYCASTS_LINESEGMENT2:
			DebugAddScreenText("WASD: move raycast start | IJKL: move raycast end | F8: Randomize line segments", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			break;
		case GAME_MODE_BILLIARDS_2D:
			DebugAddScreenText("WASD: move raycast start | IJKL: move raycast end | Spacebar: Launch billiard | F8: Randomize bumpers and reset billiards", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			cursorY -= 14.0f;
			DebugAddScreenText("QE: decrease/Increase billiard elasticity | F3: clamp/unclamp billiard elasticity | F2: reset billiard elasticity to 0.9", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			break;
		case GAME_MODE_PACHINKO_2D:
			DebugAddScreenText("WASD: move spawn point | IJKL: move spawn velocity | Spacebar: Launch billiard | F: Toggle floor", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			cursorY -= 14.0f;
			DebugAddScreenText("F8: Randomize bumpers and reset billiards | F4: Toggle physics controls", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			break;
		case GAME_MODE_3D:
		{
			DebugAddScreenText("WASD: move cam(reference point) along XY plane | QE: move along Z (absolute up/down) | Mouse: look around", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			cursorY -= 14.0f;
			DebugAddScreenText("Spacebar: Park/Unpark reference point | CTRL: Reset camera to origin | F8: Randomize shapes | Left Mouse Button: Pickup/Drop shapes", Vec2(0.0f, cursorY), 0.0f, Vec2(0.0f, 1.0f), 12.0f, Rgba8::GREEN);
			break;
		}
		case NUM_GAME_MODES:
		default:
			break;
	}
}


void Game::SetCameraMode(CameraMode newMode)
{
	m_camMode = newMode;
}


void Game::Render() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	g_theRenderer->BeginCamera(m_renderCamera);	

	switch (m_gameMode)
	{
		case GAME_MODE_NEAREST_POINT_2D:
			break;
		case GAME_MODE_RAYCASTS_Disk2D:
			break;
		case GAME_MODE_RAYCASTS_AABB2:
			break;
		case GAME_MODE_RAYCASTS_LINESEGMENT2:
			break;
		case GAME_MODE_BILLIARDS_2D:
			m_billiards2D.Render();
			break;
		case GAME_MODE_PACHINKO_2D:
			m_pachinko2D.Render();
			break;
		case GAME_MODE_3D:
		{
			Texture* defaultDepthStencil = g_theRenderer->CreateOrGetDepthStencil("Default");
			g_theRenderer->ClearDepth(defaultDepthStencil, 1.0f, 0);
			g_theRenderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
			g_theRenderer->SetCullMode(CullMode::BACK);
			Texture* texture = g_theRenderer->CreateOrGetTexture("Data/Images/Test_StbiFlippedAndOpenGL.png");
			g_theRenderer->BindTexture(0, texture);
			g_theRenderer->DrawVertexArray((int) m_solid3dShapesVerts.size(), m_solid3dShapesVerts.data());
			g_theRenderer->BindTexture(0, nullptr);
			g_theRenderer->DrawVertexArray((int) m_wire3dShapesVerts.size(), m_wire3dShapesVerts.data());
			break;
		}
		case NUM_GAME_MODES:
		default:
			break;
	}

	g_theRenderer->DrawVertexArray((int) m_verts.size(), m_verts.data());
	g_theRenderer->EndCamera(m_renderCamera);

	if (m_camMode == CAMERA_MODE_PERSPECTIVE)
	{
		DebugRenderWorldToCamera(m_renderCamera);
	}
}


void Game::RandomizeShapesForNearestPoint2D()
{
	RandomizeDisk2D(m_disk2dCenter, m_disk2dRadius);
	RandomizeAABB2D(m_aabb2);
	RandomizeCapsule2D(m_capsule2d);
	RandomizeOBB2D(m_obb2);
	RandomizeLineSegment2D(m_finiteLine2d);
	RandomizeLineSegment2D(m_infiniteLine2d);
}


void Game::RandomizeDisk2D(Vec2& out_diskCenter, float& out_diskRadius)
{
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
	
	float randomCenterX = g_rng->GetRandomFloatInRange(40.0f, worldX - 40.0f);
	float randomCenterY = g_rng->GetRandomFloatInRange(20.0f, worldY - 20.0f);
	out_diskRadius		= g_rng->GetRandomFloatInRange(5.0f, 25.0f);
	out_diskCenter		= Vec2(randomCenterX, randomCenterY);
}


void Game::RandomizeDisk2D(Vec2& out_diskCenter, float& out_diskRadius, FloatRange const& radiusRange, AABB2 const& centerBounds)
{
	float randomCenterX = g_rng->GetRandomFloatInRange(centerBounds.m_mins.x, centerBounds.m_maxs.x);
	float randomCenterY = g_rng->GetRandomFloatInRange(centerBounds.m_mins.y, centerBounds.m_maxs.y);
	out_diskRadius = g_rng->GetRandomFloatInRange(radiusRange.m_min, radiusRange.m_max);
	out_diskCenter = Vec2(randomCenterX, randomCenterY);
}


void Game::RandomizeAABB2D(AABB2& out_aabb2)
{
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);

	float randomCenterX = g_rng->GetRandomFloatInRange(40.0f, worldX - 40.0f);
	float randomCenterY = g_rng->GetRandomFloatInRange(20.0f, worldY - 20.0f);
	Vec2 randomCenter	= Vec2(randomCenterX, randomCenterY);

	float randomWidth	= g_rng->GetRandomFloatInRange(10.0f, 20.0f);
	float randomHeigth	= g_rng->GetRandomFloatInRange(10.0f, 20.0f);
	Vec2 randomDims		= Vec2(randomWidth, randomHeigth);

	out_aabb2.m_mins = Vec2(randomCenter - randomDims * 0.5f);
	out_aabb2.m_maxs = Vec2(randomCenter + randomDims * 0.5f);
}


void Game::RandomizeOBB2D(OBB2& out_obb2)
{
	LineSegment2 line;
	RandomizeLineSegment2D(line);
	Vec2 randomIBasis = line.GetDirection();

	float randomWidth	 = g_rng->GetRandomFloatInRange(10.0f, 20.0f);
	float randomHeigth	 = g_rng->GetRandomFloatInRange(10.0f, 20.0f);
	Vec2  randomHalfDims = Vec2(randomWidth, randomHeigth) * 0.5f;

	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);

	float randomCenterX = g_rng->GetRandomFloatInRange(40.0f, worldX - 40.0f);
	float randomCenterY = g_rng->GetRandomFloatInRange(20.0f, worldY - 20.0f);
	Vec2  randomCenter	= Vec2(randomCenterX, randomCenterY);

	out_obb2 = OBB2(randomCenter, randomIBasis, randomHalfDims);
}


void Game::RandomizeCapsule2D(Capsule2& out_capsule)
{
	RandomizeLineSegment2D(out_capsule.m_bone);
	out_capsule.m_radius = g_rng->GetRandomFloatInRange(5.0f, 10.0f);
}


void Game::RandomizeLineSegment2D(LineSegment2& out_lineSegment)
{
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);

	float randomStartX	= g_rng->GetRandomFloatInRange(20.0f, worldX - 20.0f);
	float randomStartY	= g_rng->GetRandomFloatInRange(10.0f, worldY - 10.0f);

	float randomEndX	= g_rng->GetRandomFloatInRange(20.0f, worldX - 20.0f);
	float randomEndY	= g_rng->GetRandomFloatInRange(10.0f, worldY - 10.0f);

	out_lineSegment.m_start = Vec2(randomStartX, randomStartY);
	out_lineSegment.m_end	= Vec2(randomEndX,	 randomEndY);
}


void Game::UpdateVertexVectorNearestPoint2D()
{
	Rgba8 shapeColor;

	//disk
	shapeColor = IsPointInsideDisk2D(m_referencePoint, m_disk2dCenter, m_disk2dRadius) ? m_highlightedShapeColor : m_shapeColor;
	AddVertsForDisk2DToVector(m_verts, m_disk2dCenter, m_disk2dRadius, shapeColor);

	//aabb2
	shapeColor = IsPointInsideAABB2D(m_referencePoint, m_aabb2) ? m_highlightedShapeColor : m_shapeColor;
	AddVertsForAABB2ToVector(m_verts, m_aabb2, shapeColor);
	
	//capsule2
	shapeColor = IsPointInsideCapsule2D(m_referencePoint, m_capsule2d) ? m_highlightedShapeColor : m_shapeColor;
	AddVertsForCapsule2DToVector(m_verts, m_capsule2d, shapeColor);
	
	//obb2
	shapeColor = IsPointInsideOBB2D(m_referencePoint, m_obb2) ? m_highlightedShapeColor : m_shapeColor;
	AddVertsForOBB2DToVector(m_verts, m_obb2, shapeColor);

	//line segments
	LineSegment2 infiniteLine = m_infiniteLine2d.GetStretchedFromCenter(1000.0f);
	AddVertsForLineSegment2DToVector(m_verts, m_finiteLine2d,	m_shapeColor, 1.0f, 1.0f);
	AddVertsForLineSegment2DToVector(m_verts, infiniteLine,		m_shapeColor);

	Vec2 nearestPointToDisk = GetNearestPointOnDisk2D(m_referencePoint, m_disk2dCenter, m_disk2dRadius);
	Vec2 nearestPointToAABB2 = GetNearestPointOnAABB2D(m_referencePoint, m_aabb2);
	Vec2 nearestPointToCapsule = GetNearestPointOnCapsule2D(m_referencePoint, m_capsule2d);
	Vec2 nearestPointToOBB2 = GetNearestPointOnOBB2D(m_referencePoint, m_obb2);
	Vec2 nearestPointToFiniteLineSegment = GetNearentPointOnLineSegment2D(m_referencePoint, m_finiteLine2d);
	Vec2 nearestPointToInfiniteLineSegment = GetNearestPointOnInfiniteLine2D(m_referencePoint, m_infiniteLine2d);

	//lines from reference point to nearest points
	Rgba8 lineToReferencePointColor = Rgba8::GREY;
	lineToReferencePointColor.a = 155;
	AddVertsForLineSegment2DToVector(m_verts, LineSegment2(m_referencePoint, nearestPointToDisk), lineToReferencePointColor, 0.25f, 0.0f);
	AddVertsForLineSegment2DToVector(m_verts, LineSegment2(m_referencePoint, nearestPointToAABB2), lineToReferencePointColor, 0.25f, 0.0f);
	AddVertsForLineSegment2DToVector(m_verts, LineSegment2(m_referencePoint, nearestPointToCapsule), lineToReferencePointColor, 0.25f, 0.0f);
	AddVertsForLineSegment2DToVector(m_verts, LineSegment2(m_referencePoint, nearestPointToOBB2), lineToReferencePointColor, 0.25f, 0.0f);
	AddVertsForLineSegment2DToVector(m_verts, LineSegment2(m_referencePoint, nearestPointToFiniteLineSegment), lineToReferencePointColor, 0.25f, 0.0f);
	AddVertsForLineSegment2DToVector(m_verts, LineSegment2(m_referencePoint, nearestPointToInfiniteLineSegment), lineToReferencePointColor, 0.25f, 0.0f);

	//nearest points
	AddVertsForDisk2DToVector(m_verts, nearestPointToDisk, m_referencePointRadius, m_nearestPointColor);
	AddVertsForDisk2DToVector(m_verts, nearestPointToAABB2, m_referencePointRadius, m_nearestPointColor);
	AddVertsForDisk2DToVector(m_verts, nearestPointToCapsule, m_referencePointRadius, m_nearestPointColor);
	AddVertsForDisk2DToVector(m_verts, nearestPointToOBB2, m_referencePointRadius, m_nearestPointColor);
	AddVertsForDisk2DToVector(m_verts, nearestPointToFiniteLineSegment, m_referencePointRadius, m_nearestPointColor);
	AddVertsForDisk2DToVector(m_verts, nearestPointToInfiniteLineSegment, m_referencePointRadius, m_nearestPointColor);
	AddVertsForDisk2DToVector(m_verts, m_referencePoint, m_referencePointRadius, m_referencePointColor);
}


void Game::RandomizeDisksForRaycast2D()
{
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);

	for (int diskIndex = 0; diskIndex < numDisksRaycast2D; diskIndex++)
	{
		Disk2D& disk = m_disks[diskIndex];
		RandomizeDisk2D(disk.m_center, disk.m_radius, FloatRange(5.0f, 10.0f), AABB2(20.0f, 20.0f, worldX - 20.0f, worldY - 20.0f));
	}
}


void Game::RaycastAllDisks2D()
{
	m_impactDisk.m_center = Vec2(-100.0f, -100.0f);
	m_impactDisk.m_radius = -20.0f;
	Vec2 raycast = m_raycast2DEnd - m_raycast2DStart;
	Vec2 raycastDirection = raycast.GetNormalized();
	float raycastDistance = raycast.GetLength();

	float minDistance = 10000.0f;
	for (int diskIndex = 0; diskIndex < numDisksRaycast2D; diskIndex++)
	{
		Disk2D& disk = m_disks[diskIndex];
		BaseRaycastResult2D raycastResult = RaycastVsDisk2D(m_raycast2DStart, raycastDirection, raycastDistance, disk.m_center, disk.m_radius);
		if (raycastResult.m_didHit)
		{
			float impactDistance = raycastResult.m_impactDistance;
			if (impactDistance < minDistance)
			{
				minDistance = impactDistance;
				m_impactDisk = disk;
				m_impactPoint2D = raycastResult.m_impactPoint;
				m_impactNormal2D = raycastResult.m_impactNormal;
			}
		}
	}
}


void Game::UpdateVertexVectorRaycastVDisk2D()
{
	float lineWidth = 0.3f;
	bool impactDiskExists = false;

	for (int diskIndex = 0; diskIndex < numDisksRaycast2D; diskIndex++)
	{
		Disk2D& disk = m_disks[diskIndex];
		if(disk.m_center == m_impactDisk.m_center && m_impactDisk.m_radius == disk.m_radius)
		{
			impactDiskExists = true;
		}
		else
		{
			AddVertsForDisk2DToVector(m_verts, disk.m_center, disk.m_radius, m_shapeColor);
		}
	}

	if (impactDiskExists)
	{
		float impactNormalLength = 5.0f;

		AddVertsForArrow2DToVector(m_verts, m_raycast2DStart, m_raycast2DEnd, m_nearestPointColor, lineWidth, 0.05f);
		AddVertsForDisk2DToVector(m_verts, m_impactDisk.m_center, m_impactDisk.m_radius, m_highlightedShapeColor);
		AddVertsForArrow2DToVector(m_verts, m_impactPoint2D, m_impactPoint2D + m_impactNormal2D * impactNormalLength, Rgba8::GREEN, lineWidth, 0.05f);
		AddVertsForDisk2DToVector(m_verts, m_impactPoint2D, 0.5f, Rgba8::CYAN);
	}
	else
	{
		AddVertsForArrow2DToVector(m_verts, m_raycast2DStart, m_raycast2DEnd, m_referencePointColor, lineWidth, 0.05f);
	}
}


void Game::RandomizeAABB2sForRaycast()
{
	for (int aabb2Index = 0; aabb2Index < numAABB2Raycast2D; aabb2Index++)
	{
		AABB2& aabb2 = m_aabb2s[aabb2Index];
		RandomizeAABB2D(aabb2);
	}
}


void Game::RaycastAllAABB2s()
{
	m_impactAABB2 = AABB2(Vec2::ZERO, Vec2::ZERO);

	Vec2 raycast = m_raycast2DEnd - m_raycast2DStart;
	Vec2 raycastDirection = raycast.GetNormalized();
	float raycastDistance = raycast.GetLength();

	float minDistance = 10000.0f;
	for (int aabb2Index = 0; aabb2Index < numAABB2Raycast2D; aabb2Index++)
	{
		AABB2& aabb2 = m_aabb2s[aabb2Index];
		BaseRaycastResult2D raycastResult = RaycastVsAABB2(m_raycast2DStart, raycastDirection, raycastDistance, aabb2);
		if (raycastResult.m_didHit)
		{
			Vec2 impactPoint = raycastResult.m_impactPoint;
			float impactDistance = raycastResult.m_impactDistance;
			if (impactDistance < minDistance)
			{
				minDistance = impactDistance;
				m_impactAABB2 = aabb2;
				m_impactPoint2D = impactPoint;
				m_impactNormal2D = raycastResult.m_impactNormal;
			}
		}
	}
}


void Game::UpdateVertexVectorRaycastVAABB2()
{
	float lineWidth = 0.3f;
	bool impactAABB2Exists = false;

	for (int aabb2Index = 0; aabb2Index < numAABB2Raycast2D; aabb2Index++)
	{
		AABB2& aabb2 = m_aabb2s[aabb2Index];
		if (aabb2.m_mins == m_impactAABB2.m_mins && aabb2.m_maxs == m_impactAABB2.m_maxs )
		{
			impactAABB2Exists = true;
		}
		else
		{
			AddVertsForAABB2ToVector(m_verts, aabb2, m_shapeColor);
		}
	}

	if (impactAABB2Exists)
	{
		float impactNormalLength = 5.0f;

		AddVertsForArrow2DToVector(m_verts, m_raycast2DStart, m_raycast2DEnd, m_nearestPointColor, lineWidth, 1.0f, 0.0f);
		AddVertsForAABB2ToVector(m_verts, m_impactAABB2, m_highlightedShapeColor);
		AddVertsForDisk2DToVector(m_verts, m_impactPoint2D, 0.5f, Rgba8::CYAN);
		AddVertsForArrow2DToVector(m_verts, m_impactPoint2D, m_impactPoint2D + m_impactNormal2D * impactNormalLength, Rgba8::GREEN, lineWidth, 1.0f, 0.0f);
	}
	else
	{
		AddVertsForArrow2DToVector(m_verts, m_raycast2DStart, m_raycast2DEnd, m_referencePointColor, lineWidth, 1.0f, 0.0f);
	}
}


void Game::RandomizeLineSegment2sForRaycast()
{
	for (int lineSeg2Index = 0; lineSeg2Index < numLineSegment2Raycast2D; lineSeg2Index++)
	{
		LineSegment2& lineSegment2 = m_lineSegment2s[lineSeg2Index];
		RandomizeLineSegment2D(lineSegment2);
	}
}


void Game::RaycastAllLineSegment2s()
{
	m_impactLineSegment = LineSegment2(Vec2::ZERO, Vec2::ZERO);

	Vec2 raycast = m_raycast2DEnd - m_raycast2DStart;
	Vec2 raycastDirection = raycast.GetNormalized();
	float raycastDistance = raycast.GetLength();

	float minDistance = 10000.0f;
	for (int lineSegment2Index = 0; lineSegment2Index < numLineSegment2Raycast2D; lineSegment2Index++)
	{
		LineSegment2& lineSegment2 = m_lineSegment2s[lineSegment2Index];
		BaseRaycastResult2D raycastResult = RaycastVsLineSegment2(m_raycast2DStart, raycastDirection, raycastDistance, lineSegment2);
		if (raycastResult.m_didHit)
		{
			Vec2 impactPoint = raycastResult.m_impactPoint;
			float impactDistance = raycastResult.m_impactDistance;
			if (impactDistance < minDistance)
			{
				minDistance = impactDistance;
				m_impactLineSegment = lineSegment2;
				m_impactPoint2D = impactPoint;
				m_impactNormal2D = raycastResult.m_impactNormal;
			}
		}
	}
}


void Game::UpdateVertexVectorRaycastVLineSegment2()
{
	float lineWidth = 0.3f;
	bool impactLineSegment2Exists = false;

	for (int lineSegment2Index = 0; lineSegment2Index < numLineSegment2Raycast2D; lineSegment2Index++)
	{
		LineSegment2& lineSegment2 = m_lineSegment2s[lineSegment2Index];
		if (lineSegment2 == m_impactLineSegment)
		{
			impactLineSegment2Exists = true;
		}
		else
		{
			AddVertsForLineSegment2DToVector(m_verts, lineSegment2, m_shapeColor, 1.0f, 0.05f);
		}
	}

	if (impactLineSegment2Exists)
	{
		float impactNormalLength = 5.0f;

		AddVertsForArrow2DToVector(m_verts, m_raycast2DStart, m_raycast2DEnd, m_nearestPointColor, lineWidth, 0.05f);
		AddVertsForLineSegment2DToVector(m_verts, m_impactLineSegment, m_highlightedShapeColor, 1.0f, 0.05f);
		AddVertsForDisk2DToVector(m_verts, m_impactPoint2D, 0.5f, Rgba8::CYAN);
		AddVertsForArrow2DToVector(m_verts, m_impactPoint2D, m_impactPoint2D + m_impactNormal2D * impactNormalLength, Rgba8::GREEN, lineWidth, 0.05f);
	}
	else
	{
		AddVertsForArrow2DToVector(m_verts, m_raycast2DStart, m_raycast2DEnd, m_referencePointColor, lineWidth, 0.05f);
	}
}


void Game::RaycastAll3DShapes()
{
	float pointRadius = 0.1f;
	float lineThickness = 0.04f;

	Vec3 camForwardNormal = m_persCamAngles.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	m_raycast3DStart = m_isRefPointParked ? m_referencePoint3D : m_persCamPosition;
	Vec3 rayForwardNormal = m_isRefPointParked ? m_raycast3DFwdNormal : camForwardNormal;
	BaseRaycastResult3D finalHitResult = {};
	float earliestHitDistance = 99999.0f;

	m_impactShape3D = nullptr;
	for (int shapeIndex = 0; shapeIndex < (int) m_3dShapes.size(); shapeIndex++)
	{
		Shape3D*& shape = m_3dShapes[shapeIndex];
		BaseRaycastResult3D result = shape->Raycast(m_raycast3DStart, rayForwardNormal, m_raycast3DLength);
		if (result.m_didHit && result.m_impactDistance < earliestHitDistance)
		{
			earliestHitDistance = result.m_impactDistance;
			finalHitResult = result;
			m_impactShape3D = shape;
		}
	}

	if (finalHitResult.m_didHit)
	{
		if (m_impactShape3D->m_isWire && !m_isObjectPickedUp)
		{
			Rgba8 color = m_impactShape3D->m_color;
			color = m_highlightedShapeColor;
			TransformVertexArrayColor(m_impactShape3D->m_vertCount, &m_wire3dShapesVerts[m_impactShape3D->m_vertsStartIndex], color);
		}
		else if(!m_impactShape3D->m_isWire && !m_isObjectPickedUp)
		{
			Rgba8 color = m_impactShape3D->m_color;
			color = color * Rgba8(100, 100, 255, 255);
			TransformVertexArrayColor(m_impactShape3D->m_vertCount, &m_solid3dShapesVerts[m_impactShape3D->m_vertsStartIndex], color);
		}
		AddVertsForUVSphereZ3DToVector(m_verts, finalHitResult.m_impactPoint, pointRadius, 16.0f, 12.0f, Rgba8::CYAN);
		DebugAddWorldArrow(finalHitResult.m_impactPoint, finalHitResult.m_impactPoint + finalHitResult.m_impactNormal, 0.0f, Rgba8::GREEN, lineThickness, DebugRenderMode::XRAY);
	}

	if (m_isRefPointParked)
	{
		if (finalHitResult.m_didHit)
			DebugAddWorldArrow(m_raycast3DStart, m_raycast3DStart + m_raycast3DFwdNormal * m_raycast3DLength, 0.0f, Rgba8::YELLOW, lineThickness, DebugRenderMode::XRAY);
		else
			DebugAddWorldArrow(m_raycast3DStart, m_raycast3DStart + m_raycast3DFwdNormal * m_raycast3DLength, 0.0f, Rgba8::WHITE, lineThickness, DebugRenderMode::XRAY);
	}
}


void Game::UpdateNearestPointVerts3D()
{
	float pointRadius = 0.1f;
	Vec3 referencePoint = m_isRefPointParked ? m_referencePoint3D : m_persCamPosition;
	
	Rgba8 referencePointColor = m_referencePointColor;
	Rgba8 nearestPointColor = Rgba8::RED;
	Rgba8 nearestNearestPointColor = Rgba8::GREEN;

	std::vector<Vec3> nearestPoints;
	int nearestNearestPointIndex = 0;
	float shortestDistance = 999999.0f;
	for (int shapeIndex = 0; shapeIndex < (int) m_3dShapes.size(); shapeIndex++)
	{
		Shape3D*& shape = m_3dShapes[shapeIndex];
		Vec3 nearestPoint = shape->GetNearestPoint(referencePoint);
		float distanceFromRefSqrd = (nearestPoint - referencePoint).GetLengthSquared();
		nearestPoints.push_back(nearestPoint);

		if (distanceFromRefSqrd < shortestDistance)
		{
			shortestDistance = distanceFromRefSqrd;
			nearestNearestPointIndex = shapeIndex;
		}
	}

	for (int nearestPtIndex = 0; nearestPtIndex < (int) nearestPoints.size(); nearestPtIndex++)
	{
		Vec3 nearestPt = nearestPoints[nearestPtIndex];
		if (nearestPtIndex == nearestNearestPointIndex)
		{
			AddVertsForUVSphereZ3DToVector(m_verts, nearestPt, pointRadius, 12.0f, 8.0f, nearestNearestPointColor);
			if (m_isRefPointParked)
			{
				Rgba8 lineColor = Rgba8(34, 122, 34, 255);
				AddVertsForLine3DToVector(m_verts, referencePoint, nearestPt, 0.04f, 0.05f, lineColor);
			}
		}
		else
		{
			AddVertsForUVSphereZ3DToVector(m_verts, nearestPt, pointRadius, 12.0f, 8.0f, nearestPointColor);
			if (m_isRefPointParked)
			{
				Rgba8 lineColor = Rgba8(156, 64, 45, 255);
				AddVertsForLine3DToVector(m_verts, referencePoint, nearestPt, 0.04f, 0.05f, lineColor);
			}
		}
	
	}
		
	if (m_isRefPointParked)
		AddVertsForUVSphereZ3DToVector(m_verts, referencePoint, pointRadius, 12.0f, 8.0f, referencePointColor);
}


void Game::RandomizeShapes3D()
{
	for (int shapeIndex = 0; shapeIndex < m_3dShapes.size(); shapeIndex++)
	{
		Shape3D*& shape = m_3dShapes[shapeIndex];
		if (shape)
		{
			delete shape;
			shape = nullptr;
		}
	}
	m_3dShapes.clear();
	m_solid3dShapesVerts.clear();
	m_wire3dShapesVerts.clear();

	int numSolidCubes		= g_rng->GetRandomIntInRange(2, 3);
	int numSolidSpheres		= g_rng->GetRandomIntInRange(2, 3);
	int numSolidCylinders	= g_rng->GetRandomIntInRange(2, 3);
	int numWireCubes		= g_rng->GetRandomIntInRange(2, 3);
	int numWireSpheres		= g_rng->GetRandomIntInRange(2, 3);
	int numWireCylinders	= g_rng->GetRandomIntInRange(2, 3);

	for (int i = 0; i < numSolidCubes; i++)
	{
		int existingVertCount = (int) m_solid3dShapesVerts.size();

		Vec3 center = g_rng->GetRandomPositionInAABB3(Vec3(1.0f, 1.0f, 1.0f), Vec3(8.0f, 8.0f, 8.0f));
		float width = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
		float height = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
		float length = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
		AABB3 randomCube = AABB3(center, length, width, height);
		AddVertsForAABBZ3DToVector(m_solid3dShapesVerts, randomCube, Rgba8::WHITE);

		int newVertCount = (int) m_solid3dShapesVerts.size();

		Shape3D* cube = new AABB3D(randomCube);
		cube->m_vertCount = newVertCount - existingVertCount;
		cube->m_vertsStartIndex = existingVertCount;
		cube->m_color = Rgba8::WHITE;
		cube->m_shape = SHAPE_AABB3;
		cube->m_isWire = false;
		m_3dShapes.push_back(cube);
	}

	for (int i = 0; i < numSolidCylinders; i++)
	{
		int existingVertCount = (int) m_solid3dShapesVerts.size();

		Vec2 center = g_rng->GetRandomPositionInAABB2(Vec2(1.0f, 1.0f), Vec2(10.0f, 10.0f));
		FloatRange minMax;
		minMax.m_min = g_rng->GetRandomFloatInRange(-5.0f, 5.0f);
		minMax.m_max = g_rng->GetRandomFloatInRange(5.0f, 10.0f);
		float radius = g_rng->GetRandomFloatInRange(0.2f, 2.5f);
		AddVertsForCylinderZ3DToVector(m_solid3dShapesVerts, center, minMax, radius, 32.0f, Rgba8::WHITE);

		int newVertCount = (int) m_solid3dShapesVerts.size();

		Shape3D* cylinder = new ZCylinder3D(radius, center, minMax);
		cylinder->m_vertCount = newVertCount - existingVertCount;
		cylinder->m_vertsStartIndex = existingVertCount;
		cylinder->m_color = Rgba8::WHITE;
		cylinder->m_shape = SHAPE_ZCYLINDER;
		cylinder->m_isWire = false;
		m_3dShapes.push_back(cylinder);
	}

	for (int i = 0; i < numSolidSpheres; i++)
	{
		int existingVertCount = (int) m_solid3dShapesVerts.size();

		Vec3 center = g_rng->GetRandomPositionInAABB3(Vec3(-10.0f, -10.0f, -10.0f), Vec3(10.0f, 10.0f, 10.0f));
		float radius = g_rng->GetRandomFloatInRange(0.5f, 3.0f);
		AddVertsForUVSphereZ3DToVector(m_solid3dShapesVerts, center, radius, 32.0f, 16.0f, Rgba8::WHITE);

		int newVertCount = (int) m_solid3dShapesVerts.size();

		Shape3D* sphere = new ZSphere3D(radius, center);
		sphere->m_vertCount = newVertCount - existingVertCount;
		sphere->m_vertsStartIndex = existingVertCount;
		sphere->m_color = Rgba8::WHITE;
		sphere->m_shape = SHAPE_ZSPHERE;
		sphere->m_isWire = false;
		m_3dShapes.push_back(sphere);
	}

	for (int i = 0; i < numWireCubes; i++)
	{
		int existingVertCount = (int) m_wire3dShapesVerts.size();

		Vec3 center = g_rng->GetRandomPositionInAABB3(Vec3(-8.0f, -8.0f, -8.0f), Vec3(-1.0f, -1.0f, -1.0f));
		float width = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
		float height = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
		float length = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
		AABB3 randomCube = AABB3(center, length, width, height);
		AddVertsForWireAABBZ3DToVector(m_wire3dShapesVerts, randomCube, m_shapeColor);

		int newVertCount = (int) m_wire3dShapesVerts.size();

		Shape3D* cube = new AABB3D(randomCube);
		cube->m_vertCount = newVertCount - existingVertCount;
		cube->m_vertsStartIndex = existingVertCount;
		cube->m_color = m_shapeColor;
		cube->m_shape = SHAPE_AABB3;
		cube->m_isWire = true;
		m_3dShapes.push_back(cube);
	}

	for (int i = 0; i < numWireCylinders; i++)
	{
		int existingVertCount = (int) m_wire3dShapesVerts.size();

		Vec2 center = g_rng->GetRandomPositionInAABB2(Vec2(-10.0f, -10.0f), Vec2(-1.0f, -1.0f));
		FloatRange minMax;
		minMax.m_min = g_rng->GetRandomFloatInRange(-5.0f, 5.0f);
		minMax.m_max = g_rng->GetRandomFloatInRange(5.0f, 10.0f);
		float radius = g_rng->GetRandomFloatInRange(0.2f, 2.5f); 
		AddVertsForWireCylinderZ3DToVector(m_wire3dShapesVerts, center, minMax, radius, 16.0f, m_shapeColor);

		int newVertCount = (int) m_wire3dShapesVerts.size();

		Shape3D* cylinder = new ZCylinder3D(radius, center, minMax);
		cylinder->m_vertCount = newVertCount - existingVertCount;
		cylinder->m_vertsStartIndex = existingVertCount;
		cylinder->m_color = m_shapeColor;
		cylinder->m_shape = SHAPE_ZCYLINDER;
		cylinder->m_isWire = true;
		m_3dShapes.push_back(cylinder);
	}

	for (int i = 0; i < numWireSpheres; i++)
	{
		int existingVertCount = (int) m_wire3dShapesVerts.size();

		Vec3 center = g_rng->GetRandomPositionInAABB3(Vec3(-10.0f, -10.0f, -10.0f), Vec3(10.0f, 10.0f, 10.0f));
		float radius = g_rng->GetRandomFloatInRange(0.5f, 3.0f);
		AddVertsForWireSphereZ3DToVector(m_wire3dShapesVerts, center, radius, 16.0f, 12.0f, m_shapeColor);

		int newVertCount = (int) m_wire3dShapesVerts.size();

		Shape3D* sphere = new ZSphere3D(radius, center);
		sphere->m_vertCount = newVertCount - existingVertCount;
		sphere->m_vertsStartIndex = existingVertCount;
		sphere->m_color = m_shapeColor;
		sphere->m_shape = SHAPE_ZSPHERE;
		sphere->m_isWire = true;
		m_3dShapes.push_back(sphere);
	}
}


void Game::ParkReferencePointNearestPoint3D()
{
	if (m_isRefPointParked)
		return;

	m_referencePoint3D = m_persCamPosition;
	m_raycast3DFwdNormal = m_persCamAngles.GetFoward_XFwd_YLeft_ZUp();
	m_isRefPointParked = true;
}


void Game::UnParkReferencePointNearestPoint3D()
{
	m_isRefPointParked = false;
}


void Game::CheckAndUpdateVertAlphasBasedOnOverlaps()
{
	double time = GetCurrentTimeSeconds();
	float sinTime = (SinRadians((float) time * 5.0f) * 0.5f + 0.5f);
	unsigned char overlapAlpha = FloatToByte(sinTime);

	for (int shapeAIndex = 0; shapeAIndex < (int) m_3dShapes.size(); shapeAIndex++)
	{
		Shape3D*& shapeA = m_3dShapes[shapeAIndex];
		bool isShapeAOverlapping = false;
		
		for (int shapeBIndex = 0; shapeBIndex < (int) m_3dShapes.size(); shapeBIndex++)
		{
			if (shapeAIndex != shapeBIndex)
			{
				Shape3D*& shapeB = m_3dShapes[shapeBIndex];
				isShapeAOverlapping |= Do3DShapesOverlap(shapeA, shapeB);
				if (isShapeAOverlapping)
				{
					int a = 0;
					a += 5;
				}
			}
		}

		shapeA->m_isOverlapping = isShapeAOverlapping;
	}

	for (int shapeIndex = 0; shapeIndex < (int) m_3dShapes.size(); shapeIndex++)
	{
		Shape3D*& shape = m_3dShapes[shapeIndex];
		if (shape->m_isOverlapping)
		{
			if (shape->m_isWire)
			{
				Rgba8 color = shape->m_color;
				color.a = overlapAlpha;
				int shapeVertStartIndex = shape->m_vertsStartIndex;
				TransformVertexArrayColor(shape->m_vertCount, &m_wire3dShapesVerts[shapeVertStartIndex], color);
			}
			else
			{
				Rgba8 color = shape->m_color;
				color.a = overlapAlpha;
				int shapeVertStartIndex = shape->m_vertsStartIndex;
				TransformVertexArrayColor(shape->m_vertCount, &m_solid3dShapesVerts[shapeVertStartIndex], color);
			}
		}
		else
		{
			if (shape->m_isWire)
			{
				Rgba8 color = shape->m_color;
				int shapeVertStartIndex = shape->m_vertsStartIndex;
				TransformVertexArrayColor(shape->m_vertCount, &m_wire3dShapesVerts[shapeVertStartIndex], color);
			}
			else
			{
				Rgba8 color = shape->m_color;
				int shapeVertStartIndex = shape->m_vertsStartIndex;
				TransformVertexArrayColor(shape->m_vertCount, &m_solid3dShapesVerts[shapeVertStartIndex], color);
			}
		}
	}
}


bool Game::Do3DShapesOverlap(Shape3D* shapeA, Shape3D* shapeB)
{
	Shape3DName a = shapeA->m_shape;
	Shape3DName b = shapeB->m_shape;

	bool result = false;

	if (a == SHAPE_AABB3 && b == SHAPE_AABB3)
	{
		AABB3D* aabb3A = dynamic_cast<AABB3D*>(shapeA);
		AABB3D* aabb3B = dynamic_cast<AABB3D*>(shapeB);
		result =  DoAABB3sOverlap3D(aabb3A->m_aabb3, aabb3B->m_aabb3);
	}

	if (a == SHAPE_AABB3 && b == SHAPE_ZCYLINDER)
	{
		AABB3D* aabb3 = dynamic_cast<AABB3D*>(shapeA);
		ZCylinder3D* cylinder = dynamic_cast<ZCylinder3D*>(shapeB);
		result = DoesAABB3OverlapWithZCylinder3D(aabb3->m_aabb3, cylinder->m_center, cylinder->m_minMax, cylinder->m_radius);
	}

	if (a == SHAPE_AABB3 && b == SHAPE_ZSPHERE)
	{
		AABB3D* aabb3 = dynamic_cast<AABB3D*>(shapeA);
		ZSphere3D* sphere = dynamic_cast<ZSphere3D*>(shapeB);
		result = DoesSphereOverlapWithAABB3D(sphere->m_center, sphere->m_radius, aabb3->m_aabb3);
	}

	if (b == SHAPE_AABB3 && a == SHAPE_ZCYLINDER)
	{
		AABB3D* aabb3 = dynamic_cast<AABB3D*>(shapeB);
		ZCylinder3D* cylinder = dynamic_cast<ZCylinder3D*>(shapeA);
		result = DoesAABB3OverlapWithZCylinder3D(aabb3->m_aabb3, cylinder->m_center, cylinder->m_minMax, cylinder->m_radius);
	}

	if (b == SHAPE_AABB3 && a == SHAPE_ZSPHERE)
	{
		AABB3D* aabb3 = dynamic_cast<AABB3D*>(shapeB);
		ZSphere3D* sphere = dynamic_cast<ZSphere3D*>(shapeA);
		result = DoesSphereOverlapWithAABB3D(sphere->m_center, sphere->m_radius, aabb3->m_aabb3);
	}

	if (a == SHAPE_ZSPHERE && b == SHAPE_ZSPHERE)
	{
		ZSphere3D* sphereA = dynamic_cast<ZSphere3D*>(shapeA);
		ZSphere3D* sphereB = dynamic_cast<ZSphere3D*>(shapeB);
		result = DoSpheresOverlap3D(sphereA->m_center, sphereA->m_radius, sphereB->m_center, sphereB->m_radius);
	}

	if (a == SHAPE_ZSPHERE && b == SHAPE_ZCYLINDER)
	{
		ZSphere3D* sphere = dynamic_cast<ZSphere3D*>(shapeA);
		ZCylinder3D* cylinder = dynamic_cast<ZCylinder3D*>(shapeB);
		result = DoesSphereOverlapWithZCylinder3D(sphere->m_center, sphere->m_radius, cylinder->m_center, cylinder->m_minMax, cylinder->m_radius);
	}

	if (b == SHAPE_ZSPHERE && a == SHAPE_ZCYLINDER)
	{
		ZSphere3D* sphere = dynamic_cast<ZSphere3D*>(shapeB);
		ZCylinder3D* cylinder = dynamic_cast<ZCylinder3D*>(shapeA);
		result = DoesSphereOverlapWithZCylinder3D(sphere->m_center, sphere->m_radius, cylinder->m_center, cylinder->m_minMax, cylinder->m_radius);
	}

	if (a == SHAPE_ZCYLINDER && b == SHAPE_ZCYLINDER)
	{
		ZCylinder3D* cylinderA = dynamic_cast<ZCylinder3D*>(shapeA);
		ZCylinder3D* cylinderB = dynamic_cast<ZCylinder3D*>(shapeB);
		result = DoZCylindersOverlap3D(cylinderA->m_center, cylinderA->m_minMax, cylinderA->m_radius, cylinderB->m_center, cylinderB->m_minMax, cylinderB->m_radius);
	}

	return result;
}


void Game::SetTimeScale(float timeScale)
{
	m_clock.SetTimeScale((double) timeScale);
}
