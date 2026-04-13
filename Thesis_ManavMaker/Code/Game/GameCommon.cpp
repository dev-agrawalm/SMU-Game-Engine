#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Game/Level.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/Entity.hpp"
#include "Game/Tile.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/AABB2.hpp"

float g_editorMusicVolume = EDITOR_MUSIC_VOLUME;
float g_gameMusicVolume = GAME_MUSIC_VOLUME;

void DebugDrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color)
{
	const int NUM_VERTS_LINE = 6;
	Vertex_PCU lineVerts[NUM_VERTS_LINE];

	float originX = 0.0f;
	float originY = 0.0f;
	float halfWidth = width / 2.0f;
	float lenght = line.GetLength();

	for (int index = 0; index < NUM_VERTS_LINE; index++)
	{
		lineVerts[index].m_color = color;
	}

	//lower triangle
	lineVerts[0].m_position = Vec3(originX, originY - halfWidth);
	lineVerts[1].m_position = Vec3(originX + lenght, originY - halfWidth);
	lineVerts[2].m_position = Vec3(originX + lenght, originY + halfWidth);
	//upper triangle
	lineVerts[3].m_position = Vec3(originX + lenght, originY + halfWidth);
	lineVerts[4].m_position = Vec3(originX, originY + halfWidth);
	lineVerts[5].m_position = Vec3(originX, originY - halfWidth);

	Vec2 iBasis = line.GetNormalized();
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	TransformVertexArrayXY3D(NUM_VERTS_LINE, lineVerts, iBasis, jBasis, startPos);

	g_theRenderer->SetDiffuseTexture(nullptr);
	g_theRenderer->DrawVertexArray(NUM_VERTS_LINE, lineVerts);
}


void DebugDrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color)
{
	const int NUM_QUADS_RING = 32;
	const int NUM_VERTS_QUAD = 6;
	const int TOTAL_VERT_COUNT = NUM_QUADS_RING * NUM_VERTS_QUAD; //total vertexes = number of quads in a ring * number of vertexes in a single quad
	Vertex_PCU* localVertexes = new Vertex_PCU[TOTAL_VERT_COUNT];

	float thetaInterval = (360.0f / static_cast<float>(NUM_QUADS_RING)); //the angle at the center of the circle for each individual quad segment of the ring
	
	float startThetaDegrees = 0.0f; //angle of the starting line of our quad segment
	float endThetaDegrees = thetaInterval; //angle of the ending line of our quad segment
	float outerCircleRadius = radius + width;
	float innerCircleRadius = radius;
	int initVertexCount = 0; //number of vertexes in the vertex array that have been initialized

	//Outer Point 2 ....................... Outer Point 1      //Points on the outer circle of the ring
	//              .                     .
	//              .                     .                                //Diagram for a single quad in our ring (an approximation for a segment of the ring)
	//              .                     .
	//Inner Point 2 ....................... Inner Point 1      //Points on the inner circle of the ring
	
	while (initVertexCount < TOTAL_VERT_COUNT)                                 
	{						
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		Vec2 outerPoint2 = Vec2(outerCircleRadius * cosEndTheta, outerCircleRadius * sinEndTheta);
		Vec2 outerPoint1 = Vec2(outerCircleRadius * cosStartTheta, outerCircleRadius * sinStartTheta);
		Vec2 innerPoint2 = Vec2(innerCircleRadius * cosEndTheta, innerCircleRadius * sinEndTheta);
		Vec2 innerPoint1 = Vec2(innerCircleRadius * cosStartTheta, innerCircleRadius * sinStartTheta);

		for (int index = initVertexCount; index < initVertexCount + NUM_VERTS_QUAD; index++)
		{
			localVertexes[index].m_color = color;
		}

		//lower triangle
		localVertexes[initVertexCount].m_position = Vec2(centerPosition + innerPoint2).GetVec3();
		localVertexes[initVertexCount + 1].m_position = Vec2(centerPosition + innerPoint1).GetVec3();
		localVertexes[initVertexCount + 2].m_position = Vec2(centerPosition + outerPoint1).GetVec3();
		//upper triangle
		localVertexes[initVertexCount + 3].m_position = Vec2(centerPosition + outerPoint1).GetVec3();
		localVertexes[initVertexCount + 4].m_position = Vec2(centerPosition + outerPoint2).GetVec3();
		localVertexes[initVertexCount + 5].m_position = Vec2(centerPosition + innerPoint2).GetVec3();

		initVertexCount += NUM_VERTS_QUAD;
		startThetaDegrees += thetaInterval;							 
		endThetaDegrees += thetaInterval;                             
	}

	g_theRenderer->SetDiffuseTexture(nullptr);
	g_theRenderer->DrawVertexArray(TOTAL_VERT_COUNT, localVertexes);

	delete[] localVertexes;
	localVertexes = nullptr;
}


