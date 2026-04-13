#include "Game/Player.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/Block.hpp"
#include "Game/GameCamera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/World.hpp"

Player::Player()
{
	m_movementSpeed		= g_gameConfigBlackboard.GetValue("playerMovementSpeed",	m_movementSpeed);
	m_sprintSpeed		= g_gameConfigBlackboard.GetValue("playerSprintSpeed",		m_sprintSpeed);
	m_mouseSensitivity	= g_gameConfigBlackboard.GetValue("playerMouseSensitivity", m_mouseSensitivity);
}


Player::Player(Vec3 const& position)
	: Entity(position)
{
	m_movementSpeed		= g_gameConfigBlackboard.GetValue("playerMovementSpeed",	m_movementSpeed);
	m_sprintSpeed		= g_gameConfigBlackboard.GetValue("playerSprintSpeed",		m_sprintSpeed);
	m_mouseSensitivity	= g_gameConfigBlackboard.GetValue("playerMouseSensitivity", m_mouseSensitivity);
	std::string placementBlockName = g_gameConfigBlackboard.GetValue("playerPlacementBlock1", "");
	m_placementBlockDefId = BlockDef::GetBlockDefIdByName(placementBlockName);

	m_eyeHeight = g_gameConfigBlackboard.GetValue("playerEyePos", 1.65f);
	float playerHeight = g_gameConfigBlackboard.GetValue("playerHeight", 1.8f);
	float playerWidth = g_gameConfigBlackboard.GetValue("playerWidth", 0.6f);
	m_localBounds.m_mins = Vec3(-playerWidth * 0.5f, -playerWidth * 0.5f, 0.0f);
	m_localBounds.m_maxs = Vec3(playerWidth * 0.5f, playerWidth * 0.5f, playerHeight);

	m_physicsMode = ENTITY_PHYSICS_MODE_FLYING;
}


Player::~Player()
{
}


void Player::Update(float deltaSeconds)
{
	CheckKeyboardAndMouseInput(deltaSeconds);
	CheckGamepadInput(deltaSeconds);
	m_orientation.m_pitch = Clamp(m_orientation.m_pitch, -85.0f, 85.0f);
	m_orientation.m_roll = Clamp(m_orientation.m_roll, -45.0f, 45.0f);

	CalculateWorldPhysicsForces();
	MovePlayer(deltaSeconds);
	ResetFrameAcceleration();

	DisplayDebugInformationOnScreen();
}


void Player::DisplayDebugInformationOnScreen()
{
	static float uiX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float uiY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	std::string placementBlockName = BlockDef::GetBlockDefById(m_placementBlockDefId)->m_name;
	DebugAddScreenText(Stringf("Placement block: %s", placementBlockName.c_str()), Vec2(0.0f, uiY - 17.0f), 0.0f, Vec2(0.0f, 1.0f), 16.0f);
	switch (m_physicsMode)
	{
		case ENTITY_PHYSICS_MODE_WALKING:
			DebugAddScreenText(Stringf("PhysicsMode: Walking"), Vec2(0.0f, uiY - 34.0f), 0.0f, Vec2(0.0f, 1.0f), 16.0f);
			break;												
		case ENTITY_PHYSICS_MODE_FLYING:						
			DebugAddScreenText(Stringf("PhysicsMode: Flying"), Vec2(0.0f, uiY - 34.0f), 0.0f, Vec2(0.0f, 1.0f), 16.0f);
			break;												
		case ENTITY_PHYSICS_MODE_NO_CLIP:						
			DebugAddScreenText(Stringf("PhysicsMode: No Clip"), Vec2(0.0f, uiY - 34.0f), 0.0f, Vec2(0.0f, 1.0f), 16.0f);
			break;
		default:
			break;
	}
}


