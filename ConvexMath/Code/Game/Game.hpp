#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/Plane2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RaycastUtils.hpp"

enum class GameState
{
	NONE,
	GAME_STATE_CONVEX_SCENE,
};


enum BSPPartitionScheme : int
{
	BSPPartitionScheme_NAIVE,
	BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_1,
	BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_2,
	BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_3,
	BSPPartitionScheme_COUNT
};

class Entity;
class VertexBuffer;
class BufferParser;
class BufferWriter;

//class representing an instance of our game
//it is owned by App
class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void ShutDown();
	void LoadAssets();
	void HandleQuitRequest();
	void CheckInputDeveloperCheats();
	void Update();
	void UpdateCameras();
	void Render();

	void InitConvexScene();
	void DeInitConvexScene();
	void CheckInputConvexScene();
	void UpdateConvexScene(float deltaSeconds);
	void RenderConvexScene();

	//mutators
	void SetGameState(GameState gameState);
	void SetTimeScale(float timeScale);

	BaseRaycastResult2D RaycastScene(Vec2 const& rayStart, Vec2 const& rayFwdNormal, float maxDistance);
	BaseRaycastResult2D RaycastScene_WithoutBSP(Vec2 const& rayStart, Vec2 const& rayFwdNormal, float maxDistance);
	BaseRaycastResult2D RaycastScene_WithBSP(Vec2 const& rayStart, Vec2 const& rayFwdNormal, float maxDistance);
	BaseRaycastResult2D RaycastVsConvexShape(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, ConvexShape const& convexShape);

	void WriteSceneToFile();
	void ReadSceneFromFile(std::string const& filePath);
	void ParseChunk(BufferParser& parser, uint32_t chunkStartOffset, uint8_t chunkIdToParse, uint32_t totalChunkSize);
	void WriteBSPTreeToBuffer(BufferWriter& writer, uint32_t& out_byteSizeOfTree);
	void WriteBSPNodeToBuffer(BufferWriter& writer, BSPNode* node, uint8_t& nodeIndex, uint32_t& out_byteSizeOfNode);
private:
	void RandomizeScene();
	void UpdateVertexBuffers();
	void RandomizeConvexShape(ConvexShape& out_shape);
	void RandomizePlane(Plane2& plane);
	void RandomlyRaycastScene();
	
	void AddShapesInFrontOfPlaneToVector(std::vector<ConvexShape>& vector, Plane2 const& plane, std::vector<ConvexShape> const& refShapes);
	void AddShapesBehindPlaneToVector(std::vector<ConvexShape>& vector, Plane2 const& plane, std::vector<ConvexShape> const& refShapes);
	void AddShapesIntersectionPlaneToVector(std::vector<ConvexShape>& vector, Plane2 const& plane, std::vector<ConvexShape> const& refShapes);
	
	bool IsShapeInFrontOfPlane(Plane2 const& plane, ConvexShape const& shape);
	bool IsShapeBehindPlane(Plane2 const& plane, ConvexShape const& shape);
	bool IsShapeIntersectinPlane(Plane2 const& plane, ConvexShape const& shape);
	
	void GenerateBSPTree(BSPNode*& node);
	void RenderBSPNode(BSPNode* node, Rgba8 const& partitionColor);
	void AddChildNodesToNode(BSPNode*& bspNode);

	BSPPartitionStruct GetBestPartition(std::vector<BSPPartitionStruct> const& partitionData, BSPNode* bspNode);
	void AddPartitionToNode(BSPNode*& bspNode);

	void CalculateBSPTreeDepth(BSPNode* node, int& out_treeDepth);
	void ClearBSPTree(BSPNode*& node);

	void GoUpTheTreeAndFindPointsOfIntersectionBetweenPartitionPlanes(std::vector<Vec2>& pointsOfIntersection, BSPNode* node);
	bool IsPointOffScreen(Vec2 const& pt1);
	bool ArePlanesVirtuallyEqual(Plane2 const& plane1, Plane2 const& plane2);
	void CreatePartitionInNode(BSPNode*& bspNode);
	bool IsPlaneInBSPTree(Plane2 const& plane);
	bool IsPartitionOutsideNode(BSPPartitionStruct const& partitionInfo, BSPNode* node);
	void DisplayBSPTreeData(BSPNode* bspNode);
	uint32_t FindChunkOffsetForChunkId(BufferParser& parser, uint32_t tocChunkDataOffset, int numChunks, uint8_t refChunkId);
	uint32_t FindChunkSizeForChunkId(BufferParser& parser, uint32_t tocChunkDataOffset, int numChunks, uint8_t refChunkId);
private: //methods
	//test
// 	Plane2 m_p2;
// 	Plane2 m_p1;

	std::vector<Vec2> m_pointsOfIntersection;
	BSPNode* m_testNode = nullptr;

	//bsp
	BSPTree m_bspTree;
	int m_maxBSPDepth = 20;
	bool m_renderBSP = false;
	bool m_showBSPWindow = false;
	bool m_raycastUsingBSP = false;
	BSPPartitionScheme m_partitionScheme = BSPPartitionScheme_SCORE_BASED_SCORING_SCHEME_3;

	//raycast variables
	int m_numInvisibleRaycasts = 0;
	bool m_checkAgainstBoundingDisks = false;
	bool m_raycastScene = false;
	BaseRaycastResult2D m_raycastResult = {};
	
	//input variables
	bool m_isInputActive = false;
	bool m_isInputLocked = false;
	Vec2 m_inputStart;
	Vec2 m_inputEnd;
	
	//shapes
	std::vector<ConvexShape> m_convexShapes;
	int m_numShapes = 0;
	bool m_renderShapePlanes = false;
	bool m_renderShapesAsBlob = false;
	bool m_renderBoundingDisks = false;
	float m_worldLineWidth = 2.0f;
	VertexBuffer* m_shapesWithOutlineVBO = nullptr;
	VertexBuffer* m_shapesWithoutOutlineVBO = nullptr;
	VertexBuffer* m_shapeOutlineVBO = nullptr;
	int m_numVertsWithOutline = 0;
	int m_numVertsWithoutOutline = 0;
	int m_numVertsForOutline = 0;
	
	//time
	float m_raycastingTimeSeconds = 0.0f;
	float m_renderingShapesTimeSeconds = 0.0f;
	float m_bspTreeGenerationTimeSeconds = 0.0f;
	
	//controls
	bool m_displaySceneStats = true;
	bool m_displayControls = false;
	
	//cameras
	Vec2 m_worldOrthoBottomLeft;
	Vec2 m_worldOrthoTopRight;
	Camera m_worldCamera;
	Camera m_uiCamera;
	
	GameState m_gameState = GameState::NONE;
	Clock m_gameClock;
};