void DebugDrawRaycast(LevelRaycastResult const& raycast)
{
	std::vector<Vertex_PCU> raycastVerts;

	if (raycast.m_didHit)
	{
		AddVertsForLineSegment2DToVector(raycastVerts, LineSegment2(raycast.m_startPos, raycast.m_impactPoint), Rgba8::GREEN, 0.1f, 0.0f);
		AddVertsForArrow2DToVector(raycastVerts, raycast.m_impactPoint, raycast.m_startPos + raycast.m_forwardNormal * raycast.m_maxDistance
								   , Rgba8::RED, 0.1f, 0.1f, 0.0f);
		AddVertsForArrow2DToVector(raycastVerts, raycast.m_impactPoint, raycast.m_impactPoint + raycast.m_impactNormal * 0.5f, Rgba8::CYAN, 0.1f, 0.1f, 0.0f);
	}
	else
	{
		AddVertsForArrow2DToVector(raycastVerts, raycast.m_startPos, raycast.m_startPos + raycast.m_forwardNormal * raycast.m_maxDistance
								   , Rgba8::GREEN, 0.1f, 0.1f, 0.0f);
	}

	g_theRenderer->DrawVertexArray((int) raycastVerts.size(), raycastVerts.data());
}


void DebugDrawAABB2Outline(AABB2 const& aabb2, float width, Rgba8 const& color)
{
	Vec2 mins = aabb2.m_mins;
	Vec2 maxs = aabb2.m_maxs;

	Vec2 bottomLine	= Vec2(maxs.x, mins.y) - mins;
	Vec2 topLine	= Vec2(mins.x, maxs.y) - maxs;
	Vec2 rightLine	= Vec2(maxs.x, mins.y) - maxs;
	Vec2 leftLine	= Vec2(mins.x, maxs.y) - mins;

	DebugDrawLine(maxs, topLine,	width, color);
	DebugDrawLine(mins, bottomLine, width, color);
	DebugDrawLine(maxs, rightLine,	width, color);
	DebugDrawLine(mins, leftLine,	width, color);
}


// AABB2 GetPhysicsBoundsForEntity(EntityType entityType, Vec2 const& entityPos)
// {
// 	float height = 0.0f;
// 	float width = 0.0f;
// 	switch (entityType)
// 	{
// 		case ENTITY_TYPE_MARIO:
// 		{
// 			height = PLAYER_BIG_PHYSICS_HEIGHT;
// 			width = PLAYER_BIG_PHYSICS_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_GOOMBA:
// 		{
// 			height = GOOMBA_PHYSICS_HEIGHT;
// 			width = GOOMBA_PHYSICS_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_COIN:
// 		{
// 			height = COIN_PHYSICS_HEIGHT;
// 			width = COIN_PHYSICS_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_LEVEL_END:
// 		{
// 			height = FLAG_PHYSICS_HEIGHT;
// 			width = FLAG_PHYSICS_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_NONE:
// 		case ENTITY_TYPE_COUNT:
// 		default:
// 			break;
// 	}
// 
// 	AABB2 physicsBounds = AABB2(-width * 0.5f, 0.0f, width * 0.5f, height);
// 	physicsBounds.Translate(entityPos);
// 	return physicsBounds;
// }
// 
// 
// AABB2 GetCosmeticBoundsForEntity(EntityType entityType, Vec2 const& entityPos)
// {
// 	float height = 0.0f;
// 	float width = 0.0f;
// 	switch (entityType)
// 	{
// 		case ENTITY_TYPE_MARIO:
// 		{
// 			height = PLAYER_BIG_COSMETIC_HEIGHT;
// 			width = PLAYER_BIG_COSMETIC_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_GOOMBA:
// 		{
// 			height = GOOMBA_COSMETIC_HEIGHT;
// 			width = GOOMBA_COSMETIC_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_COIN:
// 		{
// 			height = COIN_COSMETIC_HEIGHT;
// 			width = COIN_COSMETIC_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_LEVEL_END:
// 		{
// 			height = FLAG_COSMETIC_HEIGHT;
// 			width = FLAG_COSMETIC_WIDTH;
// 			break;
// 		}
// 		case ENTITY_TYPE_NONE:
// 		case ENTITY_TYPE_COUNT:
// 		default:
// 			break;
// 	}
// 
// 	AABB2 cosmeticBounds = AABB2(-width * 0.5f, 0.0f, width * 0.5f, height);
// 	cosmeticBounds.Translate(entityPos);
// 	return cosmeticBounds;
// }


