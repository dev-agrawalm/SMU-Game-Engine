#include "Game/ActorDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteAnimation::SpriteAnimation(std::string const& framesCSV)
{
	FromCSV(framesCSV);
}


bool SpriteAnimation::FromCSV(std::string const& framesCSV)
{
	Strings animationSpriteIndices = SplitStringOnDelimiter(framesCSV, ',');
	if ((int) animationSpriteIndices.size() == 0)
		return false;

	for (int spriteFrame = 0; spriteFrame < (int) animationSpriteIndices.size(); spriteFrame++)
	{
		std::string spriteIndexText = animationSpriteIndices[spriteFrame];
		int spriteIndex = atoi(spriteIndexText.c_str());
		m_frameIndices.push_back(spriteIndex);
	}
	return true;
}


bool ViewDependentSpriteAnimationGroup::LoadFromXmlElement(XmlElement const& elem)
{
	m_name = ParseXmlAttribute(elem, "name", m_name);
	m_scaleBySpeed = ParseXmlAttribute(elem, "scaleBySpeed", m_scaleBySpeed);
	m_secondsPerFrame = ParseXmlAttribute(elem, "secondsPerFrame", m_secondsPerFrame);
	m_framesPerSecond = 1.0f / m_secondsPerFrame;

	bool isSuccess = true;
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "front",		Vec3(1.0f	,0.0f	,0.0f));
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "frontLeft",	Vec3(1.0f	,1.0f	,0.0f));
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "left",		Vec3(0.0f	,1.0f	,0.0f));
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "backLeft",	Vec3(-1.0f	,1.0f	,0.0f));
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "back",		Vec3(-1.0f	,0.0f	,0.0f));
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "backRight",	Vec3(-1.0f	,-1.0f	,0.0f));
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "right",		Vec3(0.0f	,-1.0f	,0.0f));
	isSuccess &= LoadSpriteAnimationForDirectionFromXmlElement(elem, "frontRight",	Vec3(1.0f	,-1.0f	,0.0f));
	return isSuccess;
}


bool ViewDependentSpriteAnimationGroup::LoadSpriteAnimationForDirectionFromXmlElement(XmlElement const& elem, std::string dirString, Vec3 const& direction)
{
	std::string animCSV = ParseXmlAttribute(elem, dirString.c_str(), "");
	if (animCSV != "")
	{
		SpriteAnimation animation;
		if (animation.FromCSV(animCSV))
		{
			m_directions.push_back(direction.GetNormalized());
			m_animations.push_back(animation);
		}
		else
		{
			ERROR_RECOVERABLE("Unable to parse csv for animation group (" + dirString + ") in animation (" + m_name + ")");
			return false;
		}
	}

	return true;
}


SpriteAnimation const* ViewDependentSpriteAnimationGroup::GetBestAnimationForDirection(Vec3 const& dir) const
{
	float dirAgreement = -9999.0f;
	int bestDirIndex = 0;
	for (int dirIndex = 0; dirIndex < (int) m_directions.size(); dirIndex++)
	{
		Vec3 direction = m_directions[dirIndex];
		float agreementLevel = DotProduct3D(direction, dir);
		if (agreementLevel > dirAgreement)
		{
			bestDirIndex = dirIndex;
			dirAgreement = agreementLevel;
		}
	}

	SpriteAnimation const& animation = m_animations[bestDirIndex];
	return &animation;
}


ActorAppearance::~ActorAppearance()
{
	if (m_spritesheet != nullptr)
	{
		delete m_spritesheet;
		m_spritesheet = nullptr;
	}
}


