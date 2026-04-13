#include "Game/GameCamera.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/EngineCommon.hpp"

GameCamera::GameCamera()
{

}


void GameCamera::Initialise()
{
	DefineGameSpace(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));
	Mat44 persProjMat = Mat44::CreatePerspectiveProjectionMatrix(60.0f, g_window->GetWindowConfig().m_aspectRatio, 0.1f, 1000.0f);
	SetProjectionMatrix(persProjMat);
}


void GameCamera::Update(float deltaSeconds)
{
	CheckInput(deltaSeconds);

	Mat44 camModel;
	switch (m_cameraMode)
	{
		case GAME_CAMERA_MODE_SPECTATOR:
		{
			camModel = m_orientaion.GetAsMatrix_XFwd_YLeft_ZUp();
			camModel.SetTranslation3D(m_position);
			break;
		}
		case GAME_CAMERA_MODE_FIXED_ANGLE_TRACKING:
		{
			static Vec3 fixedModeOrientation = g_gameConfigBlackboard.GetValue("fixedAngleCamOrientation", Vec3(40.0f, 30.0f, 0.0f));
			static EulerAngles camEulers = EulerAngles(fixedModeOrientation.x, fixedModeOrientation.y, fixedModeOrientation.z);

			if (m_attachedEntity)
			{
				float cameraDistance = 10.0f;
				World* world = g_game->GetWorld();
				camModel = camEulers.GetAsMatrix_XFwd_YLeft_ZUp();
				Vec3 entityEyePos = m_attachedEntity->GetEyePos();
				WorldRaycastResult raycast = world->RaycastWorld(entityEyePos, -camModel.GetIBasis3D(), cameraDistance);
				if (raycast.m_didHit)
				{
					Vec3 impactPoint = raycast.m_impactPoint;
					Vec3 camPos = impactPoint + camModel.GetIBasis3D() * 0.5f;
					camModel.SetTranslation3D(camPos);
				}
				else
				{
					camModel.SetTranslation3D(entityEyePos - camModel.GetIBasis3D() * cameraDistance);
				}
			}
			else
			{
				g_console->AddLine(DevConsole::WARNING_MESSAGE, Stringf("Camera is set to fixed tracking mode without being attached to an entity"));
			}

			m_position = camModel.GetTranslation3D();
			break;
		}
		case GAME_CAMERA_MODE_OVER_SHOULDER:
		{
			if (m_attachedEntity)
			{
				float cameraDistance = 4.0f;
				World* world = g_game->GetWorld();
				camModel = m_attachedEntity->GetModelMatrix();
				Vec3 entityEyePos = m_attachedEntity->GetEyePos();
				WorldRaycastResult raycast = world->RaycastWorld(entityEyePos, -camModel.GetIBasis3D(), cameraDistance);
				if (raycast.m_didHit)
				{
					Vec3 impactPoint = raycast.m_impactPoint;
					Vec3 camPos = impactPoint + camModel.GetIBasis3D() * 0.5f;
					camModel.SetTranslation3D(camPos);
				}
				else
				{
					camModel.SetTranslation3D(entityEyePos - camModel.GetIBasis3D() * cameraDistance);
				}
			}
			else
			{
				g_console->AddLine(DevConsole::WARNING_MESSAGE, Stringf("Camera is set to over the shoulder mode without being attached to an entity"));
			}

			m_position = camModel.GetTranslation3D();
			break;
		}
		case GAME_CAMERA_MODE_FIRST_PERSON:
		{
			if (m_attachedEntity)
			{
				camModel = m_attachedEntity->GetModelMatrix();
				camModel.SetTranslation3D(m_attachedEntity->GetEyePos());
			}
			else
			{
				g_console->AddLine(DevConsole::WARNING_MESSAGE, Stringf("Camera is set to first person mode without being attached to an entity"));
			}

			m_position = camModel.GetTranslation3D();
			break;
		}
		default:
			break;
	}

	SetCameraModelMatrix(camModel);

	DisplayDebugInfo();
}


