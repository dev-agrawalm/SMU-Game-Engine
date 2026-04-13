#include "Game/EntityDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Game\Game.hpp"
#include "Engine\Renderer\SpriteSheet.hpp"
#include "Engine\Core\VertexUtils.hpp"

EntityDefinition EntityDefinition::s_defaultDefinitions[ENTITY_TYPE_COUNT];

//initializes all entity definitions
void EntityDefinition::InitialiseDefinitions()
{
	//MARIO ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties marioProps;
	EntitySpriteInfo marioEditorSpriteInfo = {};
	marioEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 64);
	marioEditorSpriteInfo.m_spriteCoords = IntVec2(4, 0);
	marioProps.AddProperty("editorSpriteInfo", marioEditorSpriteInfo);

	EntitySpriteInfo marioPaletteSpriteInfo = {};
	marioPaletteSpriteInfo.m_spriteSheetLayout = IntVec2(64, 64);
	marioPaletteSpriteInfo.m_spriteCoords = IntVec2(14, 0);
	marioProps.AddProperty("paletteSpriteInfo", marioPaletteSpriteInfo);

	EntityAnimationDataContainer marioAnimations;
	EntityAnimStateData marioIdleAnimData = {};
	marioIdleAnimData.m_secondsPerFrame = 0.25f;
	marioIdleAnimData.m_animStartIndex = 5;
	marioIdleAnimData.m_animEndIndex = 5;
	marioIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	marioIdleAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	marioAnimations.AddAnimation(ANIMATION_STATE_IDLE, marioIdleAnimData);

	EntityAnimStateData marioMovingAnimData = {};
	marioMovingAnimData.m_secondsPerFrame = 0.08f;
	marioMovingAnimData.m_animStartIndex = 5;
	marioMovingAnimData.m_animEndIndex = 7;
	marioMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::PING_PONG;
	marioMovingAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	marioAnimations.AddAnimation(ANIMATION_STATE_MOVING, marioMovingAnimData);

	EntityAnimStateData marioJumpingAnimData = {};
	marioJumpingAnimData.m_secondsPerFrame = 0.25f;
	marioJumpingAnimData.m_animStartIndex = 8;
	marioJumpingAnimData.m_animEndIndex = 8;
	marioJumpingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	marioJumpingAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	marioAnimations.AddAnimation(ANIMATION_STATE_JUMPING_UP, marioJumpingAnimData);

	EntityAnimStateData marioTakeDamageAnimData = {};
	marioTakeDamageAnimData.m_secondsPerFrame = 0.05f;
	marioTakeDamageAnimData.m_playbackDuration = 0.5f;
	marioTakeDamageAnimData.m_animStartIndex = 12;
	marioTakeDamageAnimData.m_animEndIndex = 13;
	marioTakeDamageAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	marioTakeDamageAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	marioTakeDamageAnimData.m_blocksOtherAnimations = true;
	marioAnimations.AddAnimation(ANIMATION_STATE_TAKE_DAMAGE, marioTakeDamageAnimData);

	EntityAnimStateData marioFallingAnimData = {};
	marioFallingAnimData.m_secondsPerFrame = 0.25f;
	marioFallingAnimData.m_animStartIndex = 9;
	marioFallingAnimData.m_animEndIndex = 9;
	marioFallingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	marioFallingAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	marioAnimations.AddAnimation(ANIMATION_STATE_FALLING_DOWN, marioFallingAnimData);
	marioProps.AddProperty("animationData", marioAnimations);

	marioProps.AddProperty("cosmeticHeight",				PLAYER_BIG_COSMETIC_HEIGHT);
	marioProps.AddProperty("cosmeticWidth",					PLAYER_BIG_COSMETIC_WIDTH);
	marioProps.AddProperty("physicsHeight",					PLAYER_BIG_PHYSICS_HEIGHT);
	marioProps.AddProperty("physicsWidth",					PLAYER_BIG_PHYSICS_WIDTH);

	uint32_t marioOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	marioProps.AddProperty("entityOptions", marioOptions);
	uint32_t marioCollisionLayers = ENTITY_COLLISION_LAYER_BLOCKS | ENTITY_COLLISION_LAYER_PICKUPS | ENTITY_COLLISION_LAYER_ENEMIES;
	marioProps.AddProperty("collisionLayers", marioCollisionLayers);
	AddDefinition(ENTITY_TYPE_MARIO, marioProps);

	//SMALL MARIO ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties smallMarioProps;
	EntitySpriteInfo smallMarioEditorSpriteInfo = {};
	smallMarioEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 64);
	smallMarioEditorSpriteInfo.m_spriteCoords = IntVec2(17, 0);
	smallMarioProps.AddProperty("editorSpriteInfo", smallMarioEditorSpriteInfo);

	EntityAnimationDataContainer smallMarioAnimations;
	EntityAnimStateData smallMarioIdleAnimData = {};
	smallMarioIdleAnimData.m_secondsPerFrame = 0.25f;
	smallMarioIdleAnimData.m_animStartIndex = 13;
	smallMarioIdleAnimData.m_animEndIndex = 13;
	smallMarioIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	smallMarioIdleAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	smallMarioAnimations.AddAnimation(ANIMATION_STATE_IDLE, smallMarioIdleAnimData);

	EntityAnimStateData smallMarioMovingAnimData = {};
	smallMarioMovingAnimData.m_secondsPerFrame = 0.08f;
	smallMarioMovingAnimData.m_animStartIndex = 13;
	smallMarioMovingAnimData.m_animEndIndex = 14;
	smallMarioMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::PING_PONG;
	smallMarioMovingAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	smallMarioAnimations.AddAnimation(ANIMATION_STATE_MOVING, smallMarioMovingAnimData);

	EntityAnimStateData smallMarioJumpingAnimData = {};
	smallMarioJumpingAnimData.m_secondsPerFrame = 0.25f;
	smallMarioJumpingAnimData.m_animStartIndex = 15;
	smallMarioJumpingAnimData.m_animEndIndex = 15;
	smallMarioJumpingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	smallMarioJumpingAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	smallMarioAnimations.AddAnimation(ANIMATION_STATE_JUMPING_UP, smallMarioJumpingAnimData);

	EntityAnimStateData smallMarioHealAnimData = {};
	smallMarioHealAnimData.m_secondsPerFrame = 0.05f;
	smallMarioHealAnimData.m_playbackDuration = 0.5f;
	smallMarioHealAnimData.m_animStartIndex = 18;
	smallMarioHealAnimData.m_animEndIndex = 19;
	smallMarioHealAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	smallMarioHealAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	smallMarioHealAnimData.m_blocksOtherAnimations = true;
	smallMarioAnimations.AddAnimation(ANIMATION_STATE_HEAL, smallMarioHealAnimData);

	EntityAnimStateData smallMarioDeathAnimData = {};
	smallMarioDeathAnimData.m_secondsPerFrame = 0.06f;
	smallMarioDeathAnimData.m_animStartIndex = 16;
	smallMarioDeathAnimData.m_animEndIndex = 19;
	smallMarioDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	smallMarioDeathAnimData.m_spriteSheetLayout = IntVec2(16, 16);
	smallMarioDeathAnimData.m_playbackDuration = 0.25f;
	smallMarioAnimations.AddAnimation(ANIMATION_STATE_DEATH, smallMarioDeathAnimData);

	EntityAnimStateData smallMarioFallingAnimData = {};
	smallMarioFallingAnimData.m_secondsPerFrame = 0.25f;
	smallMarioFallingAnimData.m_animStartIndex = 16;
	smallMarioFallingAnimData.m_animEndIndex = 16;
	smallMarioFallingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	smallMarioFallingAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	smallMarioAnimations.AddAnimation(ANIMATION_STATE_FALLING_DOWN, smallMarioFallingAnimData);
	smallMarioProps.AddProperty("animationData", smallMarioAnimations);

	smallMarioProps.AddProperty("cosmeticHeight",	PLAYER_SMALL_COSMETIC_HEIGHT);
	smallMarioProps.AddProperty("cosmeticWidth",	PLAYER_SMALL_COSMETIC_WIDTH);
	smallMarioProps.AddProperty("physicsHeight",	PLAYER_SMALL_PHYSICS_HEIGHT);
	smallMarioProps.AddProperty("physicsWidth",		PLAYER_SMALL_PHYSICS_WIDTH);

	uint32_t smallMarioOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	smallMarioProps.AddProperty("entityOptions", smallMarioOptions);
	uint32_t smallMarioCollisionLayers = ENTITY_COLLISION_LAYER_BLOCKS | ENTITY_COLLISION_LAYER_PICKUPS | ENTITY_COLLISION_LAYER_ENEMIES;
	smallMarioProps.AddProperty("collisionLayers", smallMarioCollisionLayers);
	AddDefinition(ENTITY_TYPE_SMALL_MARIO, smallMarioProps);

	//GOOMBA ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties goombaProps;
	EntitySpriteInfo goombaEditorSpriteInfo = {};
	goombaEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	goombaEditorSpriteInfo.m_spriteCoords = IntVec2(4, 2);
	goombaProps.AddProperty("editorSpriteInfo", goombaEditorSpriteInfo);

	EntityAnimationDataContainer goombaAnimations;
	EntityAnimStateData goombaMovingAnimData = {};
	goombaMovingAnimData.m_secondsPerFrame = 0.12f;
	goombaMovingAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	goombaMovingAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(4, 2), goombaMovingAnimData.m_spriteSheetLayout);
	goombaMovingAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(5, 2), goombaMovingAnimData.m_spriteSheetLayout);
	goombaMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::PING_PONG;
	goombaAnimations.AddAnimation(ANIMATION_STATE_MOVING, goombaMovingAnimData);

	EntityAnimStateData goombaDeathAnimData = {};
	goombaDeathAnimData.m_secondsPerFrame = 0.06f;
	goombaDeathAnimData.m_animStartIndex = 16;
	goombaDeathAnimData.m_animEndIndex = 19;
	goombaDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	goombaDeathAnimData.m_spriteSheetLayout = IntVec2(16, 16);
	goombaDeathAnimData.m_playbackDuration = 0.25f;
	goombaAnimations.AddAnimation(ANIMATION_STATE_DEATH, goombaDeathAnimData);
	goombaProps.AddProperty("animationData", goombaAnimations);
	
	goombaProps.AddProperty("cosmeticHeight",				GOOMBA_COSMETIC_HEIGHT);
	goombaProps.AddProperty("cosmeticWidth",				GOOMBA_COSMETIC_WIDTH);
	goombaProps.AddProperty("physicsHeight",				GOOMBA_PHYSICS_HEIGHT);
	goombaProps.AddProperty("physicsWidth",					GOOMBA_PHYSICS_WIDTH);
	
	uint32_t goombaOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	goombaProps.AddProperty("entityOptions", goombaOptions);
	uint32_t goombaCollisionLayers = ENTITY_COLLISION_LAYER_ENEMIES;
	goombaProps.AddProperty("collisionLayers", goombaCollisionLayers);
	AddDefinition(ENTITY_TYPE_GOOMBA, goombaProps);

	//COIN -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties coinProps;
	EntitySpriteInfo coinEditorSpriteInfo = {};
	coinEditorSpriteInfo.m_spriteSheetLayout = IntVec2(32, 32);
	coinEditorSpriteInfo.m_spriteCoords = IntVec2(1, 1);
	coinProps.AddProperty("editorSpriteInfo", coinEditorSpriteInfo);

	EntityAnimationDataContainer coinAnimations;
	EntityAnimStateData coinIdleAnimData = {};
	coinIdleAnimData.m_secondsPerFrame = 0.25f;
	coinIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(1, 1), IntVec2(32, 32));
	coinIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(1, 1), IntVec2(32, 32));;
	coinIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	coinIdleAnimData.m_spriteSheetLayout = IntVec2(32, 32);
	coinAnimations.AddAnimation(ANIMATION_STATE_IDLE, coinIdleAnimData);

	EntityAnimStateData coinDeathAnimData = {};
	coinDeathAnimData.m_secondsPerFrame = 0.05f;
	coinDeathAnimData.m_animStartIndex = 34;
	coinDeathAnimData.m_animEndIndex = 38;
	coinDeathAnimData.m_playbackDuration = 0.25f;
	coinDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::PING_PONG;
	coinDeathAnimData.m_spriteSheetLayout = IntVec2(32, 32);
	coinDeathAnimData.m_scale = 5.0f;
	coinAnimations.AddAnimation(ANIMATION_STATE_DEATH, coinDeathAnimData);
	coinProps.AddProperty("animationData", coinAnimations);
	
	coinProps.AddProperty("cosmeticHeight",					COIN_COSMETIC_HEIGHT);
	coinProps.AddProperty("cosmeticWidth",					COIN_COSMETIC_WIDTH);
	coinProps.AddProperty("physicsHeight",					COIN_PHYSICS_HEIGHT);
	coinProps.AddProperty("physicsWidth",					COIN_PHYSICS_WIDTH);
	
	uint32_t coinOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	coinProps.AddProperty("entityOptions", coinOptions);
	uint32_t coinCollisionLayers = ENTITY_COLLISION_LAYER_PICKUPS;
	coinProps.AddProperty("collisionLayers", coinCollisionLayers);
	AddDefinition(ENTITY_TYPE_COIN, coinProps);

	//LEVEL END -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties levelEndFlagProps;
	EntitySpriteInfo levelEndEditorSpriteInfo = {};
	levelEndEditorSpriteInfo.m_spriteSheetLayout = IntVec2(32, 32);
	levelEndEditorSpriteInfo.m_spriteCoords = IntVec2(0, 5);
	levelEndEditorSpriteInfo.m_pivot = Vec2(0.25f, 0.25f);
	levelEndFlagProps.AddProperty("editorSpriteInfo", levelEndEditorSpriteInfo);

	EntitySpriteInfo levelEndPaletteSpriteInfo = {};
	levelEndPaletteSpriteInfo.m_spriteSheetLayout = IntVec2(32, 32);
	levelEndPaletteSpriteInfo.m_spriteCoords = IntVec2(6, 0);
	levelEndFlagProps.AddProperty("paletteSpriteInfo", levelEndPaletteSpriteInfo);

	EntityAnimationDataContainer levelEndFlagAnimations;
	EntityAnimStateData levelEndIdleAnimData = {};
	levelEndIdleAnimData.m_secondsPerFrame = 0.25f;
	levelEndIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 5), IntVec2(32, 32));
	levelEndIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(0, 5), IntVec2(32, 32));;
	levelEndIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	levelEndIdleAnimData.m_spriteSheetLayout = IntVec2(32, 32);
	levelEndIdleAnimData.m_pivot = Vec2(0.25f, 0.25f);
	levelEndFlagAnimations.AddAnimation(ANIMATION_STATE_IDLE, levelEndIdleAnimData);

	EntityAnimStateData levelEndDeathAnimData = {};
	levelEndDeathAnimData.m_secondsPerFrame = 0.05f;
	levelEndDeathAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 5), IntVec2(32, 32));
	levelEndDeathAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(3, 5), IntVec2(32, 32));
	levelEndDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	levelEndDeathAnimData.m_spriteSheetLayout = IntVec2(32, 32);
	levelEndDeathAnimData.m_pivot = Vec2(0.25f, 0.25f);
	levelEndFlagAnimations.AddAnimation(ANIMATION_STATE_DEATH, levelEndDeathAnimData);
	levelEndFlagProps.AddProperty("animationData", levelEndFlagAnimations);
	
	levelEndFlagProps.AddProperty("cosmeticHeight",			FLAG_COSMETIC_HEIGHT);
	levelEndFlagProps.AddProperty("cosmeticWidth",			FLAG_COSMETIC_WIDTH);
	levelEndFlagProps.AddProperty("physicsHeight",			FLAG_PHYSICS_HEIGHT);
	levelEndFlagProps.AddProperty("physicsWidth",			FLAG_PHYSICS_WIDTH);
	
	uint32_t levelEndOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	levelEndFlagProps.AddProperty("entityOptions", levelEndOptions);
	uint32_t levelEndCollisionLayers = ENTITY_COLLISION_LAYER_PICKUPS;
	levelEndFlagProps.AddProperty("collisionLayers", levelEndCollisionLayers);
	AddDefinition(ENTITY_TYPE_LEVEL_END, levelEndFlagProps);

	//SWITCH ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties switchProps;
	EntitySpriteInfo switchEditorSpriteInfo = {};
	switchEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	switchEditorSpriteInfo.m_spriteCoords = IntVec2(2, 5);
	switchProps.AddProperty("editorSpriteInfo", switchEditorSpriteInfo);

	EntityAnimationDataContainer switchAnimations;
	EntityAnimStateData switchIdleAnimData = {};
	switchIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	switchIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(2, 5), IntVec2(128, 128));
	switchIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(3, 5), IntVec2(128, 128));
	switchIdleAnimData.m_secondsPerFrame = 0.2f;
	switchIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	switchAnimations.AddAnimation(ANIMATION_STATE_IDLE, switchIdleAnimData);

	EntityAnimStateData switchDeathAnimData = {};
	switchDeathAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	switchDeathAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(2, 4), IntVec2(128, 128));
	switchDeathAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(3, 4), IntVec2(128, 128));
	switchDeathAnimData.m_secondsPerFrame = 0.2f;
	switchDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	switchAnimations.AddAnimation(ANIMATION_STATE_DEATH, switchDeathAnimData);
	switchProps.AddProperty("animationData", switchAnimations);

	switchProps.AddProperty("cosmeticHeight",	SWITCH_COSMETIC_HEIGHT	);
	switchProps.AddProperty("cosmeticWidth",	SWITCH_COSMETIC_WIDTH	);
	switchProps.AddProperty("physicsHeight",	SWITCH_PHYSICS_HEIGHT	);
	switchProps.AddProperty("physicsWidth",		SWITCH_PHYSICS_WIDTH	);
	
	uint32_t switchOptions = ENTITY_OPTIONS_CAN_BE_STEPPED_ON | ENTITY_OPTIONS_IS_STATIC | ENTITY_OPTIONS_COLLIDES_WHILE_DEAD | ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	switchProps.AddProperty("entityOptions", switchOptions);
	uint32_t switchCollisionLayers = ENTITY_COLLISION_LAYER_BLOCKS | ENTITY_COLLISION_LAYER_PICKUPS | ENTITY_COLLISION_LAYER_ENEMIES;
	switchProps.AddProperty("collisionLayers", switchCollisionLayers);
	AddDefinition(ENTITY_TYPE_SWITCH_BLOCK, switchProps);

	//RED BLOCK --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties redBlockProps;
	EntitySpriteInfo redBlockEditorSpriteInfo = {};
	redBlockEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	redBlockEditorSpriteInfo.m_spriteCoords = IntVec2(0, 4);
	redBlockProps.AddProperty("editorSpriteInfo", redBlockEditorSpriteInfo);

	EntityAnimationDataContainer redBlockAnimations;
	EntityAnimStateData redBlockIdleAnimData = {};
	redBlockIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	redBlockIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 4), IntVec2(128, 128));
	redBlockIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(0, 4), IntVec2(128, 128));
	redBlockIdleAnimData.m_secondsPerFrame = 0.4f;
	redBlockIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	redBlockAnimations.AddAnimation(ANIMATION_STATE_IDLE, redBlockIdleAnimData);

	EntityAnimStateData redBlockDeathAnimData = {};
	redBlockDeathAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	redBlockDeathAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(1, 4), IntVec2(128, 128));
	redBlockDeathAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(1, 4), IntVec2(128, 128));
	redBlockDeathAnimData.m_secondsPerFrame = 0.4f;
	redBlockDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	redBlockAnimations.AddAnimation(ANIMATION_STATE_DEATH, redBlockDeathAnimData);
	redBlockProps.AddProperty("animationData", redBlockAnimations);

	redBlockProps.AddProperty("cosmeticHeight", RED_BLOCK_COSMETIC_HEIGHT);
	redBlockProps.AddProperty("cosmeticWidth",	RED_BLOCK_COSMETIC_WIDTH);
	redBlockProps.AddProperty("physicsHeight",	RED_BLOCK_PHYSICS_HEIGHT);
	redBlockProps.AddProperty("physicsWidth",	RED_BLOCK_PHYSICS_WIDTH);

	uint32_t redBlockOptions = ENTITY_OPTIONS_CAN_BE_STEPPED_ON | ENTITY_OPTIONS_IS_STATIC;
	redBlockProps.AddProperty("entityOptions", redBlockOptions);
	uint32_t redBlockCollisionLayers = ENTITY_COLLISION_LAYER_BLOCKS | ENTITY_COLLISION_LAYER_PICKUPS | ENTITY_COLLISION_LAYER_ENEMIES;
	redBlockProps.AddProperty("collisionLayers", redBlockCollisionLayers);
	AddDefinition(ENTITY_TYPE_RED_BLOCK, redBlockProps);

	//BLUE BLOCK --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties blueBlockProps;
	EntitySpriteInfo blueBlockEditorSpriteInfo = {};
	blueBlockEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	blueBlockEditorSpriteInfo.m_spriteCoords = IntVec2(1, 5);
	blueBlockProps.AddProperty("editorSpriteInfo", blueBlockEditorSpriteInfo);

	EntityAnimationDataContainer blueBlockAnimations;
	EntityAnimStateData blueBlockIdleAnimData = {};
	blueBlockIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	blueBlockIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 5), IntVec2(128, 128));
	blueBlockIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(0, 5), IntVec2(128, 128));
	blueBlockIdleAnimData.m_secondsPerFrame = 0.4f;
	blueBlockIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	blueBlockAnimations.AddAnimation(ANIMATION_STATE_IDLE, blueBlockIdleAnimData);

	EntityAnimStateData blueBlockDeathAnimData = {};
	blueBlockDeathAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	blueBlockDeathAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(1, 5), IntVec2(128, 128));
	blueBlockDeathAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(1, 5), IntVec2(128, 128));
	blueBlockDeathAnimData.m_secondsPerFrame = 0.4f;
	blueBlockDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	blueBlockAnimations.AddAnimation(ANIMATION_STATE_DEATH, blueBlockDeathAnimData);
	blueBlockProps.AddProperty("animationData", blueBlockAnimations);

	blueBlockProps.AddProperty("cosmeticHeight",	BLUE_BLOCK_COSMETIC_HEIGHT);
	blueBlockProps.AddProperty("cosmeticWidth",		BLUE_BLOCK_COSMETIC_WIDTH);
	blueBlockProps.AddProperty("physicsHeight",		BLUE_BLOCK_PHYSICS_HEIGHT);
	blueBlockProps.AddProperty("physicsWidth",		BLUE_BLOCK_PHYSICS_WIDTH);

	uint32_t blueBlockOptions = ENTITY_OPTIONS_CAN_BE_STEPPED_ON | ENTITY_OPTIONS_IS_STATIC;
	blueBlockProps.AddProperty("entityOptions", blueBlockOptions);
	uint32_t blueBlockCollisionLayers = ENTITY_COLLISION_LAYER_BLOCKS | ENTITY_COLLISION_LAYER_PICKUPS | ENTITY_COLLISION_LAYER_ENEMIES;
	blueBlockProps.AddProperty("collisionLayers", blueBlockCollisionLayers);
	AddDefinition(ENTITY_TYPE_BLUE_BLOCK, blueBlockProps);

	//BREAK BLOCK -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties breakBlockProps;
	EntitySpriteInfo breakBlockEditorSpriteInfo = {};
	breakBlockEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	breakBlockEditorSpriteInfo.m_spriteCoords = IntVec2(0, 7);
	breakBlockProps.AddProperty("editorSpriteInfo", breakBlockEditorSpriteInfo);

	EntityAnimationDataContainer breakBlockAnimations;
	EntityAnimStateData breakBlockIdleAnimData = {};
	breakBlockIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	breakBlockIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 7), IntVec2(128, 128));
	breakBlockIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(0, 7), IntVec2(128, 128));
	breakBlockIdleAnimData.m_secondsPerFrame = 0.4f;
	breakBlockIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	breakBlockAnimations.AddAnimation(ANIMATION_STATE_IDLE, breakBlockIdleAnimData);

	EntityAnimStateData breakBlockDeathAnimData = {};
	breakBlockDeathAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	breakBlockDeathAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(1, 7), IntVec2(128, 128));
	breakBlockDeathAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(3, 7), IntVec2(128, 128));
	breakBlockDeathAnimData.m_secondsPerFrame = 0.067f;
	breakBlockDeathAnimData.m_playbackDuration = 0.25f;
	breakBlockDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	breakBlockAnimations.AddAnimation(ANIMATION_STATE_DEATH, breakBlockDeathAnimData);
	breakBlockProps.AddProperty("animationData", breakBlockAnimations);

	breakBlockProps.AddProperty("cosmeticHeight",	BREAK_BLOCK_COSMETIC_HEIGHT);
	breakBlockProps.AddProperty("cosmeticWidth",	BREAK_BLOCK_COSMETIC_WIDTH);
	breakBlockProps.AddProperty("physicsHeight",	BREAK_BLOCK_PHYSICS_HEIGHT);
	breakBlockProps.AddProperty("physicsWidth",		BREAK_BLOCK_PHYSICS_WIDTH);
	
	uint32_t breakBlockOptions = ENTITY_OPTIONS_CAN_BE_STEPPED_ON | ENTITY_OPTIONS_IS_STATIC | ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	breakBlockProps.AddProperty("entityOptions", breakBlockOptions);
	uint32_t breakBlockCollisionLayers = ENTITY_COLLISION_LAYER_BLOCKS | ENTITY_COLLISION_LAYER_PICKUPS | ENTITY_COLLISION_LAYER_ENEMIES;
	breakBlockProps.AddProperty("collisionLayers", breakBlockCollisionLayers);
	AddDefinition(ENTITY_TYPE_BREAKABLE_BLOCK, breakBlockProps);

	//FLIP BLOCK -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties flipBlockProps;
	EntitySpriteInfo flipBlockEditorSpriteInfo = {};
	flipBlockEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	flipBlockEditorSpriteInfo.m_spriteCoords = IntVec2(0, 6);
	flipBlockProps.AddProperty("editorSpriteInfo", flipBlockEditorSpriteInfo);

	EntityAnimationDataContainer flipBlockAnimations;
	EntityAnimStateData flipBlockIdleAnimData = {};
	flipBlockIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	flipBlockIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 6), IntVec2(128, 128));
	flipBlockIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(0, 6), IntVec2(128, 128));
	flipBlockIdleAnimData.m_secondsPerFrame = 0.4f;
	flipBlockIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	flipBlockAnimations.AddAnimation(ANIMATION_STATE_IDLE, flipBlockIdleAnimData);

	EntityAnimStateData flipBlockDeathAnimData = {};
	flipBlockDeathAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	flipBlockDeathAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 6), IntVec2(128, 128));
	flipBlockDeathAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(3, 6), IntVec2(128, 128));
	flipBlockDeathAnimData.m_secondsPerFrame = 0.05f;
	flipBlockDeathAnimData.m_playbackDuration = 2.0f;
	flipBlockDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	flipBlockAnimations.AddAnimation(ANIMATION_STATE_DEATH, flipBlockDeathAnimData);
	flipBlockProps.AddProperty("animationData", flipBlockAnimations);

	flipBlockProps.AddProperty("cosmeticHeight",	FLIP_BLOCK_COSMETIC_HEIGHT);
	flipBlockProps.AddProperty("cosmeticWidth",		FLIP_BLOCK_COSMETIC_WIDTH);
	flipBlockProps.AddProperty("physicsHeight",		FLIP_BLOCK_PHYSICS_HEIGHT);
	flipBlockProps.AddProperty("physicsWidth",		FLIP_BLOCK_PHYSICS_WIDTH);

	uint32_t flipBlockOptions = ENTITY_OPTIONS_CAN_BE_STEPPED_ON | ENTITY_OPTIONS_IS_STATIC | ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	flipBlockProps.AddProperty("entityOptions", flipBlockOptions);
	uint32_t flipBlockCollisionLayers = ENTITY_COLLISION_LAYER_BLOCKS | ENTITY_COLLISION_LAYER_PICKUPS | ENTITY_COLLISION_LAYER_ENEMIES;
	flipBlockProps.AddProperty("collisionLayers", flipBlockCollisionLayers);
	AddDefinition(ENTITY_TYPE_FLIPPABLE_BLOCK, flipBlockProps);

	//TURTLE ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties turtleProps;
	EntitySpriteInfo turtleEditorSpriteInfo = {};
	turtleEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 64);
	turtleEditorSpriteInfo.m_spriteCoords = IntVec2(10, 0);
	turtleProps.AddProperty("editorSpriteInfo", turtleEditorSpriteInfo);

	EntitySpriteInfo turtlePaletteSpriteInfo = {};
	turtlePaletteSpriteInfo.m_spriteSheetLayout = IntVec2(64, 64);
	turtlePaletteSpriteInfo.m_spriteCoords = IntVec2(15, 0);
	turtleProps.AddProperty("paletteSpriteInfo", turtlePaletteSpriteInfo);

	EntityAnimationDataContainer turtleAnimations;
	EntityAnimStateData turtleMovingAnimData = {};
	turtleMovingAnimData.m_secondsPerFrame = 0.12f;
	turtleMovingAnimData.m_spriteSheetLayout = IntVec2(128, 64);
	turtleMovingAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(10, 0), turtleMovingAnimData.m_spriteSheetLayout);
	turtleMovingAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(11, 0), turtleMovingAnimData.m_spriteSheetLayout);
	turtleMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::PING_PONG;
	turtleAnimations.AddAnimation(ANIMATION_STATE_MOVING, turtleMovingAnimData);

	EntityAnimStateData turtleDeathAnimData = {};
	turtleDeathAnimData.m_secondsPerFrame = 0.06f;
	turtleDeathAnimData.m_animStartIndex = 16;
	turtleDeathAnimData.m_animEndIndex = 19;
	turtleDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	turtleDeathAnimData.m_spriteSheetLayout = IntVec2(16, 16);
	turtleDeathAnimData.m_playbackDuration = 0.25f;
	turtleAnimations.AddAnimation(ANIMATION_STATE_DEATH, turtleDeathAnimData);
	turtleProps.AddProperty("animationData", turtleAnimations);

	turtleProps.AddProperty("cosmeticHeight", TURTLE_COSMETIC_HEIGHT);
	turtleProps.AddProperty("cosmeticWidth",  TURTLE_COSMETIC_WIDTH);
	turtleProps.AddProperty("physicsHeight",  TURTLE_PHYSICS_HEIGHT);
	turtleProps.AddProperty("physicsWidth",   TURTLE_PHYSICS_WIDTH);

	uint32_t turtleOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	turtleProps.AddProperty("entityOptions", turtleOptions);
	uint32_t turtleCollisionLayers = ENTITY_COLLISION_LAYER_ENEMIES;
	turtleProps.AddProperty("collisionLayers", turtleCollisionLayers);
	AddDefinition(ENTITY_TYPE_KOOPA_TROOPA, turtleProps);

	//TURTLE SHELL --------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties turtleShellProps;
	EntitySpriteInfo turtleShellEditorSpriteInfo = {};
	turtleShellEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	turtleShellEditorSpriteInfo.m_spriteCoords = IntVec2(6, 2);
	turtleShellProps.AddProperty("editorSpriteInfo", turtleShellEditorSpriteInfo);

	EntityAnimationDataContainer turtleShellAnimations;
	EntityAnimStateData turtleShellMovingAnimData = {};
	turtleShellMovingAnimData.m_secondsPerFrame = 0.12f;
	turtleShellMovingAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	turtleShellMovingAnimData.m_animStartIndex =	GetSpriteIndex(IntVec2(6, 2), turtleShellMovingAnimData.m_spriteSheetLayout);
	turtleShellMovingAnimData.m_animEndIndex =		GetSpriteIndex(IntVec2(9, 2), turtleShellMovingAnimData.m_spriteSheetLayout);
	turtleShellMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	turtleShellAnimations.AddAnimation(ANIMATION_STATE_MOVING, turtleShellMovingAnimData);

	EntityAnimStateData turtleShellIdleAnimData = {};
	turtleShellIdleAnimData.m_secondsPerFrame = 0.06f;
	turtleShellIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(6, 2), turtleShellMovingAnimData.m_spriteSheetLayout);
	turtleShellIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(6, 2), turtleShellMovingAnimData.m_spriteSheetLayout);
	turtleShellIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	turtleShellIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	turtleShellIdleAnimData.m_playbackDuration = 0.25f;
	turtleShellAnimations.AddAnimation(ANIMATION_STATE_IDLE, turtleShellIdleAnimData);
	turtleShellProps.AddProperty("animationData", turtleShellAnimations);

	turtleShellProps.AddProperty("cosmeticHeight",	GREEN_SHELL_COSMETIC_HEIGHT);
	turtleShellProps.AddProperty("cosmeticWidth",	GREEN_SHELL_COSMETIC_WIDTH);
	turtleShellProps.AddProperty("physicsHeight",	GREEN_SHELL_PHYSICS_HEIGHT);
	turtleShellProps.AddProperty("physicsWidth",	GREEN_SHELL_PHYSICS_WIDTH);

	uint32_t turtleShellOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	turtleShellProps.AddProperty("entityOptions", turtleShellOptions);
	uint32_t turtleShellCollisionLayers = ENTITY_COLLISION_LAYER_ENEMIES;
	turtleShellProps.AddProperty("collisionLayers", turtleShellCollisionLayers);
	AddDefinition(ENTITY_TYPE_TURTLE_SHELL, turtleShellProps);

	//RED MUSHROOM ------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties redMushroomProps;
	EntitySpriteInfo redMushroomEditorSpriteInfo = {};
	redMushroomEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	redMushroomEditorSpriteInfo.m_spriteCoords = IntVec2(10, 2);
	redMushroomProps.AddProperty("editorSpriteInfo", redMushroomEditorSpriteInfo);

	EntityAnimationDataContainer redMushroomsAnimations;
	EntityAnimStateData redMushroomMovingAnimData = {};
	redMushroomMovingAnimData.m_secondsPerFrame = 0.12f;
	redMushroomMovingAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	redMushroomMovingAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(10, 2), redMushroomMovingAnimData.m_spriteSheetLayout);
	redMushroomMovingAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(10, 2), redMushroomMovingAnimData.m_spriteSheetLayout);
	redMushroomMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	redMushroomsAnimations.AddAnimation(ANIMATION_STATE_MOVING, redMushroomMovingAnimData);

	EntityAnimStateData redMushroomIdleAnimData = {};
	redMushroomIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	redMushroomIdleAnimData.m_secondsPerFrame = 0.06f;
	redMushroomIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(10, 2), redMushroomIdleAnimData.m_spriteSheetLayout);
	redMushroomIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(10, 2), redMushroomIdleAnimData.m_spriteSheetLayout);
	redMushroomIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	redMushroomIdleAnimData.m_playbackDuration = 0.25f;
	redMushroomsAnimations.AddAnimation(ANIMATION_STATE_IDLE, redMushroomIdleAnimData);
	redMushroomProps.AddProperty("animationData", redMushroomsAnimations);

	redMushroomProps.AddProperty("cosmeticHeight",	RED_MUSHROOM_COSMETIC_HEIGHT);
	redMushroomProps.AddProperty("cosmeticWidth",	RED_MUSHROOM_COSMETIC_WIDTH);
	redMushroomProps.AddProperty("physicsHeight",	RED_MUSHROOM_PHYSICS_HEIGHT);
	redMushroomProps.AddProperty("physicsWidth",	RED_MUSHROOM_PHYSICS_WIDTH);

	uint32_t redMushroomOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	redMushroomProps.AddProperty("entityOptions", redMushroomOptions);
	uint32_t redMushroomCollisionLayers = ENTITY_COLLISION_LAYER_PICKUPS;
	redMushroomProps.AddProperty("collisionLayers", redMushroomCollisionLayers);
	AddDefinition(ENTITY_TYPE_RED_MUSHROOM, redMushroomProps);

	//WAYPOINT ------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties waypointProps;
	EntitySpriteInfo waypointEditorSpriteInfo = {};
	waypointEditorSpriteInfo.m_spriteSheetLayout = IntVec2(128, 128);
	waypointEditorSpriteInfo.m_spriteCoords = IntVec2(7, 3);
	waypointProps.AddProperty("editorSpriteInfo", waypointEditorSpriteInfo);

	EntityAnimationDataContainer waypointAnimations;
	EntityAnimStateData waypointIdleAnimData = {};
	waypointIdleAnimData.m_spriteSheetLayout = IntVec2(128, 128);
	waypointIdleAnimData.m_secondsPerFrame = 0.06f;
	waypointIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(7, 3), waypointIdleAnimData.m_spriteSheetLayout);
	waypointIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(7, 3), waypointIdleAnimData.m_spriteSheetLayout);
	waypointIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	waypointIdleAnimData.m_playbackDuration = 0.25f;
	waypointAnimations.AddAnimation(ANIMATION_STATE_IDLE, waypointIdleAnimData);
	waypointProps.AddProperty("animationData", waypointAnimations);

	waypointProps.AddProperty("cosmeticHeight", WAYPOINT_COSMETIC_HEIGHT);
	waypointProps.AddProperty("cosmeticWidth",	WAYPOINT_COSMETIC_WIDTH);
	waypointProps.AddProperty("physicsHeight",	WAYPOINT_PHYSICS_HEIGHT);
	waypointProps.AddProperty("physicsWidth",	WAYPOINT_PHYSICS_WIDTH);

	AddDefinition(ENTITY_TYPE_WAYPOINT, waypointProps);

	//BOWSER ------------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties bowserProps;
	EntitySpriteInfo bowserEditorSpriteInfo = {};
	bowserEditorSpriteInfo.m_spriteSheetLayout = IntVec2(32, 32);
	bowserEditorSpriteInfo.m_spriteCoords = IntVec2(0, 6);
	bowserProps.AddProperty("editorSpriteInfo", bowserEditorSpriteInfo);

	EntitySpriteInfo bowserPaletteSpriteInfo = {};
	bowserPaletteSpriteInfo.m_spriteSheetLayout = IntVec2(32, 32);
	bowserPaletteSpriteInfo.m_spriteCoords = IntVec2(5, 0);
	bowserProps.AddProperty("paletteSpriteInfo", bowserPaletteSpriteInfo);

	EntityAnimationDataContainer bowserAnimations;
	EntityAnimStateData bowserIdleAnimData = {};
	bowserIdleAnimData.m_spriteSheetLayout = IntVec2(32, 32);
	bowserIdleAnimData.m_secondsPerFrame = 0.3f;
	bowserIdleAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 6), bowserIdleAnimData.m_spriteSheetLayout);
	bowserIdleAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(1, 6), bowserIdleAnimData.m_spriteSheetLayout);
	bowserIdleAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	bowserAnimations.AddAnimation(ANIMATION_STATE_IDLE, bowserIdleAnimData);

	EntityAnimStateData bowserMovingAnimData = {};
	bowserMovingAnimData.m_spriteSheetLayout = IntVec2(32, 32);
	bowserMovingAnimData.m_secondsPerFrame = 0.25f;
	bowserMovingAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 6), bowserMovingAnimData.m_spriteSheetLayout);
	bowserMovingAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(3, 6), bowserMovingAnimData.m_spriteSheetLayout);
	bowserMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	bowserAnimations.AddAnimation(ANIMATION_STATE_MOVING, bowserMovingAnimData);

	EntityAnimStateData bowserDeathAnimData = {};
	bowserDeathAnimData.m_secondsPerFrame = 0.06f;
	bowserDeathAnimData.m_animStartIndex = 16;
	bowserDeathAnimData.m_animEndIndex = 19;
	bowserDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	bowserDeathAnimData.m_spriteSheetLayout = IntVec2(16, 16);
	bowserDeathAnimData.m_playbackDuration = 0.25f;
	bowserDeathAnimData.m_blocksOtherAnimations = true;
	bowserAnimations.AddAnimation(ANIMATION_STATE_DEATH, bowserDeathAnimData);

	EntityAnimStateData bowserAttackAnimData = {};
	bowserAttackAnimData.m_spriteSheetLayout = IntVec2(32, 32);
	bowserAttackAnimData.m_secondsPerFrame = 0.15f;
	bowserAttackAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(5, 6), bowserAttackAnimData.m_spriteSheetLayout);
	bowserAttackAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(7, 6), bowserAttackAnimData.m_spriteSheetLayout);
	bowserAttackAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	bowserAttackAnimData.m_playbackDuration = 0.5f;
	bowserAttackAnimData.m_blocksOtherAnimations = true;
	bowserAnimations.AddAnimation(ANIMATION_STATE_ATTACK, bowserAttackAnimData);
	bowserProps.AddProperty("animationData", bowserAnimations);

	bowserProps.AddProperty("cosmeticHeight",	BOWSER_COSMETIC_HEIGHT);
	bowserProps.AddProperty("cosmeticWidth",	BOWSER_COSMETIC_WIDTH);
	bowserProps.AddProperty("physicsHeight",	BOWSER_PHYSICS_HEIGHT);
	bowserProps.AddProperty("physicsWidth",		BOWSER_PHYSICS_WIDTH);

	uint32_t bowserOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	bowserProps.AddProperty("entityOptions", bowserOptions);
	uint32_t bowserCollisionLayers = ENTITY_COLLISION_LAYER_ENEMIES;
	bowserProps.AddProperty("collisionLayers", bowserCollisionLayers);

	AddDefinition(ENTITY_TYPE_BOWSER, bowserProps);

	//FIREBALL -------------------------------------------------------------------------------------------------------------------------------------------------------
	EntityProperties fireballProps;

	EntityAnimationDataContainer fireballAnimations;
	EntityAnimStateData fireballMovingAnimData = {};
	fireballMovingAnimData.m_spriteSheetLayout = IntVec2(64, 128);
	fireballMovingAnimData.m_secondsPerFrame = 0.06f;
	fireballMovingAnimData.m_animStartIndex = GetSpriteIndex(IntVec2(0, 29), fireballMovingAnimData.m_spriteSheetLayout);
	fireballMovingAnimData.m_animEndIndex = GetSpriteIndex(IntVec2(1, 29), fireballMovingAnimData.m_spriteSheetLayout);
	fireballMovingAnimData.m_playbackMode = SpriteAnimPlaybackMode::LOOP;
	fireballAnimations.AddAnimation(ANIMATION_STATE_MOVING, fireballMovingAnimData);

	EntityAnimStateData fireballDeathAnimData = {};
	fireballDeathAnimData.m_secondsPerFrame = 0.06f;
	fireballDeathAnimData.m_animStartIndex = 16;
	fireballDeathAnimData.m_animEndIndex = 19;
	fireballDeathAnimData.m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	fireballDeathAnimData.m_spriteSheetLayout = IntVec2(16, 16);
	fireballDeathAnimData.m_playbackDuration = 0.25f;
	fireballAnimations.AddAnimation(ANIMATION_STATE_DEATH, fireballDeathAnimData);

	fireballProps.AddProperty("animationData", fireballAnimations);

	fireballProps.AddProperty("cosmeticHeight", FIREBALL_COSMETIC_HEIGHT);
	fireballProps.AddProperty("cosmeticWidth",	FIREBALL_COSMETIC_WIDTH);
	fireballProps.AddProperty("physicsHeight",	FIREBALL_PHYSICS_HEIGHT);
	fireballProps.AddProperty("physicsWidth",	FIREBALL_PHYSICS_WIDTH);

	uint32_t fireballOptions = ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT;
	fireballProps.AddProperty("entityOptions", fireballOptions);
	uint32_t fireballCollisionLayers = ENTITY_COLLISION_LAYER_ENEMIES;
	fireballProps.AddProperty("collisionLayers", fireballCollisionLayers);

	AddDefinition(ENTITY_TYPE_FIREBALL, fireballProps);
}