bool ActorAppearance::LoadFromXmlElement(XmlElement const& elem)
{
	m_size = ParseXmlAttribute(elem, "size", m_size);
	m_pivot = ParseXmlAttribute(elem, "pivot", m_pivot);
	m_defaultAnimName = ParseXmlAttribute(elem, "defaultAnim", "");

	std::string spriteSheetText = ParseXmlAttribute(elem, "spritesheet", "");
	IntVec2 spriteSheetCellDims = ParseXmlAttribute(elem, "cellCount", IntVec2(0, 0));
	if (spriteSheetText != "" && spriteSheetCellDims != IntVec2(0, 0))
	{
		Texture* spriteSheetTexture = g_theRenderer->CreateOrGetTexture(spriteSheetText.c_str());
		m_spritesheet = new SpriteSheet(*spriteSheetTexture, spriteSheetCellDims);
	}

	std::string billboardTypeText = ParseXmlAttribute(elem, "billboard", "");
	if (_stricmp(billboardTypeText.c_str(), "CameraAlignedXY") == 0)
	{
		m_billboard = BillboardType::CAMERA_ALIGNED_XY;
	}
	else if (_stricmp(billboardTypeText.c_str(), "CameraFacingXY") == 0)
	{
		m_billboard = BillboardType::CAMERA_FACING_XY;
	}
	else if (_stricmp(billboardTypeText.c_str(), "CameraFacing") == 0)
	{
		m_billboard = BillboardType::CAMERA_FACING;
	}

	XmlElement const* animationElement = elem.FirstChildElement("AnimationGroup");
	while (animationElement != nullptr)
	{
		ViewDependentSpriteAnimationGroup spriteAnimGroup;
		if (spriteAnimGroup.LoadFromXmlElement(*animationElement))
		{
			m_animationGroups.push_back(spriteAnimGroup);
		}
		else
			return false;

		animationElement = animationElement->NextSiblingElement("AnimationGroup");
	}

	std::string shaderName = ParseXmlAttribute(elem, "shader", "Invalid");
	if (_stricmp(shaderName.c_str(), "Invalid") != 0)
	{
		m_shader = g_theRenderer->CreateOrGetShader(shaderName.c_str());
	}

	return true;
}


Mat44 ActorAppearance::GetBillboardMatrix(Vec3 const& objPos, Camera const& viewer) const
{
	Mat44 camModelMat = viewer.GetModelMatrix();
	Mat44 billboardMat;
	switch (m_billboard)
	{
		case BillboardType::CAMERA_FACING_XY:
		{
			Vec3 camPos = camModelMat.GetTranslation3D();
			Vec3 forwardDir = (camPos - objPos).GetNormalized().GetVec2().GetVec3().GetNormalized();
			Vec3 upDir = Vec3(0.0f, 0.0f, 1.0f);
			Vec3 leftDir = CrossProduct3D(upDir, forwardDir).GetNormalized();
			billboardMat = Mat44(forwardDir, leftDir, upDir, objPos);
			break;
		}
		case BillboardType::CAMERA_ALIGNED_XY:
		{
			Vec3 forwardDir = -camModelMat.GetIBasis3D().GetVec2().GetVec3().GetNormalized();
			Vec3 upDir = Vec3(0.0f, 0.0f, 1.0f);
			Vec3 leftDir = CrossProduct3D(upDir, forwardDir).GetNormalized();;
			billboardMat = Mat44(forwardDir, leftDir, upDir, objPos);
			break;
		}
		case BillboardType::CAMERA_FACING:
		{
			Vec3 camPos = camModelMat.GetTranslation3D();
			Vec3 forwardDir = (camPos - objPos).GetNormalized();
			billboardMat = Mat44::CreateLookAtMatrix_XFwd_YLeft_ZUp(forwardDir);
			break;
		}
		case BillboardType::NONE:
		default:
			break;
	}
	return billboardMat;
}


SpriteInfo ActorAppearance::GetSpriteInfo(Vec3 const& facing) const
{
	UNUSED(facing);
	SpriteInfo info = {};
	return info;
}


ViewDependentSpriteAnimationGroup ActorAppearance::GetAnimationGroupByName(std::string const& name) const
{
	for (int animGroupIndex = 0; animGroupIndex < (int) m_animationGroups.size(); animGroupIndex++)
	{
		ViewDependentSpriteAnimationGroup const& animGroup = m_animationGroups[animGroupIndex];
		if (_stricmp(animGroup.m_name.c_str(), name.c_str()) == 0)
		{
			return animGroup;
		}
	}

	ViewDependentSpriteAnimationGroup animGroup = {};
	animGroup.m_name = "null";
	return animGroup;
}