//SpriteSheet* GetSpriteSheetForEntity(EntityType entityType)
//{
//	switch (entityType)
//	{
//		case ENTITY_TYPE_MARIO:
//			return g_spriteSheet_128x64;
//		case ENTITY_TYPE_GOOMBA:
//			return g_spriteSheet_128x128;
//		case ENTITY_TYPE_COIN:
//			return g_spriteSheet_32x32;
//		case ENTITY_TYPE_LEVEL_END:
//			return g_spriteSheet_32x32;
//		case ENTITY_TYPE_NONE:	//fallthrough
//		case ENTITY_TYPE_COUNT:	//fallthrough
//		default:
//			return nullptr;
//	}
//}
//
//
//IntVec2 GetEditorSpriteCoordsForEntity(EntityType entityType)
//{
//	switch (entityType)
//	{
//		case ENTITY_TYPE_MARIO:
//			return IntVec2(4, 0);
//		case ENTITY_TYPE_GOOMBA:
//			return IntVec2(4, 2);
//		case ENTITY_TYPE_COIN:
//			return IntVec2(1, 1);
//		case ENTITY_TYPE_LEVEL_END:
//			return IntVec2(0, 5);
//		case ENTITY_TYPE_NONE:	//fallthrough
//		case ENTITY_TYPE_COUNT:	//fallthrough
//		default:
//			return IntVec2::ZERO;
//	}
//}
//
//
//SpriteDefinition const* GetSpriteDefinitionForEntity(EntityType entityType)
//{
//	SpriteSheet* spriteSheet = GetSpriteSheetForEntity(entityType);
//	IntVec2 spriteCoords = GetEditorSpriteCoordsForEntity(entityType);
//	SpriteDefinition const& buttonSprite = spriteSheet->GetSpriteDefinition(spriteCoords);
//	return &buttonSprite;
//}


int GetSpriteIndex(IntVec2 const& spriteCoords, IntVec2 const& spriteSheetGridLayout)
{
	return spriteCoords.x + spriteCoords.y * spriteSheetGridLayout.x;
}


AABB2CollisionData GetCollisionDataForAABB2s(AABB2 const& refAABB2, AABB2 const& aabb2ToCheck)
{
	AABB2CollisionData result = {};
	result.m_aabb2ToCheck = aabb2ToCheck;
	result.m_refAABB2 = refAABB2;
	result.m_isTouching = AreAABB2sTouching(refAABB2, aabb2ToCheck);
	result.m_isOverlapping = DoAABB2sOverlap(refAABB2, aabb2ToCheck);

	float rightOverlap = refAABB2.m_maxs.x - aabb2ToCheck.m_mins.x;
	float leftOverlap = refAABB2.m_mins.x - aabb2ToCheck.m_maxs.x;
	float bottomOverlap = refAABB2.m_mins.y - aabb2ToCheck.m_maxs.y;
	float topOverlap = refAABB2.m_maxs.y - aabb2ToCheck.m_mins.y;
	float smallestOverlap = GetAbsoluteMin(GetAbsoluteMin(leftOverlap, rightOverlap), GetAbsoluteMin(topOverlap, bottomOverlap));
	if (smallestOverlap == rightOverlap)
		result.m_shortestOverlappingSide = AABB2_RIGHT_SIDE;
	if (smallestOverlap == leftOverlap)
		result.m_shortestOverlappingSide = AABB2_LEFT_SIDE;
	if (smallestOverlap == topOverlap)
		result.m_shortestOverlappingSide = AABB2_TOP_SIDE;
	if (smallestOverlap == bottomOverlap)
		result.m_shortestOverlappingSide = AABB2_BOTTOM_SIDE;
	result.m_shortestOverlappingDisplacement = smallestOverlap;
	return result;
}


