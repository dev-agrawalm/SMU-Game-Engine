#include "Game/PlayerController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameMode.hpp"

bool g_spawnActorFromConsole = false;
SpawnInfo g_spawnInfo = {};

bool SpawnActor(EventArgs& args)
{
	g_spawnInfo = {};
	g_spawnActorFromConsole = false;
	std::string actorType = args.GetProperty("type", "");
	
	if (actorType.size() > 0)
	{
		ActorDefinition const* actorDef = ActorDefinition::GetActorDefinition(actorType);
		if (actorDef == nullptr)
		{
			g_console->AddLine(DevConsole::ERROR_MESSAGE, "Invalid actor type");
			return false;
		}
		g_spawnInfo.m_definition = actorDef;
		g_spawnActorFromConsole = true;
		return true;
	}
	
	return false;
}


PlayerController::PlayerController(World* world, int playerIndex, int controllerIndex) 
	: Controller(world)
	, m_playerIndex(playerIndex)
	, m_controllerIndex(controllerIndex)
{
	m_worldCamera.DefineGameSpace(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));
	Mat44 persProjMat = Mat44::CreatePerspectiveProjectionMatrix(60.0f, g_window->GetWindowConfig().m_aspectRatio, 0.1f, 100.0f);
	m_worldCamera.SetProjectionMatrix(persProjMat);
	m_movementSpeed = g_gameConfigBlackboard.GetValue("playerSpeed", m_movementSpeed);
	m_position = Vec3(4.5f, 4.5f, 1.5f);

	g_eventSystem->SubscribeEventCallbackFunction("SpawnActor", SpawnActor);
}


PlayerController::~PlayerController()
{
	g_eventSystem->UnsubscribeEventCallbackFunction("SpawnActor", SpawnActor);
}


void PlayerController::PreMapUpdate(float deltaSeconds)
{
	Actor* possesedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
	if (possesedActor == nullptr || g_game->IsEditorModeActive() && m_controllerIndex == -1)
	{
		CheckKeyboardAndMouseInput_EditorMode(deltaSeconds);
		CheckGamepadInput_EditorMode(deltaSeconds);
	}
}


void PlayerController::PostMapUpdate(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	Actor* possesedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
	if (possesedActor && !g_game->IsEditorModeActive())
	{
		float fov = possesedActor->GetCameraFOV();
		Mat44 perspectiveMat = Mat44::CreatePerspectiveProjectionMatrix(fov, g_window->GetWindowConfig().m_aspectRatio, 0.1f, 100.0f);
		m_worldCamera.SetProjectionMatrix(perspectiveMat);
		
		Vec3 eyePos = possesedActor->IsDead()? possesedActor->GetDeadEyePosition() : possesedActor->GetEyePosition();
		EulerAngles orientation = possesedActor->GetOrientation();
		Mat44 camModel = Mat44::CreateTranslation3D(eyePos);
		camModel.Append(orientation.GetAsMatrix_XFwd_YLeft_ZUp());
		m_worldCamera.SetCameraModelMatrix(camModel);
		
		m_position = eyePos;
		m_orientation = orientation;
	}
	else
	{
		m_worldCamera.SetCameraModelMatrix(GetModelMatrix());
		EditorModeUpdate();
	}

}