void ActorDefinition::LoadActorDefinitionsFromXMLFile(std::string filePath)
{
	XmlDocument xmlDoc;
	XmlError result = xmlDoc.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(result == 0, "Unable to load file for actor definitions: " + filePath);

	XmlElement* rootElement = xmlDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "Definitions") == 0, "Actor Definition file does not have definitions element: " + filePath);

	XmlElement* definitionElement = rootElement->FirstChildElement("ActorDefinition");
	while (definitionElement)
	{
		ActorDefinition* definition = new ActorDefinition();
		definition->LoadFromXmlElement(*definitionElement);
		GUARANTEE_OR_DIE(!DoesDefinitionExist(definition->m_name), "Duplicate data in actor definitions: " + filePath);
		s_actorDefinitions.push_back(definition);

		definitionElement = definitionElement->NextSiblingElement("ActorDefinition");
	}
}


ActorDefinition const* ActorDefinition::GetActorDefinition(std::string definitionName)
{
	for (int defIndex = 0; defIndex < (int) s_actorDefinitions.size(); defIndex++)
	{
		ActorDefinition const*& def = s_actorDefinitions[defIndex];
		if (_stricmp(def->m_name.c_str(), definitionName.c_str()) == 0)
		{
			return def;
		}
	}

	return nullptr;
}


