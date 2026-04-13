#pragma once
#include "Engine/Math/BezierCurve2D.hpp"
#include<vector>
#include "Engine/Core/XmlUtils.hpp"

class Spline
{
public:
	Spline();
	Spline(XmlElement const* fromXmlElement);
	void Render() const;

	void SetCurveAtIndex(CubicHermiteCurve2D curve, int index);
	CubicHermiteCurve2D AddCurveAtEnd();
	CubicHermiteCurve2D DeleteCurveAtEnd();
	void Clear();

	Vec2 GetPointAtFraction(float zeroToOneFraction) const;
	CubicHermiteCurve2D GetCurveAtIndex(int index) const;
	int GetLength() const;
	CubicHermiteCurve2D GetLastCurve();

private:
	std::vector<CubicHermiteCurve2D> m_curves;
};


struct SplineDefinition
{
public:
	static void LoadSplineDefinitionsFromXml();
	static SplineDefinition const* GetSplineDefinitionByName(std::string const& name);
	static bool DoesDefinitionAlreadyExist(std::string const& definitionName);
	static void DeleteAllSplineDefinitions();

public:
	Spline m_spline;
	std::string name;

protected:
	static std::vector<SplineDefinition const*> s_splineDefinitions;
};