void PlayerController::RenderHUD()
{
	Actor* possessedActor = GetPossessedActor();
	if (possessedActor && !g_game->IsEditorModeActive())
	{
		float hudWidth = m_hudDims.x;
		float hudHeight = m_hudDims.y;

		float bottomHUDWidth = hudWidth * 0.8f;
		float bottomHUDHeight = hudHeight * 0.1f;
		Vec2 bottomHudCenter = Vec2(hudWidth * 0.5f, bottomHUDHeight * 0.5f);
		AABB2 bottomHudQuad = AABB2(bottomHudCenter, bottomHUDWidth, bottomHUDHeight);
		std::vector<Vertex_PCU> bottomHudVerts;
		AddVertsForAABB2ToVector(bottomHudVerts, bottomHudQuad, Rgba8::WHITE);
		
		std::string bottonHudTexturePath = g_gameConfigBlackboard.GetValue("playerHUDImage", "Default");
		Texture* bottomHudTexture = g_theRenderer->CreateOrGetTexture(bottonHudTexturePath.c_str());
		g_theRenderer->BindTexture(0, bottomHudTexture);
		g_theRenderer->DrawVertexArray((int) bottomHudVerts.size(), bottomHudVerts.data());

		int health = possessedActor->GetHealth();
		std::vector<Vertex_PCU> textVerts;
		BitmapFont* textFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		Vec2 healthTextBoxCenter = bottomHudQuad.m_mins + Vec2(bottomHUDWidth * 0.3f, bottomHUDHeight * 0.55f);
		AABB2 healthTextBounds = AABB2(healthTextBoxCenter, bottomHUDWidth * 0.3f, bottomHUDHeight * 0.55f);
		std::string healthText = health > 0 ? Stringf("%i", health) : "DEAD";
		textFont->AddVertsForTextInAABB2(textVerts, healthTextBounds, healthTextBounds.GetDimensions().y, healthText);
		g_theRenderer->BindTexture(0, &textFont->GetTexture());
		g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());

		if (possessedActor->IsDead())
		{
			RenderDeadHud();
		}

		if (possessedActor->HasEquippedWeapon())
		{
			Weapon* equippedWeapon = possessedActor->GetEquippedWeapon();
			if (equippedWeapon != nullptr)
				RenderWeaponHUD(equippedWeapon);
		}

		TintHUD();
	}
}


void PlayerController::RenderWeaponHUD(Weapon* weapon)
{
	float hudWidth = m_hudDims.x;
	float hudHeight = m_hudDims.y;

	Texture* reticleTexture = weapon->GetReticle();
	Vec2 reticleDims = weapon->GetReticleDims();
	AABB2 reticleBounds = AABB2(Vec2(hudWidth * 0.5f, hudHeight * 0.5f), reticleDims.x, reticleDims.y);
	std::vector<Vertex_PCU> reticleVerts;
	AddVertsForAABB2ToVector(reticleVerts, reticleBounds, Rgba8::WHITE);

	g_theRenderer->BindTexture(0, reticleTexture);
	g_theRenderer->DrawVertexArray((int) reticleVerts.size(), reticleVerts.data());

	SpriteDefinition weaponSpriteDef = weapon->GetWeaponSprite();
	Vec2 spriteScale = weapon->GetSpriteScale();
	AABB2 weaponSpriteBounds = AABB2(Vec2(hudWidth * 0.5f, hudHeight * 0.1f + spriteScale.y * 0.5f), spriteScale.x, spriteScale.y);
	std::vector<Vertex_PCU> weaponVerts;
	Vec2 uvMins;
	Vec2 uvMaxs;
	weaponSpriteDef.GetUVs(uvMins, uvMaxs);
	AddVertsForAABB2ToVector(weaponVerts, weaponSpriteBounds, Rgba8::WHITE, uvMins, uvMaxs);

	g_theRenderer->BindTexture(0, &weaponSpriteDef.GetTexture());
	g_theRenderer->DrawVertexArray((int) weaponVerts.size(), weaponVerts.data());
}


void PlayerController::TintHUD()
{
	Rgba8 tint = GetCurrentTint();
	if (tint.a > 0)
	{
		AABB2 hudQuad = AABB2(0.0f, 0.0f, m_hudDims.x, m_hudDims.y);
		std::vector<Vertex_PCU> tintVerts;
		AddVertsForAABB2ToVector(tintVerts, hudQuad, tint);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->DrawVertexArray((int) tintVerts.size(), tintVerts.data());
	}
}


void PlayerController::ApplyTint(Rgba8 const& startTint, Rgba8 const& endTint, float duration)
{
	m_startTint = startTint;
	m_endTint = endTint;
	m_tintTimer.Start(duration);
}


void PlayerController::ApplyTint(Rgba8 const& endTint, float duration)
{
	m_startTint = GetCurrentTint();
	m_endTint = endTint;
	m_tintTimer.Start(duration);
}