bool ActorDefinition::DoesDefinitionExist(std::string definitionName)
{
	for (int defIndex = 0; defIndex < (int) s_actorDefinitions.size(); defIndex++)
	{
		ActorDefinition const*& def = s_actorDefinitions[defIndex];
		if (_stricmp(def->m_name.c_str(), definitionName.c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}


void ActorDefinition::DeleteAllActorDefinitions()
{
	for (int defIndex = 0; defIndex < (int) s_actorDefinitions.size(); defIndex++)
	{
		ActorDefinition const*& def = s_actorDefinitions[defIndex];
		if (def)
		{
			delete def;
			def = nullptr;
		}
	}

	s_actorDefinitions.clear();
}


bool ActorDefinition::LoadFromXmlElement(XmlElement const& node)
{
	m_name		 = ParseXmlAttribute(node, "name", m_name);
	m_options	 = ACTOR_OPTION_NONE;
	m_health	 = ParseXmlAttribute(node, "health", m_health);
	m_lingerTime = ParseXmlAttribute(node, "lingerTime", m_lingerTime);
	m_options	|= ParseXmlAttribute(node, "possessable", false) ? ACTOR_OPTION_CAN_BE_POSSESSED : ACTOR_OPTION_NONE;
	m_options	|= ParseXmlAttribute(node, "dieOnSpawn", false) ? ACTOR_OPTION_DIE_ON_SPAWN : ACTOR_OPTION_NONE;
	m_deathSfx	 = ParseXmlAttribute(node, "sfxDeath", "");
	m_painSfx	 = ParseXmlAttribute(node, "sfxPain", "");

	XmlElement const* collisionElement = node.FirstChildElement("Collision");
	if (collisionElement)
	{
		m_physicsRadius = ParseXmlAttribute(*collisionElement, "radius", m_physicsRadius);
		m_physicsHeight = ParseXmlAttribute(*collisionElement, "height", m_physicsHeight);
		m_physicsBase	= ParseXmlAttribute(*collisionElement, "height", m_physicsBase);
		m_options |= ParseXmlAttribute(*collisionElement, "pushOffWorld",	false) ? ACTOR_OPTION_COLLIDES_WITH_MAP		: ACTOR_OPTION_NONE;
		m_options |= ParseXmlAttribute(*collisionElement, "pushOffActors",	false) ? ACTOR_OPTION_COLLIDES_WITH_ACTORS	: ACTOR_OPTION_NONE;
		m_options |= ParseXmlAttribute(*collisionElement, "immovable",		false) ? ACTOR_OPTION_IMMOVABLE				: ACTOR_OPTION_NONE;
		m_options |= ParseXmlAttribute(*collisionElement, "dieOnCollision",	false) ? ACTOR_OPTION_DIE_ON_COLLISION		: ACTOR_OPTION_NONE;
		m_options |= ParseXmlAttribute(*collisionElement, "immovable",		false) ? ACTOR_OPTION_IMMOVABLE				: ACTOR_OPTION_NONE;
		m_damageOnTouch = ParseXmlAttribute(*collisionElement, "damageOnTouch", m_damageOnTouch);
		if (m_damageOnTouch != FloatRange::ZERO)
			m_options |= ACTOR_OPTION_DAMAGE_ON_TOUCH;
		m_impulseOnTouch = ParseXmlAttribute(*collisionElement, "impulseOnTouch", m_impulseOnTouch);
		if (m_impulseOnTouch != -1.0f)
			m_options |= ACTOR_OPTION_IMPULSE_ON_TOUCH;
	}

	XmlElement const* physicsElement = node.FirstChildElement("Physics");
	if (physicsElement)
	{
		m_options |= ParseXmlAttribute(*physicsElement, "enabled", false) ? ACTOR_OPTION_PHYSICS_UPDATE : ACTOR_OPTION_NONE;
		m_options |= ParseXmlAttribute(*physicsElement, "flying", false) ? ACTOR_OPTION_CAN_FLY : ACTOR_OPTION_NONE;
		m_walkSpeed			= ParseXmlAttribute(*physicsElement, "walkSpeed",	m_walkSpeed);
		m_runSpeed			= ParseXmlAttribute(*physicsElement, "runSpeed",	m_runSpeed);
		Vec3 turnSpeed		= ParseXmlAttribute(*physicsElement, "turnSpeed", Vec3(0.0f, 0.0f, 0.0f));
		m_turnSpeed.m_yaw	= turnSpeed.x;
		m_turnSpeed.m_pitch = turnSpeed.y;
		m_turnSpeed.m_roll	= turnSpeed.z;
		m_drag				= ParseXmlAttribute(*physicsElement, "drag",		m_drag);
	}

	XmlElement const* appearanceElement = node.FirstChildElement("Appearance");
	if (appearanceElement)
	{
		if (!m_appearance.LoadFromXmlElement(*appearanceElement))
		{
			ERROR_AND_DIE("Unable to read actor appearance xml element for actor: " + m_name);
		}
	}
	if (!m_appearance.m_defaultAnimName.empty())
	{
		m_options |= ACTOR_OPTION_HAS_DEFAULT_ANIM;
	}
	
	XmlElement const* cameraElement = node.FirstChildElement("Camera");
	if (cameraElement)
	{
		m_eyeHeight			= ParseXmlAttribute(*cameraElement, "eyeHeight", m_eyeHeight);
		m_cameraFOVDegrees	= ParseXmlAttribute(*cameraElement, "cameraFOV", m_cameraFOVDegrees);
		m_deadEyeHeight		= ParseXmlAttribute(*cameraElement, "deadEyeHeight", m_deadEyeHeight);
	}

	XmlElement const* aiElement = node.FirstChildElement("AI");
	if (aiElement)
	{
		m_options |= ACTOR_OPTION_HAS_AI;
		m_aiType = ParseXmlAttribute(*aiElement, "type", "");
		m_sightAngle = ParseXmlAttribute(*aiElement, "sightAngle", m_sightAngle);
		m_sightRadius = ParseXmlAttribute(*aiElement, "sightRadius", m_sightRadius);
	}

	XmlElement const* inventoryElement = node.FirstChildElement("Inventory");
	if (inventoryElement)
	{
		XmlElement const* weaponElement = inventoryElement->FirstChildElement("Weapon");
		m_weaponInventory.clear();
		while (weaponElement)
		{
			std::string weaponName = ParseXmlAttribute(*weaponElement, "name", "");
			GUARANTEE_OR_DIE((int) weaponName.size() > 0, "Empty weapon name provided in inventory for actor definition: " + m_name);
			WeaponDefinition const* weaponDef = WeaponDefinition::GetWeaponDefinitionByName(weaponName);
			if (weaponDef == nullptr)
				ERROR_AND_DIE("Null weapon definition for weapon: " + weaponName);
			m_weaponInventory.push_back(weaponDef);

			weaponElement = weaponElement->NextSiblingElement("Weapon");
		}
	}

	return true;
}


std::vector<ActorDefinition const*> ActorDefinition::s_actorDefinitions;