void EntityDefinition::AddDefinition(EntityType type, EntityProperties const& properties)
{
	EntityDefinition definition = {};
	definition.m_properties = properties;
	s_defaultDefinitions[type] = definition;
}


EntityDefinition const* EntityDefinition::GetEntityDefinitionByType(EntityType type)
{
	if (type <= ENTITY_TYPE_NONE || type >= ENTITY_TYPE_COUNT)
		ERROR_RECOVERABLE("Trying to access definition of an invalid entity type");

	return &s_defaultDefinitions[type];
}


SpriteDefinition const* EntityDefinition::GetEditorSpriteDefinitionForEntity(EntityType type)
{
	EntityDefinition const* definition = GetEntityDefinitionByType(type);
	EntitySpriteInfo editorSpriteInfo = {};
	definition->m_properties.GetProperty("editorSpriteInfo", editorSpriteInfo, editorSpriteInfo);
	return editorSpriteInfo.GetSpriteDefinition();
}


SpriteDefinition const* EntityDefinition::GetPaletteSpriteDefinitionForEntity(EntityType type)
{
	EntityDefinition const* definition = GetEntityDefinitionByType(type);
	EntitySpriteInfo spriteInfo = {};
	definition->m_properties.GetProperty("paletteSpriteInfo", spriteInfo, spriteInfo);
	if (spriteInfo.m_spriteSheetLayout == IntVec2::ZERO)
	{
		definition->m_properties.GetProperty("editorSpriteInfo", spriteInfo, spriteInfo);
	}
	return spriteInfo.GetSpriteDefinition();
}