WorldRaycastResult GetRaycastForPointInDirection(Vec3 const& playerCenter, Vec3 const& startPos, Vec3 const& direction, float distance)
{
	Vec3 centerToPos = (startPos - playerCenter);
	float dot = DotProduct3D(centerToPos, direction);
	WorldRaycastResult result = {};
	static float raycastBackupDistance = g_gameConfigBlackboard.GetValue("raycastBackupDistance", 0.1f);
	if (dot >= 0.0f)
	{
		return g_world->RaycastWorld(startPos, direction, distance, raycastBackupDistance);
	}

	return result;
}


void Player::MovePlayer(float deltaSeconds)
{
	m_velocity += m_frameAcceleration * deltaSeconds;
	
	Vec3 playerBoundsHalfDims = m_localBounds.GetHalfDimensions();
	Vec3 bottomNorthEast = m_bottomCenterPos + Vec3(playerBoundsHalfDims.x, playerBoundsHalfDims.y);
	Vec3 bottomNorthWest = m_bottomCenterPos + Vec3(-playerBoundsHalfDims.x, playerBoundsHalfDims.y);
	Vec3 bottomSouthEast = m_bottomCenterPos + Vec3(playerBoundsHalfDims.x, -playerBoundsHalfDims.y);
	Vec3 bottomSouthWest = m_bottomCenterPos + Vec3(-playerBoundsHalfDims.x, -playerBoundsHalfDims.y);
	Vec3 midNorthEast = m_bottomCenterPos + Vec3(playerBoundsHalfDims.x, playerBoundsHalfDims.y, playerBoundsHalfDims.z);
	Vec3 midNorthWest = m_bottomCenterPos + Vec3(-playerBoundsHalfDims.x, playerBoundsHalfDims.y, playerBoundsHalfDims.z);
	Vec3 midSouthEast = m_bottomCenterPos + Vec3(playerBoundsHalfDims.x, -playerBoundsHalfDims.y, playerBoundsHalfDims.z);
	Vec3 midSouthWest = m_bottomCenterPos + Vec3(-playerBoundsHalfDims.x, -playerBoundsHalfDims.y, playerBoundsHalfDims.z);
	Vec3 topNorthEast = m_bottomCenterPos + Vec3(playerBoundsHalfDims.x, playerBoundsHalfDims.y, playerBoundsHalfDims.z * 2.0f);
	Vec3 topNorthWest = m_bottomCenterPos + Vec3(-playerBoundsHalfDims.x, playerBoundsHalfDims.y, playerBoundsHalfDims.z * 2.0f);
	Vec3 topSouthEast = m_bottomCenterPos + Vec3(playerBoundsHalfDims.x, -playerBoundsHalfDims.y, playerBoundsHalfDims.z * 2.0f);
	Vec3 topSouthWest = m_bottomCenterPos + Vec3(-playerBoundsHalfDims.x, -playerBoundsHalfDims.y, playerBoundsHalfDims.z * 2.0f);
	Vec3 boundsCenter = m_bottomCenterPos + Vec3(0.0f, 0.0f, playerBoundsHalfDims.z);

	float tForMovementOwed = 1.0f;
	while (tForMovementOwed > 0.0f)
	{
		Vec3 frameDisplacement = m_velocity * deltaSeconds * tForMovementOwed;
		Vec3 frameDisplacementCopy = frameDisplacement;
		float frameDistance = frameDisplacementCopy.GetLengthAndNormalise();
		Vec3 displacementDir = frameDisplacementCopy;

		if (frameDistance > 0)
		{
			WorldRaycastResult movementRaycasts[12];
			static float raycastVisualisingDuration = g_gameConfigBlackboard.GetValue("raycastVisualisingDuration", 0.0f);
			movementRaycasts[0] = GetRaycastForPointInDirection(boundsCenter, bottomNorthEast, displacementDir, frameDistance);
			movementRaycasts[1] = GetRaycastForPointInDirection(boundsCenter, bottomNorthWest, displacementDir, frameDistance);
			movementRaycasts[2] = GetRaycastForPointInDirection(boundsCenter, bottomSouthEast, displacementDir, frameDistance);
			movementRaycasts[3] = GetRaycastForPointInDirection(boundsCenter, bottomSouthWest, displacementDir, frameDistance);
			movementRaycasts[4] = GetRaycastForPointInDirection(boundsCenter, midNorthEast, displacementDir, frameDistance);
			movementRaycasts[5] = GetRaycastForPointInDirection(boundsCenter, midNorthWest, displacementDir, frameDistance);
			movementRaycasts[6] = GetRaycastForPointInDirection(boundsCenter, midSouthEast, displacementDir, frameDistance);
			movementRaycasts[7] = GetRaycastForPointInDirection(boundsCenter, midSouthWest, displacementDir, frameDistance);
			movementRaycasts[8] = GetRaycastForPointInDirection(boundsCenter, topNorthEast, displacementDir, frameDistance);
			movementRaycasts[9] = GetRaycastForPointInDirection(boundsCenter, topNorthWest, displacementDir, frameDistance);
			movementRaycasts[10] = GetRaycastForPointInDirection(boundsCenter, topSouthEast, displacementDir, frameDistance);
			movementRaycasts[11] = GetRaycastForPointInDirection(boundsCenter, topSouthWest, displacementDir, frameDistance);

// 			if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
// 			{
// 				VisualiseRaycast(movementRaycasts[0], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[1], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[2], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[3], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[4], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[5], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[6], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[7], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[8], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[9], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[10], raycastVisualisingDuration);
// 				VisualiseRaycast(movementRaycasts[11], raycastVisualisingDuration);
//			}

			WorldRaycastResult closestImpactMovementRaycast = GetClosestImpactRaycast(movementRaycasts, 12);
			if (closestImpactMovementRaycast.m_didHit && m_physicsMode != ENTITY_PHYSICS_MODE_NO_CLIP)
			{
				m_velocity -= GetProjectedOnto3D(m_velocity, closestImpactMovementRaycast.m_impactNormal);
				m_bottomCenterPos += frameDisplacement * closestImpactMovementRaycast.m_impactFraction;
				tForMovementOwed -= closestImpactMovementRaycast.m_impactFraction;
			}
			else
			{
				m_bottomCenterPos += frameDisplacement;
				tForMovementOwed = 0.0f;
			}
		}
		else
		{
			break;
		}
	}

	Vec3 downwardDir = Vec3(0.0f, 0.0f, -1.0f);
	m_isOnGround = false;
	static float groundedEntityDistance = g_gameConfigBlackboard.GetValue("groundedEntityDistance", 0.1f);
	WorldRaycastResult groundRaycasts[4];
	groundRaycasts[0] = g_world->RaycastWorld(bottomNorthEast, downwardDir, groundedEntityDistance);
	groundRaycasts[1] = g_world->RaycastWorld(bottomSouthEast, downwardDir, groundedEntityDistance);
	groundRaycasts[2] = g_world->RaycastWorld(bottomNorthWest, downwardDir, groundedEntityDistance);
	groundRaycasts[3] = g_world->RaycastWorld(bottomSouthWest, downwardDir, groundedEntityDistance);
	for (int raycastIndex = 0; raycastIndex < 4; raycastIndex++)
	{
		bool didHit = groundRaycasts[raycastIndex].m_didHit;
		if (didHit)
		{
			m_isOnGround = true;
			break;
		}
	}
}