void PlayerController::SetMovementSpeed(float moveSpeed)
{
	m_movementSpeed = moveSpeed;
}


void PlayerController::SetWorldCameraViewport(AABB2 const& viewport)
{
	m_worldCamera.SetScreenViewport(viewport);
}


void PlayerController::SetScreenCameraViewport(AABB2 const& viewport)
{
	m_screenCamera.SetScreenViewport(viewport);
	
	float viewportHeight = viewport.m_maxs.y - viewport.m_mins.y;
	float viewportWidth = viewport.m_maxs.x - viewport.m_mins.x;
	float viewportAspect = viewportWidth / viewportHeight;

	float hudHeight = g_gameConfigBlackboard.GetValue("playerHudHeight", 720.0f);
	float hudWidth = viewportAspect * hudHeight;
	m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(hudWidth, hudHeight));
	m_hudDims = Vec2(hudWidth, hudHeight);
}


void PlayerController::SetNormalisedViewport(AABB2 const& normalisedViewport)
{
	m_normalisedViewport = normalisedViewport;
}


void PlayerController::SetRespawnTime(float respawnTime)
{
	m_respawnTimer.SetDuration((double) respawnTime);
}


float PlayerController::GetMoveSpeed() const
{
	return m_movementSpeed;
}


Camera const& PlayerController::GetWorldCamera() const
{
	return m_worldCamera;
}


Camera const& PlayerController::GetScreenCamera() const
{
	return m_screenCamera;
}


Mat44 const PlayerController::GetModelMatrix() const
{
	Mat44 modelMatrix;
	modelMatrix = Mat44::CreateTranslation3D(m_position);
	Mat44 oritentationMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.Append(oritentationMat);
	return modelMatrix;
}


Vec3 const PlayerController::GetPosition() const
{
	return m_position;
}


Vec3 PlayerController::GetForward() const
{
	return m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
}


int PlayerController::GetPlayerIndex() const
{
	return m_playerIndex;
}


Rgba8 PlayerController::GetCurrentTint() const
{
	float elapsedTimeFraction = m_tintTimer.GetElapsedFraction();
	elapsedTimeFraction = ClampZeroToOne(elapsedTimeFraction);
	return Lerp(m_startTint, m_endTint, elapsedTimeFraction);
}


int PlayerController::GetControllerIndex() const
{
	return m_controllerIndex;
}


Vec2 PlayerController::GetHUDDims() const
{
	return m_hudDims;
}


void PlayerController::Think(float deltaSeconds)
{
	if (m_possessedActorUID.IsValid())
	{
		Actor* possessedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
		if (possessedActor && g_game->IsEditorModeActive())
		{
			DebugAddWorldWireSphere(possessedActor->GetPosition(), 0.3f, 0.0f, Rgba8::GREEN, DebugRenderMode::XRAY);
		}
	}

	if (g_game->IsEditorModeActive())
		return;

	if (m_controllerIndex >= 0)
		CheckGamepadInput_Possessed(deltaSeconds);
	if (m_controllerIndex == -1)
		CheckKeyboardAndMouseInput_Possessed(deltaSeconds);
}


void PlayerController::SpawnAndPossessRandomActorFromSpawnPoint()
{
	std::vector<Actor*> spawnPoints = m_currentMap->GetActorListByFaction(ACTOR_FACTION_MARINE_SPAWN);
	Actor* spawnPoint = nullptr;
	if (spawnPoints.size() > 0)
	{
		int randomSpawnPointIndex = g_rng->GetRandomIntLessThan((int) spawnPoints.size());
		spawnPoint = spawnPoints[randomSpawnPointIndex];
	}

	SpawnInfo info = {};
	if (spawnPoint)
	{
		info.m_position =  spawnPoint->GetPosition();
		info.m_orientation =  spawnPoint->GetOrientation();
	}
	else
	{
		Tile tile = m_currentMap->GetRandomNonSolidTile();
		info.m_position = m_currentMap->GetTileCenter(tile.m_gridCoords);
	}
	info.m_definition = ActorDefinition::GetActorDefinition("Marine");
	info.m_faction = ACTOR_FACTION_MARINES;
	Actor* spawnedActor = m_currentMap->SpawnActor(info);
	PossessActor(spawnedActor);
}


