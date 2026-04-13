#pragma once
#include "Engine/Math/Vec2.hpp"

struct QuadraticBezierCurve2D
{
public:
	QuadraticBezierCurve2D();
	QuadraticBezierCurve2D(Vec2 const& start, Vec2 const& guide, Vec2 const& end);
	
	void SetMiddlePoint(Vec2 const& middlePoint);
	
	Vec2 GetPointAtFractionOfCurve(float fraction) const;
	Vec2 GetMiddlePoint() const;
public:
	Vec2 m_start;
	Vec2 m_guide;
	Vec2 m_end;
};


struct CubicHermiteCurve2D;

struct CubicBezierCurve2D
{
public:
	CubicBezierCurve2D();
	CubicBezierCurve2D(Vec2 const& start, Vec2 const& startGuide, Vec2 const& endGuide, Vec2 const& end);

	Vec2 GetPointAtFraction(float zeroToOne) const;
	CubicHermiteCurve2D GetCubicHermiteCurve() const;

	bool operator==(CubicBezierCurve2D const& compare);
public:
	Vec2 m_start;
	Vec2 m_startGuide;
	Vec2 m_endGuide;
	Vec2 m_end;
};


struct CubicHermiteCurve2D
{
public:
	Vec2 GetPointAtFraction(float zeroToOne) const;
	CubicBezierCurve2D GetCubicBezierCurve() const;
	Vec2 GetStartVelocityEndPoint() const;
	Vec2 GetEndVelocityEndPoint() const;
public:
	Vec2 m_start;
	Vec2 m_end;
	Vec2 m_startVelocity;
	Vec2 m_endVelocity;
};