bool AreAABB2sTouching(AABB2 const& a, AABB2 const& b)
{
	bool touchingOnX = ((a.m_mins.x <= b.m_maxs.x && a.m_mins.x >= b.m_mins.x) || (a.m_maxs.x >= b.m_mins.x && a.m_maxs.x <= b.m_maxs.x))
		|| ((b.m_mins.x <= a.m_maxs.x && b.m_mins.x >= a.m_mins.x) || (b.m_maxs.x >= a.m_mins.x && b.m_maxs.x <= a.m_maxs.x));
	bool touchingOnY = ((a.m_mins.y <= b.m_maxs.y && a.m_mins.y >= b.m_mins.y) || (a.m_maxs.y >= b.m_mins.y && a.m_maxs.y <= b.m_maxs.y))
		|| ((b.m_mins.y <= a.m_maxs.y && b.m_mins.y >= a.m_mins.y) || (b.m_maxs.y >= a.m_mins.y && b.m_maxs.y <= a.m_maxs.y));
	return touchingOnX && touchingOnY;
}


bool operator==(Tile const& a, Tile const& b)
{
	TileDefinition const* tileDefA = a.m_definition;
	TileDefinition const* tileDefB = b.m_definition;

	return _stricmp(tileDefA->m_name.c_str(), tileDefB->m_name.c_str()) == 0;
}


bool operator==(EntitySpawnInfo const& a, EntitySpawnInfo const& b)
{
	return (a.m_type == b.m_type) && (a.m_startingPosition == b.m_startingPosition)/* && (a.m_entityId == b.m_entityId)*/;
}

EntityAnimationDataContainer::EntityAnimationDataContainer(EntityAnimationDataContainer const& copyContainer)
{
	for (int animState = ANIMATION_STATE_IDLE; animState < ANIMATION_STATE_COUNT; animState++)
	{
		m_animations[animState] = copyContainer.m_animations[animState];
	}
}

void EntityAnimationDataContainer::AddAnimation(AnimationState state, EntityAnimStateData animation)
{
	m_animations[state] = animation;
}

SpriteDefinition const* EntitySpriteInfo::GetSpriteDefinition()
{
	SpriteSheet* spriteSheet = g_game->CreateOrGetSpriteSheet(m_spriteSheetLayout);
	SpriteDefinition const& spriteDef = spriteSheet->GetSpriteDefinition(m_spriteCoords);
	return &spriteDef;
}


SpriteAnimDefinition EntityAnimStateData::GetAsSpriteAnimDefinition() const
{
	SpriteSheet* animationSpriteSheet = g_game->CreateOrGetSpriteSheet(m_spriteSheetLayout);
	int frameCount = m_animEndIndex - m_animStartIndex + 1;
	float animDuration = frameCount * m_secondsPerFrame;
	SpriteAnimDefinition animation = SpriteAnimDefinition(*animationSpriteSheet, m_animStartIndex, m_animEndIndex, animDuration, m_playbackMode);
	return animation;
}


AABB2CollisionData AABB2CollisionData::GetCollisionDataWithInvertedReference() const
{
	AABB2CollisionData invertedData = {};
	invertedData.m_refAABB2 = m_aabb2ToCheck;
	invertedData.m_aabb2ToCheck = m_refAABB2;
	invertedData.m_isTouching = m_isTouching;
	invertedData.m_isOverlapping = m_isOverlapping;
	invertedData.m_shortestOverlappingDisplacement = -m_shortestOverlappingDisplacement;
	int overlappingSideIndex = (int) m_shortestOverlappingSide;
	invertedData.m_shortestOverlappingSide = (AABB2Side) (-overlappingSideIndex);
	return invertedData;
}