void PlayerController::OnActorPossessed(Actor* actor)
{
	if (actor != nullptr)
	{
		m_possessedActorUID = actor->GetActorUID();
		actor->OnPossessed(this);
	}
	else
	{
		m_possessedActorUID.Invalidate();
	}

	m_startTint = Rgba8::WHITE_TRANSPARENT;
	m_endTint = Rgba8::WHITE_TRANSPARENT;
	m_respawnTimer.Stop();
}


void PlayerController::OnPossessedActorDamaged()
{
	float damageTintDuration = g_gameConfigBlackboard.GetValue("damageTintDuration", 2.5f);
	ApplyTint(Rgba8(255, 0, 0, 100), Rgba8::RED_TRANSPARENT, damageTintDuration);
}


void PlayerController::OnPossessedActorKilled()
{
	float deathTintDuration = g_gameConfigBlackboard.GetValue("deathTintDuration", 2.5f);
	ApplyTint(Rgba8(0, 0, 0, 100), deathTintDuration);

	m_respawnTimer.Restart();
}


void PlayerController::RenderDeadHud()
{
	Vec2 hudDims = m_hudDims;
	float timeLeftToRespawn = (float) m_respawnTimer.GetDuration() - (float) m_respawnTimer.GetElapsedSeconds();
	if (timeLeftToRespawn < 0.0f)
		timeLeftToRespawn = 0.0f;

	std::string respawnInText = Stringf("Respawn in: %.2f seconds", timeLeftToRespawn);
	std::string respawnInstructionsText = "Press Space or A on the controller to respawn";

	std::vector<Vertex_PCU> textVerts;
	BitmapFont* textFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 respawnTimeTextBox = AABB2(hudDims * 0.5f, hudDims.x * 0.5f, hudDims.y * 0.3f);
	Vec2 respawnInstructionsTextBoxCenter = respawnTimeTextBox.GetCenter() - Vec2(0.0f, hudDims.y * 0.3f);
	AABB2 respawnInstructionsTextBox = AABB2(respawnInstructionsTextBoxCenter, hudDims.x * 0.5f, hudDims.y * 0.1f);

	textFont->AddVertsForTextInAABB2(textVerts, respawnTimeTextBox, hudDims.y * 0.2f, respawnInText);
	textFont->AddVertsForTextInAABB2(textVerts, respawnInstructionsTextBox, hudDims.y * 0.1f, respawnInstructionsText);

	g_theRenderer->BindTexture(0, &textFont->GetTexture());
	g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
}


void PlayerController::CheckKeyboardAndMouseInput_EditorMode(float deltaSeconds)
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
	float sprintSpeedMultiplier = 2.0f;
	if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
	{
		speed *= sprintSpeedMultiplier;
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

	if (g_inputSystem->IsKeyPressed('E'))
	{
		m_position.z += deltaSeconds * speed;
	}

	if (g_inputSystem->IsKeyPressed('Q'))
	{
		m_position.z -= deltaSeconds * speed;
	}

	if (g_inputSystem->WasKeyJustPressed('H'))
	{
		DebugAddWorldBasis(GetModelMatrix(), -1.0f);
	}

	IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
	m_orientation.m_yaw -= deltaSeconds * mouseDelta.x;
	m_orientation.m_pitch += deltaSeconds * mouseDelta.y;
}


