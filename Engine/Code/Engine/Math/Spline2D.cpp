#include "Engine/Math/Spline2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
//engine cannot reference game code - this is illegal
//#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Spline2D::Spline2D()
{
}


void Spline2D::Render() const
{
	std::vector<Vertex_PCU> verts;
	for (int curveIndex = 0; curveIndex < (int) m_curves.size(); curveIndex++)
	{
		CubicHermiteCurve2D const& curve = m_curves[curveIndex];
		AddVertsForCubicHermiteCurve2DToVector(verts, curve, m_color, m_width, 0.0f);
	}

	//#TODO fix this: Spline2D should not interact with the renderer
	//g_theRenderer->BindShader(nullptr);
	//g_theRenderer->BindTexture(0, nullptr);
	//g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
}


void Spline2D::SetCurveAtIndex(CubicHermiteCurve2D const& curve, int index)
{
	if (index >= (int) m_curves.size())
		index = (int) m_curves.size() - 1;

	m_curves[index] = curve;
}


void Spline2D::AddCurveAtEnd(CubicHermiteCurve2D const& curveToAdd)
{
	m_curves.push_back(curveToAdd);
}


CubicHermiteCurve2D Spline2D::DeleteCurveAtEnd()
{
	CubicHermiteCurve2D lastCurve = m_curves.back();
	m_curves.pop_back();
	return lastCurve;
}


void Spline2D::Clear()
{
	m_curves.clear();
}


Vec2 Spline2D::GetPointAtFraction(float zeroToOneFraction) const
{
	float denormalisedFraction = zeroToOneFraction * (int) m_curves.size();
	int curveIndex = RoundDownToInt(denormalisedFraction);
	curveIndex = Clamp(curveIndex, 0, (int) m_curves.size() - 1);
	float curveFraction = denormalisedFraction - curveIndex;

	return m_curves[curveIndex].GetPointAtFraction(curveFraction);
}


CubicHermiteCurve2D Spline2D::GetCurveAtIndex(int index) const
{
	return m_curves[index];
}


int Spline2D::GetNumCurves() const
{
	return (int) m_curves.size();
}


CubicHermiteCurve2D Spline2D::GetLastCurve()
{
	return m_curves.back();
}