AABB2 EntityDefinition::GetPhysicsBoundsForEntity(EntityType type, Vec2 const& translation)
{
	EntityDefinition const* definition = GetEntityDefinitionByType(type);
	return definition->GetPhysicsBounds(translation);
}

AABB2 EntityDefinition::GetCosmeticBoundsForEntity(EntityType type, Vec2 const& translation)
{
	EntityDefinition const* definition = GetEntityDefinitionByType(type);
	return definition->GetCosmeticBounds(translation);
}


EntityAnimStateData EntityDefinition::GetAnimation(AnimationState state) const
{
	EntityAnimationDataContainer animationData;
	m_properties.GetProperty("animationData", animationData, animationData);
	return animationData.m_animations[state];
}


void EntityDefinition::AddVertsForEditorSpriteToAABB2(std::vector<Vertex_PCU>& out_verts, AABB2 const& refAABB2, Rgba8 const& tint) const
{
	EntitySpriteInfo editorSpriteInfo = {};
	m_properties.GetProperty("editorSpriteInfo", editorSpriteInfo, editorSpriteInfo);
	Vec2 uvMins = Vec2::ZERO;
	Vec2 uvMaxs = Vec2::ZERO;
	SpriteSheet* spriteSheet = g_game->CreateOrGetSpriteSheet(editorSpriteInfo.m_spriteSheetLayout);
	spriteSheet->GetSpriteUVs(uvMins, uvMaxs, editorSpriteInfo.m_spriteCoords);
	AABB2 finalAABB2 = refAABB2;
	Vec2 aabb2Dims = finalAABB2.GetDimensions();
	Vec2 pivotOffset = aabb2Dims * editorSpriteInfo.m_pivot;
	finalAABB2.Translate(-pivotOffset);
	AddVertsForAABB2ToVector(out_verts, finalAABB2, tint, uvMins, uvMaxs);
}

AABB2 EntityDefinition::GetPhysicsBounds(Vec2 const& translation /*= Vec2::ZERO*/) const
{
	float height = 0.0f;
	float width = 0.0f;
	m_properties.GetProperty("physicsHeight", height, height);
	m_properties.GetProperty("physicsWidth", width, width);
	AABB2 physicsBounds = AABB2(-width * 0.5f, 0.0f, width * 0.5f, height);
	physicsBounds.Translate(translation);
	return physicsBounds;
}

AABB2 EntityDefinition::GetCosmeticBounds(Vec2 const& translation /*= Vec2::ZERO*/) const
{
	float height = 0.0f;
	float width = 0.0f;
	m_properties.GetProperty("cosmeticHeight", height, height);
	m_properties.GetProperty("cosmeticWidth", width, width);
	AABB2 cosmeticBounds = AABB2(-width * 0.5f, 0.0f, width * 0.5f, height);
	cosmeticBounds.Translate(translation);
	return cosmeticBounds;
}