void Player::CalculateWorldPhysicsForces()
{
	Vec3 downwardDir = Vec3(0.0f, 0.0f, -1.0f);
	Vec3 upwardDir = Vec3(0.0f, 0.0f, 1.0f);
	//compute gravity related stuff
	if (m_physicsMode == ENTITY_PHYSICS_MODE_WALKING)
	{
		if (IsOnGround())
		{
			float downwardVelocity = DotProduct3D(m_velocity, downwardDir);
			if (downwardVelocity > 0)
			{
				Vec3 accelerationFromGround;
				accelerationFromGround += downwardVelocity * upwardDir;
				AddAcceleration(accelerationFromGround);
			}
		}
		else
		{
			Vec3 gravityAcceleration = Vec3(0.0f, 0.0f, -1.0f) * g_gravity;
			AddAcceleration(gravityAcceleration);
		}
	}

	Vec3 velocityCopy = m_velocity;
	float speed = velocityCopy.GetLengthAndNormalise();
	Vec3 velocityDir = velocityCopy;
	//compute horizontal forces
	if (speed > 0)
	{
		static float airHorizontalFrictionModifier = g_gameConfigBlackboard.GetValue("airHorizontalFrictionModifier", 0.2f);
		Vec3 horizontalFrictionAcceleration = Vec3(0.0f, 0.0f, 0.0f);
		if (IsOnGround() && m_physicsMode == ENTITY_PHYSICS_MODE_WALKING)
		{
			horizontalFrictionAcceleration = g_horizontalFriction * speed * -velocityDir.GetXYVec3();
		}
		else
		{
			horizontalFrictionAcceleration = g_horizontalFriction * airHorizontalFrictionModifier * speed * -velocityDir.GetXYVec3();
		}
		AddAcceleration(horizontalFrictionAcceleration);
	}
}