void GameCamera::DisplayDebugInfo()
{
	float fwdOffset = 0.5f;
	Mat44 camModel = m_modelMatrix;
	Vec3 inFrontOfPlayer = m_modelMatrix.GetTranslation3D() + m_modelMatrix.GetIBasis3D() * fwdOffset;
	float basisLength = 0.01f;
	float basisThickness = 0.003f;
	DebugAddWorldLine(inFrontOfPlayer, inFrontOfPlayer + Vec3(1.0f, 0.0f, 0.0f) * basisLength, Rgba8::RED, basisThickness, 0.0f, DebugRenderMode::ALWAYS);
	DebugAddWorldLine(inFrontOfPlayer, inFrontOfPlayer + Vec3(0.0f, 1.0f, 0.0f) * basisLength, Rgba8::GREEN, basisThickness, 0.0f, DebugRenderMode::ALWAYS);
	DebugAddWorldLine(inFrontOfPlayer, inFrontOfPlayer + Vec3(0.0f, 0.0f, 1.0f) * basisLength, Rgba8::BLUE, basisThickness, 0.0f, DebugRenderMode::ALWAYS);
	static float uiOrthoY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	std::string cameraModeName;
	switch (m_cameraMode)
	{
		case GAME_CAMERA_MODE_FIRST_PERSON:
			cameraModeName = "First Person";
			break;
		case GAME_CAMERA_MODE_OVER_SHOULDER:
			cameraModeName = "Over the shoulder";
			break;
		case GAME_CAMERA_MODE_FIXED_ANGLE_TRACKING:
			cameraModeName = "Fixed Angle";
			break;
		case GAME_CAMERA_MODE_SPECTATOR:
			cameraModeName = "Spectator";
			break;
		case GAME_CAMERA_MODE_COUNT:
			break;
		default:
			break;
	}
	DebugAddScreenText(Stringf("Camera Mode: %s", cameraModeName.c_str()), Vec2(0.0f, uiOrthoY), 0.0f, Vec2(0.0f, 1.0f), 16.0f);
}


void GameCamera::CheckInput(float deltaSeconds)
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		GameCameraMode nextMode = (GameCameraMode) ((m_cameraMode + 1) % GAME_CAMERA_MODE_COUNT);
		SetCameraMode(nextMode);
	}

	if (m_cameraMode == GAME_CAMERA_MODE_SPECTATOR)
	{
		Vec3 positionDelta;
		EulerAngles orientationDelta;

		if (g_inputSystem->WasKeyJustPressed(KEYCODE_CTRL))
		{
			Mat44 identityModel = Mat44();
			m_modelMatrix = identityModel;
		}

		Vec3 forwardVec = m_modelMatrix.GetIBasis3D();
		Vec3 leftVec = m_modelMatrix.GetJBasis3D();
		Vec3 upVec = m_modelMatrix.GetKBasis3D();

		static float movementSpeed = g_gameConfigBlackboard.GetValue("playerMovementSpeed", 0.0f);
		static float sprintSpeed = g_gameConfigBlackboard.GetValue("playerSprintSpeed", 0.0f);
		static float mouseSensitivity = g_gameConfigBlackboard.GetValue("playerMouseSensitivity", 0.0f);
		
		float speed = movementSpeed;
		if (g_inputSystem->IsKeyPressed(KEYCODE_SPACE_BAR))
		{
			speed = sprintSpeed;
		}

		if (g_inputSystem->IsKeyPressed('W'))
		{
			positionDelta += deltaSeconds * speed * forwardVec.GetXYVec3();
		}

		if (g_inputSystem->IsKeyPressed('A'))
		{
			positionDelta += deltaSeconds * speed * leftVec.GetXYVec3();
		}

		if (g_inputSystem->IsKeyPressed('S'))
		{
			positionDelta -= deltaSeconds * speed * forwardVec.GetXYVec3();
		}

		if (g_inputSystem->IsKeyPressed('D'))
		{
			positionDelta -= deltaSeconds * speed * leftVec.GetXYVec3();
		}

		if (g_inputSystem->IsKeyPressed('E'))
		{
			positionDelta.z += deltaSeconds * speed;
		}

		if (g_inputSystem->IsKeyPressed('Q'))
		{
			positionDelta.z -= deltaSeconds * speed;
		}

		IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
		orientationDelta.m_yaw -= mouseSensitivity * mouseDelta.x;
		orientationDelta.m_pitch += mouseSensitivity * mouseDelta.y;

		m_orientaion += orientationDelta;
		m_orientaion.m_pitch = Clamp(m_orientaion.m_pitch, -89.5f, 89.5f);
		m_position += positionDelta;
	}
}


void GameCamera::AttachToEntity(Entity* entity)
{
	if (m_attachedEntity)
	{
		DetachFromEntity();
	}

	m_attachedEntity = entity;
	m_attachedEntity->OnCameraAttached(this);
}


void GameCamera::DetachFromEntity()
{
	m_attachedEntity->OnCameraDetached();
	m_attachedEntity = nullptr;
}


void GameCamera::SetCameraMode(GameCameraMode newCameraMode)
{
	m_cameraMode = newCameraMode;
	if (m_cameraMode == GAME_CAMERA_MODE_SPECTATOR)
	{
		DetachFromEntity();
	}
	else
	{
		Player* player = g_game->GetPlayer();
		AttachToEntity(player);
	}
}


GameCameraMode GameCamera::GetCameraMode() const
{
	return m_cameraMode;
}

