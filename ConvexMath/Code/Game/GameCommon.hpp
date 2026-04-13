#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/ConvexPoly2.hpp"
#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Plane2.hpp"
#include <vector>

class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;
class RandomNumberGenerator;

//struct Plane2;
struct BaseRaycastResult2D;

//GLOBAL VARIABLES
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern Window* g_window;
extern RandomNumberGenerator* g_rng;
extern Game* g_game;

//DEBUG RENDERING
void DrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color);
void DrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color);
void DrawArrow(Vec2 const& arrowStart, Vec2 const& arrowEnd, Rgba8 const& color, float width = 1.0f, float arrowTipLength = 1.0f);

void AddVertsForConvexPoly2BorderToVector(std::vector<Vertex_PCU>& vertVector, ConvexPoly2 const& convexPoly2, Rgba8 const& color, float borderWidth = 0.5f);
void DrawPlane2(Plane2 const& plane, float width, Rgba8 const& color, Rgba8 const& fwdNormalColor = Rgba8::WHITE, bool drawFwdNormal = false);
void DrawConvexPoly2(ConvexPoly2 const& convexPoly, Rgba8 const& color, Rgba8 const& borderColor = Rgba8::BLACK, float borderWidth = 0.5f);
void DrawRaycast(BaseRaycastResult2D const& raycastResult, float raycastLineWidth, Rgba8 const& preImpactColor = Rgba8::GREEN, Rgba8 const& postImpactColor = Rgba8::RED, Rgba8 const& impactNormalColor = Rgba8::BLUE);

constexpr int NUM_VERTS_LINE = 6;
constexpr int NUM_VERTS_QUAD = 6;
constexpr int NUM_TRIANGLES_QUAD = 2;
constexpr int NUM_QUADS_RING = 32;
constexpr float DEBUG_LINE_WIDTH = 0.2f;
constexpr float BORDER_LINE_WIDTH = 1.0f;
constexpr float ARROW_LENGTH = 50.0f;
constexpr float ARROW_TIP_LENGTH = 20.0f;

struct ConvexShape
{
public:
	ConvexHull2 m_convexHull;
	ConvexPoly2 m_convexPoly;
	float m_boundindDiskRadius = 0.0f;
	Vec2 m_boundingDiskCenter;
	int m_index = 0;
};

struct BSPPartitionStruct
{
public:
	//for debug drawing purposes
	ConvexShape m_refShape;
	Plane2 m_partitionPlane;

	//for scoring/plane selection purposes
	int m_numShapesInFront = 0;
	int m_numShapesBehind = 0;
	int m_numIntersectingShapes = 0;
	int m_partitionScore = 0;
};

struct BSPNode
{
public:
	BSPNode* m_parentNode = nullptr;
	BSPNode* m_backNode = nullptr;
	BSPNode* m_frontNode = nullptr;

	BSPPartitionStruct m_partitionInfo = {};
	
	int m_numShapesInsideNode = 0;
	std::vector<ConvexShape> m_shapesInNode;
	
	int m_nodeDepth = 0;
	bool m_isFrontNode = false;
	bool m_noMorePartitionsPossible = false;
};


struct BSPTree
{
public:
	BSPNode* GetNodeForPoint(Vec2 const& refPt);

public:
	BSPNode* m_rootNode = nullptr;
	std::vector<Plane2> m_partitionPlanes;
	int m_treeDepth = 0;
	int m_numNodes = 0;
	int m_numLeafNodes = 0;
};


struct BSPNodeFileDataStruct
{
	uint8_t m_parentNodeIndex;
	uint8_t m_frontNodeIndex;
	uint8_t m_backNodeIndex;
	Plane2 m_partitioningPlane;
	uint32_t m_numShapesInNode;
	std::vector<int> m_shapeIndexes;
};