void PlayerController::CheckGamepadInput_EditorMode(float deltaSeconds)
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
	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_RIGHT_BUMPER))
	{
		m_position.z += deltaSeconds * speed;
	}

	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_LEFT_BUMPER))
	{
		m_position.z -= deltaSeconds * speed;
	}

	AnalogJoystick leftStick = controller.GetLeftStick();
	Vec2 leftStickPos = leftStick.GetPosition();
	m_position += forwardVec * deltaSeconds * leftStickPos.y * speed;
	m_position += leftVec * deltaSeconds * -leftStickPos.x * speed;

	float controllerSpeed = g_gameConfigBlackboard.GetValue("playerJoystickSensitivityEditorMode", 20.0f);
	AnalogJoystick rightStick = controller.GetRightStick();
	Vec2 rightStickPos = rightStick.GetPosition();
	m_orientation.m_yaw -= rightStickPos.x * deltaSeconds * controllerSpeed;
	m_orientation.m_pitch -= rightStickPos.y * deltaSeconds * controllerSpeed;
}


void PlayerController::EditorModeUpdate()
{
	if (m_controllerIndex != -1)
		return;

	Map* currentMap = m_world->GetCurrentMap();
	Actor* hitActor = nullptr;

	Vec3 rayStart = m_position;
	Vec3 rayForward = GetForward();
	float rayLength = 100.0f;
	RaycastResult raycast = currentMap->RayCastAll(rayStart, rayForward, rayLength, ActorUID::INVALID, true);
	if (raycast.m_didImpact)
	{
		Vec3 impactPoint = raycast.m_impactPoint;
		Vec3 impactNormal = raycast.m_impactNormal;
		DebugAddWorldArrow(impactPoint, impactPoint + impactNormal, 0.0f, Rgba8::BLACK, 0.125f, DebugRenderMode::XRAY);

		bool actorImpact = raycast.m_impactActorUID.IsValid();
		if (actorImpact)
		{
			hitActor = currentMap->GetActorWithUID(raycast.m_impactActorUID);
			if (hitActor)
			{
				float actorCylinderHeight = hitActor->GetCollisionHeight();
				float actorCylinderRadius = hitActor->GetCollisionRadius();
				Vec3 actorCylinderBase	= hitActor->GetPosition();
				Vec3 actorCylinderTop	= Vec3(actorCylinderBase.x, actorCylinderBase.y, actorCylinderHeight);
				DebugAddWorldWireCylinder(actorCylinderBase, actorCylinderTop, actorCylinderRadius, 0.0f, Rgba8::YELLOW, DebugRenderMode::XRAY);
			}
		}

		if (g_inputSystem->WasMouseButtonJustPressed(1) && !actorImpact)
		{
			if (impactPoint.z == 0.0f)
			{
				Tile tile = m_currentMap->GetTile(impactPoint);
				if (!tile.IsSolid())
				{
					Actor* possessedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
					if (possessedActor)
						possessedActor->SetPosition(impactPoint);
				}
			}
		}

		if (impactPoint.z == 0.0f && g_spawnActorFromConsole)
		{
			Tile tile = m_currentMap->GetTile(impactPoint);
			if (!tile.IsSolid())
			{
				g_spawnInfo.m_position = impactPoint;
				if (g_spawnInfo.m_definition->m_name == "Marine")
					g_spawnInfo.m_faction = ACTOR_FACTION_MARINES;
				if (g_spawnInfo.m_definition->m_name == "Pinky")
					g_spawnInfo.m_faction = ACTOR_FACTION_DEMONS;
				m_currentMap->SpawnActor(g_spawnInfo);
				g_spawnActorFromConsole = false;
			}
		}
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F4))
	{
		if (hitActor)
		{
			hitActor->MakeGarbage();
		}
	}
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		if (hitActor)
		{
			PossessActor(hitActor);
		}
	}
}


