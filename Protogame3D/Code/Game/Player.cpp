#include "Game/Player.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/EventSystem.hpp"

Player::Player()
{

}


Player::Player(Vec3 const& position)
	: Entity(position)
{
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
}


void Player::Render() const
{
}


void Player::DebugRender() const
{
}


void Player::SetMovementSpeed(float moveSpeed)
{
	m_movementSpeed = moveSpeed;
}


void Player::SetRotationSpeed(float mouseSens)
{
	m_rotationSpeed = mouseSens;
}


void Player::SetRollSpeed(float rollSpeed)
{
	m_rollSpeed = rollSpeed;
}


void Player::SetSprintSpeed(float sprintSpeed)
{
	m_sprintSpeed = sprintSpeed;
}


float Player::GetMoveSpeed() const
{
	return m_movementSpeed;
}


float Player::GetRollSpeed() const
{
	return m_rollSpeed;
}


float Player::GetSprintSpeed() const
{
	return m_sprintSpeed;
}


float Player::GetRotationSpeed() const
{
	return m_rotationSpeed;
}


void Player::CheckKeyboardAndMouseInput(float deltaSeconds)
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_CTRL))
	{
		m_position = Vec3(0.0f, 0.0f, 0.0f);
		m_orientation = EulerAngles(0.0f, 0.0f, 0.0f);
	}

	Vec3 forwardVec;
	Vec3 leftVec;
	Vec3 upVec;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forwardVec, leftVec, upVec);

	float speed = m_movementSpeed;
	if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
	{
		speed = m_sprintSpeed;
	}

	if (g_inputSystem->IsKeyPressed('W'))
	{
		m_position += deltaSeconds * speed * forwardVec;
	}

	if (g_inputSystem->IsKeyPressed('A'))
	{
		m_position += deltaSeconds * speed * leftVec;
	}

	if (g_inputSystem->IsKeyPressed('S'))
	{
		m_position -= deltaSeconds * speed * forwardVec;
	}

	if (g_inputSystem->IsKeyPressed('D'))
	{
		m_position -= deltaSeconds * speed * leftVec;
	}

	if (g_inputSystem->IsKeyPressed('F'))
	{
		m_position.z += deltaSeconds * speed;
	}

	if (g_inputSystem->IsKeyPressed('V'))
	{
		m_position.z -= deltaSeconds * speed;
	}

	if (g_inputSystem->IsKeyPressed('E'))
	{
		m_orientation.m_roll += deltaSeconds * m_rollSpeed;
	}

	if (g_inputSystem->IsKeyPressed('Q'))
	{
		m_orientation.m_roll -= deltaSeconds * m_rollSpeed;
	}

	float playerMouseSensitivity = 0.1f;
	IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
	m_orientation.m_yaw -= playerMouseSensitivity * mouseDelta.x;
	m_orientation.m_pitch += playerMouseSensitivity * mouseDelta.y;
}


void Player::CheckGamepadInput(float deltaSeconds)
{
	XboxController const& controller = g_inputSystem->GetController(0);

	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_Y))
	{
		m_position = Vec3(0.0f, 0.0f, 0.0f);
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
		m_position.z += deltaSeconds * speed;
	}

	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_LEFT_BUMPER))
	{
		m_position.z -= deltaSeconds * speed;
	}

	float rightTrigger = controller.GetRightTrigger();
	if (rightTrigger > 0.0f)
	{
		m_orientation.m_roll += deltaSeconds * m_rollSpeed * rightTrigger;
	}

	float leftTrigger = controller.GetLeftTrigger();
	if (leftTrigger > 0.0f)
	{
		m_orientation.m_roll -= deltaSeconds * m_rollSpeed * leftTrigger;
	}

	AnalogJoystick leftStick = controller.GetLeftStick();
	Vec2 leftStickPos = leftStick.GetPosition();
	m_position += forwardVec * deltaSeconds * leftStickPos.y * speed;
	m_position += leftVec * deltaSeconds * -leftStickPos.x * speed;

	AnalogJoystick rightStick = controller.GetRightStick();
	Vec2 rightStickPos = rightStick.GetPosition();
	m_orientation.m_yaw -= rightStickPos.x * deltaSeconds * m_rotationSpeed;
	m_orientation.m_pitch -= rightStickPos.y * deltaSeconds * m_rotationSpeed;
}