void Player::Render() const
{
	if (m_attachedCamera == nullptr || m_attachedCamera->GetCameraMode() != GAME_CAMERA_MODE_FIRST_PERSON)
	{
		DebugRender();
	}
}


void Player::DebugRender() const
{
	AABB3 worldBounds = GetWorldBounds();
	DebugAddWorldWireAABB3(worldBounds, 0.0f, Rgba8::CYAN, DebugRenderMode::XRAY);
}


void Player::SetMovementSpeed(float moveSpeed)
{
	m_movementSpeed = moveSpeed;
}


void Player::SetMouseSensitivity(float mouseSens)
{
	m_mouseSensitivity = mouseSens;
}


void Player::SetSprintSpeed(float sprintSpeed)
{
	m_sprintSpeed = sprintSpeed;
}


float Player::GetMoveSpeed() const
{
	return m_movementSpeed;
}


float Player::GetSprintSpeed() const
{
	return m_sprintSpeed;
}


float Player::GetMouseSensitivity() const
{
	return m_mouseSensitivity;
}


BlockDef const* Player::GetPlacementBlockDef() const
{
	BlockDef const* placingBlockDef = BlockDef::GetBlockDefById(m_placementBlockDefId);
	return placingBlockDef;
}


uint8_t Player::GetPlacementBlockId() const
{
	return m_placementBlockDefId;
}


