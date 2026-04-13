#pragma once
#include "Engine/Math/BezierCurve2D.hpp"
#include "Engine/Math/MathUtils.hpp"


QuadraticBezierCurve2D::QuadraticBezierCurve2D(Vec2 const& start, Vec2 const& guide, Vec2 const& end)
	: m_start(start)
	, m_guide(guide)
	, m_end(end)
{

}


QuadraticBezierCurve2D::QuadraticBezierCurve2D()
{

}


void QuadraticBezierCurve2D::SetMiddlePoint(Vec2 const& middlePoint)
{
	Vec2& a = m_start;
	Vec2& c = m_end;

	Vec2 midwayAC = Lerp(a, c, 0.5f);
	
	Vec2& e = midwayAC;
	Vec2 const& f = middlePoint;
	Vec2 ef = f - e;
	float efLength = ef.GetLengthAndNormalise();
	m_guide = middlePoint + efLength * ef;
}


Vec2 QuadraticBezierCurve2D::GetPointAtFractionOfCurve(float fraction) const
{
	Vec2 const& a = m_start;
	Vec2 const& b = m_guide;
	Vec2 const& c = m_end;

	Vec2 abLerp = Lerp(a, b, fraction);
	Vec2 bcLerp = Lerp(b, c, fraction);
	Vec2 pointAtFraction = Lerp(abLerp, bcLerp, fraction);
	return pointAtFraction;
}


Vec2 QuadraticBezierCurve2D::GetMiddlePoint() const
{
	return GetPointAtFractionOfCurve(0.5f);
}


CubicBezierCurve2D::CubicBezierCurve2D()
{

}


CubicBezierCurve2D::CubicBezierCurve2D(Vec2 const& start, Vec2 const& startGuide, Vec2 const& endGuide, Vec2 const& end)
	: m_start(start)
	, m_startGuide(startGuide)
	, m_endGuide(endGuide)
	, m_end(end)
{

}


Vec2 CubicBezierCurve2D::GetPointAtFraction(float zeroToOne) const
{
	Vec2 const& a = m_start;
	Vec2 const& b = m_startGuide;
	Vec2 const& c = m_endGuide;
	Vec2 const& d = m_end;

	Vec2 abLerp = Lerp(a, b, zeroToOne);
	Vec2 bcLerp = Lerp(b, c, zeroToOne);
	Vec2 cdLerp = Lerp(c, d, zeroToOne);
	Vec2 acLerp = Lerp(abLerp, bcLerp, zeroToOne);
	Vec2 bdLerp = Lerp(bcLerp, cdLerp, zeroToOne);
	Vec2 pointAtFraction = Lerp(acLerp, bdLerp, zeroToOne);
	return pointAtFraction;
}


CubicHermiteCurve2D CubicBezierCurve2D::GetCubicHermiteCurve() const
{
	CubicHermiteCurve2D hermiteCurve = {};
	hermiteCurve.m_start = m_start;
	hermiteCurve.m_end = m_end;
	hermiteCurve.m_startVelocity = (m_startGuide - m_start) * 3.0f;
	hermiteCurve.m_endVelocity = (m_end - m_endGuide) * 3.0f;
	return hermiteCurve;
}


bool CubicBezierCurve2D::operator==(CubicBezierCurve2D const& compare)
{
	return m_start == compare.m_start && m_end == compare.m_end && m_startGuide == compare.m_startGuide && m_endGuide == compare.m_endGuide;
}


Vec2 CubicHermiteCurve2D::GetPointAtFraction(float zeroToOne) const
{
	CubicBezierCurve2D bezier = GetCubicBezierCurve();
	return bezier.GetPointAtFraction(zeroToOne);
}


CubicBezierCurve2D CubicHermiteCurve2D::GetCubicBezierCurve() const
{
	CubicBezierCurve2D bezierCurve = {};
	float oneThird = 1.0f / 3.0f;
	bezierCurve.m_start = m_start;
	bezierCurve.m_end = m_end;
	bezierCurve.m_startGuide = m_start + (m_startVelocity * oneThird);
	bezierCurve.m_endGuide = m_end - (m_endVelocity * oneThird);
	return bezierCurve;
}


Vec2 CubicHermiteCurve2D::GetStartVelocityEndPoint() const
{
	return m_start + m_startVelocity;
}


Vec2 CubicHermiteCurve2D::GetEndVelocityEndPoint() const
{
	return m_end + m_endVelocity;
}
