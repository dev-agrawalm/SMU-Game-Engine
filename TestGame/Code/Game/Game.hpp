#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include <vector>

enum class GameMode
{
	NONE,
	ATTRACT,
	PLAY
};

constexpr int NUM_CAMERAS = 5;

struct CameraData
{
public:
	float m_camWidth = 0.0f;
	float m_camHeight = 0.0f;
	float m_camFov = 60.0f;
	bool m_isFreeAspect = false;
	Vec3 m_camPosition = Vec3(0.0f, 0.0f, 3.0f);
	EulerAngles m_camOrientation;
};

struct Shape3D
{
public:
	bool m_isCylinder = false;
	bool m_isSphere = false;
	AABB3 m_cube;
	Vec3 m_sphereCenter;
	Vec3 m_zCylinderTop;
	Vec3 m_zCylinderBottom;
	float m_radius = 0.0f;
};

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
	void Update(float deltaSeconds);
	void UpdateCameras(float deltaSeconds);
	void Render() const;

	void InitAttractMode();
	void DeInitAttractMode();
	void CheckInputAttractMode();
	void UpdateAttractMode(float deltaSeconds);
	void RenderAttractMode() const;

	void RenderAttractModeDebugger() const;
	void InitPlayMode();
	void DeInitPlayMode();
	void CheckInputPlayMode(float deltaSeconds);
	void UpdatePlayMode(float deltaSeconds);
	void DeleteGarbage();
	void RenderPlayMode() const;
	void RenderPlayModeUI() const;
	void RenderPlayModeDebugger() const;
	void RenderGrid() const;

	//mutators
	void SetGameMode(GameMode gameMode);
	void SetWorldCamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration);
	void SetUICamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration);
	void SetTimeScale(float timeScale);

private:
	void SpriteAnimationTest(float uiCanvasSizeX, float uiCanvasSizeY) const;
	void BitmapFontTest(float uiCanvasSizeX, float uiCanvasSizeY) const;
	void GenerateRandomShapes();

private: //methods
	std::vector<Shape3D> m_shapes3D;
	int m_numShapes = 0;
	AABB3 m_shapeBounds;
	float m_camSpeed = 0.0f;
	float m_camSprintSpeedModifier = 0.0f;
	CameraData m_camData[NUM_CAMERAS] = {};
	int m_activeCamIndex = 0;

	//Testing variables
	float m_animationTime = 0.0f;

	//World Cam variables
	Camera m_worldCamera;
	bool m_shouldShakeWorldCam = false;
	float m_worldCamScreenShakeMagnitude = 0.0f;
	float m_worldCamScreenShakeDuration = 0.0f;
	float m_worldCamTimeSinceScreenShakeStart = 0.0f;
	
	//UI Cam variables
	Camera m_uiCamera;
	bool m_shouldShakeUICam = false;
	float m_uiCamScreenShakeMagnitude = 0.0f;
	float m_uiCamScreenShakeDuration = 0.0f;
	float m_uiCamTimeSinceScreenShakeStart = 0.0f;
	
	bool m_debugMode = false;
	bool m_isPaused = false;
	bool m_runSingleStep = false;

	GameMode m_gameMode = GameMode::NONE;
	float m_timeScale = 1.0f;
};