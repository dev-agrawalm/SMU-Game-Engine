#pragma once
#include "Engine/Math/BezierCurve2D.hpp"
#include<vector>
#include "Engine/Core/Rgba8.hpp"

class Spline2D
{
public:
	Spline2D();
	void Render() const;

	void SetCurveAtIndex(CubicHermiteCurve2D const& curve, int index);
	void AddCurveAtEnd(CubicHermiteCurve2D const& curve);
	CubicHermiteCurve2D DeleteCurveAtEnd();
	void Clear();

	Vec2 GetPointAtFraction(float zeroToOneFraction) const;
	CubicHermiteCurve2D GetCurveAtIndex(int index) const;
	int GetNumCurves() const;
	CubicHermiteCurve2D GetLastCurve();

private:
	std::vector<CubicHermiteCurve2D> m_curves;
	Rgba8 m_color;
	float m_width = 0.0f;
};