void PlayerController::CheckKeyboardAndMouseInput_Possessed(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	Actor* possessedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
	if (possessedActor == nullptr)
		return;

	Mat44 actorModelMat = possessedActor->GetModelMatrix();
	Vec2 actorForwardXY = actorModelMat.GetIBasis3D().GetVec2();
	Vec2 actorLeftXY = actorModelMat.GetJBasis3D().GetVec2();

	Vec2 desiredAccelDir = Vec2(0.0f, 0.0f); 
	if (g_inputSystem->IsKeyPressed('W'))
	{
		desiredAccelDir += actorForwardXY;
	}

	if (g_inputSystem->IsKeyPressed('A'))
	{
		desiredAccelDir += actorLeftXY;
	}

	if (g_inputSystem->IsKeyPressed('S'))
	{
		desiredAccelDir += -actorForwardXY;
	}

	if (g_inputSystem->IsKeyPressed('D'))
	{
		desiredAccelDir += -actorLeftXY;
	}

	if (desiredAccelDir != Vec2(0.0f, 0.0f))
	{
		desiredAccelDir.Normalize();

		float walkSprintRatio = 0.0f;
		if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
		{
			walkSprintRatio = 1.0f;
		}

		possessedActor->MoveInDirection(desiredAccelDir.GetVec3(), 1.0f, walkSprintRatio);
	}

	float mouseSensitivity = g_gameConfigBlackboard.GetValue("playerMouseSensitivity", 0.01f);
	IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
	EulerAngles orientationDelta = EulerAngles((float) -mouseDelta.x * mouseSensitivity, (float) mouseDelta.y * mouseSensitivity, 0.0f);
	possessedActor->TurnByOrientation(orientationDelta);

	if (g_inputSystem->IsMouseButtonPressed(0))
	{
		possessedActor->Attack(g_inputSystem->WasMouseButtonJustPressed(0));
	}

	if (g_inputSystem->WasKeyJustPressed('1'))
	{
		possessedActor->EquipWeapon(0);
	}

	if (g_inputSystem->WasKeyJustPressed('2'))
	{
		possessedActor->EquipWeapon(1);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) && possessedActor->IsDead() && m_respawnTimer.HasElapsed())
	{
		GameMode* gameMode = m_world->GetGameMode();
		gameMode->SpawnActorForPlayer(this);
	}
}


void PlayerController::CheckGamepadInput_Possessed(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	XboxController const& controller = g_inputSystem->GetController(m_controllerIndex);
	Actor* possessedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
	if (possessedActor == nullptr)
		return;

	Mat44 actorModelMat = possessedActor->GetModelMatrix();
	Vec2 actorForwardXY = actorModelMat.GetIBasis3D().GetVec2();
	Vec2 actorLeftXY = actorModelMat.GetJBasis3D().GetVec2();

	AnalogJoystick const& leftStick = controller.GetLeftStick();
	Vec2 leftStickPos = leftStick.GetPosition();
	Vec2 desiredAccelDir = leftStickPos.y * actorForwardXY + leftStickPos.x * -actorLeftXY;//leftStick.GetPosition();// Vec2(0.0f, 0.0f);
	if (desiredAccelDir != Vec2(0.0f, 0.0f))
	{
		desiredAccelDir.Normalize();

		float walkSprintRatio = 0.0f;
		if (controller.IsButtonDown(XboxController::XBOX_BUTTON_LEFT_JOYSTICK_BUTTON))
		{
			walkSprintRatio = 1.0f;
		}

		possessedActor->MoveInDirection(desiredAccelDir.GetVec3(), 1.0f, walkSprintRatio);
	}

	float controllerSpeed = g_gameConfigBlackboard.GetValue("playerJoystickSensitivity", 5.0f);
	AnalogJoystick const& rightStick = controller.GetRightStick();
	Vec2 rightStickPos = rightStick.GetPosition();
	EulerAngles orientationDelta = EulerAngles(-rightStickPos.x * controllerSpeed, -rightStickPos.y * controllerSpeed, 0.0f);
	possessedActor->TurnByOrientation(orientationDelta);

	if (controller.IsButtonDown(XboxController::XBOX_BUTTON_RIGHT_BUMPER))
	{
		possessedActor->Attack(controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_RIGHT_BUMPER));
	}

	if (controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_Y))
	{
		possessedActor->EquipWeapon(0);
	}

	if (controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_X))
	{
		possessedActor->EquipWeapon(1);
	}

	if (controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_A) && possessedActor->IsDead() && m_respawnTimer.HasElapsed())
	{
		GameMode* gameMode = m_world->GetGameMode();
		gameMode->SpawnActorForPlayer(this);
	}
}
