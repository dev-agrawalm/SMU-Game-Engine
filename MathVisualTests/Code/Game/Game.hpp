#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include<vector>
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/Shapes3D.hpp"
#include "Game/GameModeBilliards2D.hpp"
#include "Game/GameModePachinko2D.hpp"

struct Vertex_PCU;

enum CameraMode
{
	CAMERA_MODE_ORTHOGRAPHIC,
	CAMERA_MODE_PERSPECTIVE,
	NUM_CAMERA_MODES
};


enum GameMode
{
	GAME_MODE_NEAREST_POINT_2D = 0,
	GAME_MODE_RAYCASTS_Disk2D,
	GAME_MODE_RAYCASTS_AABB2,
	GAME_MODE_RAYCASTS_LINESEGMENT2,
	GAME_MODE_BILLIARDS_2D,
	GAME_MODE_PACHINKO_2D,
	GAME_MODE_3D, //all 2d modes should be before this and all 3d mode should be ahead of this
	NUM_GAME_MODES
};


constexpr int numDisksRaycast2D = 12;
constexpr int numAABB2Raycast2D = 12;
constexpr int numLineSegment2Raycast2D = 12;

//it is owned by App
class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void ShutDown();
	void HandleQuitRequest();
	void CheckDebugCheatsInput();
	void CheckInput(float deltaSeconds);
	void Update();
	void Render() const;

	//Randomize 2d shapes
	void RandomizeLineSegment2D(LineSegment2& out_lineSegment);
	void RandomizeOBB2D(OBB2& out_obb2);
	void RandomizeDisk2D(Vec2& out_diskCenter, float& out_diskRadius);
	void RandomizeDisk2D(Vec2& out_diskCenter, float& out_diskRadius, FloatRange const& radiusRange, AABB2 const& centerBounds);
	void RandomizeAABB2D(AABB2& out_aabb2);
	void RandomizeCapsule2D(Capsule2& out_capsule);

	//nearest point 2d functions
	void RandomizeShapesForNearestPoint2D();
	void UpdateVertexVectorNearestPoint2D();

	//raycast v disk2d
	void RandomizeDisksForRaycast2D();
	void RaycastAllDisks2D();
	void UpdateVertexVectorRaycastVDisk2D();

	//raycast v aabb2
	void RandomizeAABB2sForRaycast();
	void RaycastAllAABB2s();
	void UpdateVertexVectorRaycastVAABB2();

	//raycast v line segment 2
	void RandomizeLineSegment2sForRaycast();
	void RaycastAllLineSegment2s();
	void UpdateVertexVectorRaycastVLineSegment2();

	//3D functions
	void RaycastAll3DShapes();
	void UpdateNearestPointVerts3D();
	void RandomizeShapes3D();
	void ParkReferencePointNearestPoint3D();
	void UnParkReferencePointNearestPoint3D();
	void CheckAndUpdateVertAlphasBasedOnOverlaps();
	bool Do3DShapesOverlap(Shape3D* shapeA, Shape3D* shapeB);

	void DisplayGameModeAndControls(GameMode mode);

	//mutators
	void SetTimeScale(float timeScale);
	void SetCameraMode(CameraMode newMode);
	void ToggleCameraMode();
	void SetGameMode(GameMode newMode);
	void CycleGameMode(bool goForward = true);
	void SetOrthoZoomSpeed(float zoomSpeed);

	Clock const& GetClock() const;
private:
	void UpdateCamera();

private: //methods
	GameMode m_gameMode = GameMode::GAME_MODE_NEAREST_POINT_2D;
	CameraMode m_camMode = CameraMode::CAMERA_MODE_ORTHOGRAPHIC;
	Camera	m_renderCamera;
	Clock	m_clock;
	std::vector<Vertex_PCU> m_verts;

	//ortho cam variables
	float m_orthoZoomSpeed = 0.0f;
	Vec2 m_orthoBottomLeft;
	Vec2 m_orthoTopRight;

	//nearest point 2d variables
	Vec2			m_referencePoint		= Vec2::ZERO;
	float			m_referencePointRadius	= 0.0f;
	float			m_referencePointSpeed	= 0.0f;
	Rgba8			m_referencePointColor	= Rgba8::WHITE;
	Vec2			m_disk2dCenter = Vec2::ZERO;
	float			m_disk2dRadius = 0.0f;
	AABB2			m_aabb2;
	Capsule2		m_capsule2d;
	OBB2			m_obb2;
	LineSegment2	m_finiteLine2d;
	LineSegment2	m_infiniteLine2d;
	Rgba8			m_nearestPointColor;
	Rgba8			m_shapeColor;
	Rgba8			m_highlightedShapeColor;

	//raycast 2d variables
	Vec2 m_raycast2DStart;
	Vec2 m_raycast2DEnd;
	Vec2 m_impactNormal2D;
	Vec2 m_impactPoint2D;
	
	// raycast v disk 2d variables
	Disk2D m_disks[numDisksRaycast2D];
	Disk2D m_impactDisk;

	//raycast v aabb2 variables
	AABB2 m_aabb2s[numAABB2Raycast2D];
	AABB2 m_impactAABB2;

	//raycast v line segment 2 variables
	LineSegment2 m_lineSegment2s[numLineSegment2Raycast2D];
	LineSegment2 m_impactLineSegment;

	// raycast 3d variables
	Vec3 m_raycast3DStart;
	Vec3 m_raycast3DFwdNormal;
	float m_raycast3DLength = 0.0f;
	Vec3 m_impactNormal3D;
	Vec3 m_impactPoint3D;
	Shape3D* m_impactShape3D = nullptr;

	//billiards
	GameModeBilliards2D m_billiards2D;
	GameModePachinko2D m_pachinko2D;

	//shapes 3d variables
	std::vector<Vertex_PCU> m_solid3dShapesVerts;
	std::vector<Vertex_PCU> m_wire3dShapesVerts;
	std::vector<Shape3D*> m_3dShapes;
	Vec3 m_referencePoint3D;
	Shape3D* m_pickedUpObject = nullptr;
	bool m_isRefPointParked = false;
	bool m_isObjectPickedUp = false;

	//perspective cam variables
	float m_camMovementSpeed = 0.0f;
	float m_camRotationSpeed = 0.0f;
	Vec3 m_persCamPosition;
	EulerAngles m_persCamAngles;
	float m_fov = 0.0f;
	float m_zNear = 0.0f;
	float m_zFar = 0.0f;
};