void Player::CheckKeyboardAndMouseInput(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		EntityPhysicsMode nextMode = (EntityPhysicsMode) ((m_physicsMode + 1) % ENTITY_PHYSICS_MODE_COUNT);
		m_physicsMode = nextMode;
	}

	if (m_attachedCamera == nullptr)
		return;

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_CTRL))
	{
		m_bottomCenterPos = Vec3(0.0f, 0.0f, 0.0f);
		m_orientation = EulerAngles(0.0f, 0.0f, 0.0f);
	}

	Vec3 forwardVec;
	Vec3 leftVec;
	Vec3 upVec;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forwardVec, leftVec, upVec);

	static float maxAcceleration = g_gameConfigBlackboard.GetValue("playerMaxAcceleration", 5.0f);
	static float sprintAccelerationModifier = g_gameConfigBlackboard.GetValue("playerSprintAccelerationModifier", 1.5f);
	float playerAcceleration = maxAcceleration;
	if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
	{
		playerAcceleration *= sprintAccelerationModifier;
	}

	Vec3 movementDir = Vec3(0.0f, 0.0f, 0.0f);
	if (g_inputSystem->IsKeyPressed('W'))
	{
		movementDir += forwardVec.GetXYVec3();
	}

	if (g_inputSystem->IsKeyPressed('A'))
	{
		movementDir += leftVec.GetXYVec3();
	}

	if (g_inputSystem->IsKeyPressed('S'))
	{
		movementDir -= forwardVec.GetXYVec3();
	}

	if (g_inputSystem->IsKeyPressed('D'))
	{
		movementDir -= leftVec.GetXYVec3();
	}

	if (m_physicsMode != ENTITY_PHYSICS_MODE_WALKING)
	{
		if (g_inputSystem->IsKeyPressed('E'))
		{
			movementDir += Vec3(0.0f, 0.0f, 1.0f);
		}

		if (g_inputSystem->IsKeyPressed('Q'))
		{
			movementDir += Vec3(0.0f, 0.0f, -1.0f);
		}

		if (g_inputSystem->IsKeyUp('Q') && g_inputSystem->IsKeyUp('E'))
		{
			m_velocity.z = 0.0f;
		}
	}
	
	if (movementDir != Vec3(0.0f, 0.0f, 0.0f))
	{
		Vec3 movementAcceleration = movementDir.GetNormalized() * playerAcceleration;
		AddAcceleration(movementAcceleration);
	}

	if (m_physicsMode == ENTITY_PHYSICS_MODE_WALKING && IsOnGround() && g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR))
	{
		static float playerJumpAcceleration = g_gameConfigBlackboard.GetValue("playerJumpAcceleration", 20.0f);
		Vec3 jumpAcceleration = Vec3(0.0f, 0.0f, playerJumpAcceleration);
		AddAcceleration(jumpAcceleration);
	}

	IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
	m_orientation.m_yaw -= m_mouseSensitivity * mouseDelta.x;
	m_orientation.m_pitch += m_mouseSensitivity * mouseDelta.y;

	if (g_inputSystem->WasKeyJustPressed('1'))
	{
		std::string blockDefName = g_gameConfigBlackboard.GetValue("playerPlacementBlock1", "");
		m_placementBlockDefId = BlockDef::GetBlockDefIdByName(blockDefName);
	}

	if (g_inputSystem->WasKeyJustPressed('2'))
	{
		std::string blockDefName = g_gameConfigBlackboard.GetValue("playerPlacementBlock2", "");
		m_placementBlockDefId = BlockDef::GetBlockDefIdByName(blockDefName);
	}
}


void Player::CheckGamepadInput(float deltaSeconds)
{
	XboxController const& controller = g_inputSystem->GetController(0);

	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_Y))
	{
		m_bottomCenterPos = Vec3(0.0f, 0.0f, 0.0f);
		m_orientation = EulerAngles(0.0f, 0.0f, 0.0f);
	}

	Vec3 forwardVec;
	Vec3 leftVec;
	Vec3 upVec;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forwardVec, leftVec, upVec);


	float speed = m_movementSpeed;
	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_LEFT_JOYSTICK_BUTTON))
	{
		speed = m_sprintSpeed;
	}

	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_RIGHT_BUMPER))
	{
		m_bottomCenterPos.z += deltaSeconds * speed;
	}

	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_LEFT_BUMPER))
	{
		m_bottomCenterPos.z -= deltaSeconds * speed;
	}

	AnalogJoystick leftStick = controller.GetLeftStick();
	Vec2 leftStickPos = leftStick.GetPosition();
	m_bottomCenterPos += forwardVec * deltaSeconds * leftStickPos.y * speed;
	m_bottomCenterPos += leftVec * deltaSeconds * -leftStickPos.x * speed;

	AnalogJoystick rightStick = controller.GetRightStick();
	Vec2 rightStickPos = rightStick.GetPosition();
	m_orientation.m_yaw -= rightStickPos.x * m_mouseSensitivity;
	m_orientation.m_pitch -= rightStickPos.y * m_mouseSensitivity;
}
