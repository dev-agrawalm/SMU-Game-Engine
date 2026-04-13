#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/BezierCurve2D.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Game/Spline.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB2.hpp"

class Camera;

enum class ControlPointType { START, START_VELOCITY, END_VELOCITY, END };

struct SplineControlPoint
{
public:
	static const SplineControlPoint INVALID;
public:
	int m_index = -1;
	int m_curveIndexInSpline = -1;
	Vec2 m_position;
	ControlPointType m_controlPointType;
};


class SplineEditor
{
private:
	static bool HandleCharDownEvent(EventArgs& args);

	static std::string s_splineToSaveName;
	static int s_caratPos;
	static bool s_isSaving;

public:
	SplineEditor();
	void Startup();
	void Shutdown();
	void Update(float deltaSeconds);
	void CheckInput();
	void Render(Camera const& camera) const;

	bool SaveSplineToFile(std::string curveName);

	void SaveTextBoxExit();

	void AddCurveAtEndOfSpline();
	void DeleteCurveAtEndOfSpline();

	int GetIndexForClosestControlPoint(Vec2 const& refPos, float thresholdDistace = 5.0f);
	bool IsSaving() const;

private:
	Stopwatch m_pointOnSplineTimer;

	std::string m_splineNameToSave;
	bool m_showCarat = true;
	Stopwatch m_caratStopwatch;

	float m_uiScreenX = 0.0f;
	float m_uiScreenY = 0.0f;


	Vec2 m_pointOnSpline;
	Spline m_spline;
	bool m_showControlPoints = true;

	AABB2 m_uiWorldSpace;

	std::vector<SplineControlPoint> m_splineControlPoints;
	SplineControlPoint m_activeControlPoint = SplineControlPoint::INVALID;
};