#include "Game/Spline.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Spline::Spline()
{
// 	static float worldX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
// 	static float worldY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
// 
// 	CubicHermiteCurve2D hermiteCurve;
// 	hermiteCurve.m_start		= Vec2(worldX * 0.10f, worldY * 0.50f);
// 	hermiteCurve.m_end			= Vec2(worldX * 0.40f, worldY * 0.50f);
// 	hermiteCurve.m_startVelocity	= Vec2::ZERO;
// 	hermiteCurve.m_endVelocity		= Vec2::ZERO;
// 	m_curves.push_back(hermiteCurve);
}


Spline::Spline(XmlElement const* fromXmlElement)
{
	XmlElement const* curveElement = fromXmlElement->FirstChildElement("Curve");
	while (curveElement)
	{
		CubicBezierCurve2D bezierCurve;
		bezierCurve.m_start			= ParseXmlAttribute(*curveElement, "start",			Vec2::ZERO);
		bezierCurve.m_startGuide	= ParseXmlAttribute(*curveElement, "startGuide",	Vec2::ZERO);
		bezierCurve.m_endGuide		= ParseXmlAttribute(*curveElement, "endGuide",		Vec2::ZERO);
		bezierCurve.m_end			= ParseXmlAttribute(*curveElement, "end",			Vec2::ZERO);
		m_curves.push_back(bezierCurve.GetCubicHermiteCurve());

		curveElement = curveElement->NextSiblingElement("Curve");
	}
}


void Spline::Render() const
{
	Rgba8 curveColor = g_gameConfigBlackboard.GetValue("splineColor", Rgba8::MAGENTA);
	float splineWidth = g_gameConfigBlackboard.GetValue("splineWidth", 2.0f);

	std::vector<Vertex_PCU> verts;
	for (int curveIndex = 0; curveIndex < (int) m_curves.size(); curveIndex++)
	{
		CubicHermiteCurve2D const& curve = m_curves[curveIndex];
		AddVertsForCubicHermiteCurve2DToVector(verts, curve, curveColor, splineWidth, 0.0f);
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
}


void Spline::SetCurveAtIndex(CubicHermiteCurve2D curve, int index)
{
	if (index >= (int) m_curves.size())
		index = (int) m_curves.size() - 1;

	m_curves[index] = curve;
}


CubicHermiteCurve2D Spline::AddCurveAtEnd()
{
	static Vec2 defaultCurveVelocities = g_gameConfigBlackboard.GetValue("defaultCurveVelocities", Vec2(0.0f, 50.0f));
	CubicHermiteCurve2D hermiteCurve;
	int lastCurveIndex = (int) m_curves.size() - 1;
	if (lastCurveIndex >= 0)
	{
		CubicHermiteCurve2D lastCurve = m_curves[lastCurveIndex];

		hermiteCurve.m_start = lastCurve.m_end;
		hermiteCurve.m_end = hermiteCurve.m_start + Vec2(50.0f, 0.0f);
		hermiteCurve.m_startVelocity = lastCurve.m_endVelocity;
		hermiteCurve.m_endVelocity = defaultCurveVelocities;
	}
	else
	{
		static float uiScreenX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
		static float uiScreenY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);

		hermiteCurve.m_start		= Vec2(uiScreenX * 0.10f, uiScreenY * 0.50f);
		hermiteCurve.m_end			= Vec2(uiScreenX * 0.40f, uiScreenY * 0.50f);
		hermiteCurve.m_startVelocity = defaultCurveVelocities;
		hermiteCurve.m_endVelocity = defaultCurveVelocities;
	}
	m_curves.push_back(hermiteCurve);
	return hermiteCurve;
}


CubicHermiteCurve2D Spline::DeleteCurveAtEnd()
{
	CubicHermiteCurve2D lastCurve = m_curves.back();
	m_curves.pop_back();
	return lastCurve;
}


void Spline::Clear()
{
	m_curves.clear();
}


Vec2 Spline::GetPointAtFraction(float zeroToOneFraction) const
{
	float denormalisedFraction = zeroToOneFraction * (int) m_curves.size();
	int curveIndex = RoundDownToInt(denormalisedFraction);
	curveIndex = Clamp(curveIndex, 0, (int) m_curves.size() - 1);
	float curveFraction = denormalisedFraction - curveIndex;

	return m_curves[curveIndex].GetPointAtFraction(curveFraction);
}


CubicHermiteCurve2D Spline::GetCurveAtIndex(int index) const
{
	return m_curves[index];
}


int Spline::GetLength() const
{
	return (int) m_curves.size();
}


CubicHermiteCurve2D Spline::GetLastCurve()
{
	return m_curves.back();
}


void SplineDefinition::LoadSplineDefinitionsFromXml()
{
	std::string splineDefinitionsFileName = g_gameConfigBlackboard.GetValue("splineDefinitionsFileName", "Data/XMLData/SplineDefinitions.xml");
	XmlDocument splineDefinitionDoc;
	XmlError result = splineDefinitionDoc.LoadFile(splineDefinitionsFileName.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to open spline definitions xml file. TinyXml Error code: %i", result));

	XmlElement const* rootElement = splineDefinitionDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "SplineDefinitions") == 0, "Spline Definition xml does not contain the SplineDefinitionsRootElement");

	XmlElement const* definition = rootElement->FirstChildElement("Definition");
	while (definition)
	{
		std::string name = ParseXmlAttribute(*definition, "name", "");
		GUARANTEE_OR_DIE(!DoesDefinitionAlreadyExist(name), Stringf("Duplicate data in spline xml. Duplicate Name: %s", name.c_str()));
		SplineDefinition* splineDefinition = new SplineDefinition();
		splineDefinition->name = name;
		splineDefinition->m_spline = Spline(definition);
		s_splineDefinitions.push_back(splineDefinition);

		definition = definition->NextSiblingElement("Definition");
	}
}


SplineDefinition const* SplineDefinition::GetSplineDefinitionByName(std::string const& name)
{
	for (int splineIndex = 0; splineIndex < (int) s_splineDefinitions.size(); splineIndex++)
	{
		SplineDefinition const*& definition = s_splineDefinitions[splineIndex];
		if (_stricmp(definition->name.c_str(), name.c_str()) == 0)
		{
			return definition;
		}
	}

	return nullptr;
}


bool SplineDefinition::DoesDefinitionAlreadyExist(std::string const& definitionName)
{
	for (int splineIndex = 0; splineIndex < (int) s_splineDefinitions.size(); splineIndex++)
	{
		SplineDefinition const*& definition = s_splineDefinitions[splineIndex];
		if (_stricmp(definition->name.c_str(), definitionName.c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}


void SplineDefinition::DeleteAllSplineDefinitions()
{
	for (int splineIndex = 0; splineIndex < (int) s_splineDefinitions.size(); splineIndex++)
	{
		SplineDefinition const*& definition = s_splineDefinitions[splineIndex];
		if (definition)
		{
			delete definition;
			definition = nullptr;
		}
	}
	s_splineDefinitions.clear();
}


std::vector<SplineDefinition const*> SplineDefinition::s_splineDefinitions;
