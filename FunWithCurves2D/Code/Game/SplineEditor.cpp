#include "Game/SplineEditor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/DevConsole.hpp"

const unsigned char KEYCODE_ADD_CURVE		= KEYCODE_SPACE_BAR;
const unsigned char KEYCODE_DELETE_CURVE	= KEYCODE_BACKSPACE;
const unsigned char KEYCODE_DELETE_SPLINE	= KEYCODE_DELETE;
const unsigned char KEYCODE_SAVE_SPLINE		= 'P';

bool operator!=(SplineControlPoint const& a, SplineControlPoint const& b)
{
	return (a.m_index != b.m_index) && (a.m_curveIndexInSpline != b.m_curveIndexInSpline);
}


int SplineEditor::s_caratPos = 0;
bool SplineEditor::s_isSaving = false;
std::string SplineEditor::s_splineToSaveName;

bool SplineEditor::HandleCharDownEvent(EventArgs& args)
{
	std::string inputKey = "\0";
	args.GetProperty("InputKey", inputKey, inputKey);
	bool wasConsumed = false;
	if (inputKey[0] >= 32 && inputKey[0] <= 125 && s_isSaving)
	{
		s_splineToSaveName.push_back(inputKey[0]);
		s_caratPos++;
		wasConsumed = true;
	}
	return wasConsumed;
}


SplineEditor::SplineEditor()
{

}


void SplineEditor::Startup()
{
	m_pointOnSplineTimer.Start(1.0);
	
	if (m_spline.GetLength() <= 0)
	{
		AddCurveAtEndOfSpline();
	}

	m_uiScreenX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	m_uiScreenY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiWorldSpace = AABB2(Vec2(m_uiScreenX * 0.5f, m_uiScreenY * 0.5f), m_uiScreenX * 0.7f, m_uiScreenY * 0.7f);

	m_caratStopwatch = Stopwatch(g_game->GetClock(), 0.3);
}


void SplineEditor::Shutdown()
{
}


void SplineEditor::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (m_spline.GetLength() > 0)
	{
		float time = (float) m_pointOnSplineTimer.GetElapsedSeconds();
		time = ClampZeroToOne(time);
		m_pointOnSpline = m_spline.GetPointAtFraction(time);
		m_pointOnSplineTimer.CheckAndDecrement();
	}

	if (s_isSaving)
	{
		m_splineNameToSave = s_splineToSaveName;

		if (m_caratStopwatch.CheckAndRestart())
		{
			m_showCarat = !m_showCarat;
		}

		if (m_showCarat)
		{
			std::string carat(1, '|');
			m_splineNameToSave.insert(s_caratPos, carat);
		}
		else
		{
			std::string carat(1, ' ');
			m_splineNameToSave.insert(s_caratPos, carat);
		}
	}

	std::string controlText1 = "Spacebar: Create/Add curve";
	std::string controlText2 = "Backspace: Delete curve from the end";
	std::string controlText3 = "Mouse (Left mouse click): Adjust control points";
	std::string controlText4 = "P: Save curve to file | F5: Toggle control points";
	std::string optionsText = "Enter: Go to game (or) save spline after entering the name";
	float textSize = 6.0f;
	float cursorOffset = 10.0f;
	DebugAddScreenText(controlText1, Vec2(0.0f, m_uiScreenY - cursorOffset), 0.0f, Vec2::ZERO, textSize);
	DebugAddScreenText(controlText2, Vec2(0.0f, m_uiScreenY - cursorOffset * 2.0f), 0.0f, Vec2::ZERO, textSize);
	DebugAddScreenText(controlText3, Vec2(0.0f, m_uiScreenY - cursorOffset * 3.0f), 0.0f, Vec2::ZERO, textSize);
	DebugAddScreenText(controlText4, Vec2(0.0f, m_uiScreenY - cursorOffset * 4.0f), 0.0f, Vec2::ZERO, textSize);
	DebugAddScreenText(optionsText, Vec2(0.0f,	m_uiScreenY - cursorOffset * 5.0f) , 0.0f, Vec2::ZERO,	textSize);
}


void SplineEditor::CheckInput()
{
	if (s_isSaving)
	{
		if (g_inputSystem->WasKeyJustPressed(KEYCODE_BACKSPACE))
		{
			if (s_splineToSaveName.size() > 0)
			{
				s_splineToSaveName.pop_back();
				s_caratPos--;
			}
		}

		if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER))
		{
			SaveSplineToFile(s_splineToSaveName);
		}

		if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
		{
			SaveTextBoxExit();
		}
	}
	else
	{
		if (g_inputSystem->WasKeyJustPressed(KEYCODE_ADD_CURVE))
		{
			AddCurveAtEndOfSpline();
		}

		if (g_inputSystem->WasKeyJustPressed(KEYCODE_DELETE_CURVE))
		{
			DeleteCurveAtEndOfSpline();
		}

		if (g_inputSystem->WasKeyJustPressed(KEYCODE_DELETE_SPLINE))
		{
			m_spline.Clear();
			m_splineControlPoints.clear();
		}

		if (g_inputSystem->IsMouseButtonUp(0))
		{
			m_activeControlPoint = SplineControlPoint::INVALID;
		}

		if (g_inputSystem->WasKeyJustPressed(KEYCODE_F5))
		{
			m_showControlPoints = !m_showControlPoints;
		}

		Vec2 windowDims = g_window->GetWindowDims();
		IntVec2 mousePos = g_inputSystem->GetMousePosition();
		Vec2 mousePosInUISpace = RangeMap(mousePos.GetVec2(), Vec2::ZERO, windowDims, Vec2(0.0f, m_uiScreenY), Vec2(m_uiScreenX, 0.0f));

		int closestControlPointIndex = GetIndexForClosestControlPoint(mousePosInUISpace, 10.0f);
		if (closestControlPointIndex >= 0)
		{
			m_activeControlPoint = m_splineControlPoints[closestControlPointIndex];
		}

		if (g_inputSystem->IsMouseButtonPressed(0) && m_activeControlPoint != SplineControlPoint::INVALID)
		{
			Vec2 controlPointPos = m_activeControlPoint.m_position;
			int curveIndex = m_activeControlPoint.m_curveIndexInSpline;
			CubicHermiteCurve2D activeHermiteCurve = m_spline.GetCurveAtIndex(curveIndex);
			switch (m_activeControlPoint.m_controlPointType)
			{
				case ControlPointType::START:
				{
					activeHermiteCurve.m_start = mousePosInUISpace;
					if (curveIndex > 0)
					{
						CubicHermiteCurve2D previousCurveHermite = m_spline.GetCurveAtIndex(curveIndex - 1);
						previousCurveHermite.m_end = activeHermiteCurve.m_start;
						m_spline.SetCurveAtIndex(previousCurveHermite, curveIndex - 1);
					}
					int startVelocityControlPointIndex = m_activeControlPoint.m_index + 1;
					if (startVelocityControlPointIndex < (int) m_splineControlPoints.size())
						m_splineControlPoints[startVelocityControlPointIndex].m_position = activeHermiteCurve.GetStartVelocityEndPoint();
					break;
				}
				case ControlPointType::START_VELOCITY:
				{
					activeHermiteCurve.m_startVelocity = mousePosInUISpace - activeHermiteCurve.m_start;
					if (curveIndex > 0)
					{
						CubicHermiteCurve2D previousCurveHermite = m_spline.GetCurveAtIndex(curveIndex - 1);
						previousCurveHermite.m_endVelocity = activeHermiteCurve.m_startVelocity;
						m_spline.SetCurveAtIndex(previousCurveHermite, curveIndex - 1);
					}
					break;
				}
				case  ControlPointType::END_VELOCITY:
				{
					activeHermiteCurve.m_endVelocity = mousePosInUISpace - activeHermiteCurve.m_end;
					int lastCurveIndex = m_spline.GetLength() - 1;
					if (curveIndex < lastCurveIndex)
					{
						CubicHermiteCurve2D nextCurveHermite = m_spline.GetCurveAtIndex(curveIndex + 1);
						nextCurveHermite.m_startVelocity = activeHermiteCurve.m_endVelocity;
						m_spline.SetCurveAtIndex(nextCurveHermite, curveIndex + 1);
					}
					break;
				}
				case  ControlPointType::END:
				{
					activeHermiteCurve.m_end = mousePosInUISpace;
					int lastCurveIndex = m_spline.GetLength() - 1;
					if (curveIndex < lastCurveIndex)
					{
						CubicHermiteCurve2D nextCurveHermite = m_spline.GetCurveAtIndex(curveIndex + 1);
						nextCurveHermite.m_start = activeHermiteCurve.m_end;
						m_spline.SetCurveAtIndex(nextCurveHermite, curveIndex + 1);
					}
					int endVelocityControlPointIndex = m_activeControlPoint.m_index - 1;
					if (endVelocityControlPointIndex < (int) m_splineControlPoints.size())
						m_splineControlPoints[endVelocityControlPointIndex].m_position = activeHermiteCurve.GetEndVelocityEndPoint();
					break;
				}
			}
			m_spline.SetCurveAtIndex(activeHermiteCurve, curveIndex);
			m_activeControlPoint.m_position = mousePosInUISpace;
			m_splineControlPoints[m_activeControlPoint.m_index] = m_activeControlPoint;
		}

		if (g_inputSystem->WasKeyJustPressed(KEYCODE_SAVE_SPLINE) && m_spline.GetLength() > 0)
		{
			s_isSaving = true;
			g_eventSystem->SubscribeEventCallbackFunction("CharDown", HandleCharDownEvent);
		}
	}
}


void SplineEditor::Render(Camera const& camera) const
{
	g_theRenderer->ClearScreen(Rgba8::GREY);
	g_theRenderer->BeginCamera(camera);
	{
		static RenderMaterial worldSpaceMaterial = {};
		worldSpaceMaterial.m_color = Rgba8::BLACK;
		g_theRenderer->DrawAABB2D(m_uiWorldSpace, worldSpaceMaterial);

		if (m_spline.GetLength() > 0)
		{
			std::vector<Vertex_PCU> verts;

			float pointRadius = g_gameConfigBlackboard.GetValue("pointOnSplineRadius", 5.0f);
			AddVertsForDisk2DToVector(verts, m_pointOnSpline, pointRadius, Rgba8::CYAN);
			m_spline.Render();

			if (m_showControlPoints)
			{
				float splineWidth = g_gameConfigBlackboard.GetValue("splineWidth", 2.0f);
				float arrowTipWidth = g_gameConfigBlackboard.GetValue("arrowTipWidth", 10.0f);

				for (int controlPtIndex = 0; controlPtIndex < m_splineControlPoints.size(); controlPtIndex++)
				{
					SplineControlPoint controlPt = m_splineControlPoints[controlPtIndex];
					switch (controlPt.m_controlPointType)
					{
						case ControlPointType::START:
						case  ControlPointType::END:
						{
							AddVertsForDisk2DToVector(verts, controlPt.m_position, pointRadius, Rgba8::RED);
							break;
						}
						case ControlPointType::START_VELOCITY:
						{
							CubicHermiteCurve2D hermiteCurve = m_spline.GetCurveAtIndex(controlPt.m_curveIndexInSpline);
							AddVertsForArrow2DToVector(verts, hermiteCurve.m_start, hermiteCurve.GetStartVelocityEndPoint(), Rgba8::RED, splineWidth, arrowTipWidth, 0.0f);
							break;
						}
						case  ControlPointType::END_VELOCITY:
						{
							CubicHermiteCurve2D hermiteCurve = m_spline.GetCurveAtIndex(controlPt.m_curveIndexInSpline);
							AddVertsForArrow2DToVector(verts, hermiteCurve.m_end, hermiteCurve.GetEndVelocityEndPoint(), Rgba8::RED, splineWidth, arrowTipWidth, 0.0f);
							break;
						}
						default:
							break;
					}
				}

				if (m_activeControlPoint.m_curveIndexInSpline >= 0)
				{
					DrawRing(m_activeControlPoint.m_position, pointRadius * 1.4f, splineWidth, Rgba8::GREEN);
				}
			}

			g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
		}

		static BitmapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		std::vector<Vertex_PCU> textVerts;
		Rgba8 textColor = Rgba8::BLACK;
		Vec2 uiWorldSpaceDims = m_uiWorldSpace.GetDimensions();
		bitmapFont->AddVertsForText2D(textVerts, Vec2(m_uiWorldSpace.m_mins.x + uiWorldSpaceDims.x * 0.1f, m_uiWorldSpace.m_maxs.y), 20.0f, "Play Area", textColor, 1.0f);
		if (s_isSaving)
		{
			Rgba8 textBoxColor = Rgba8(100, 100, 100, 200);
			std::vector<Vertex_PCU> textBoxVerts;
			Vec2 textInputBoxMins = Vec2(m_uiScreenX * 0.30f, m_uiScreenY * 0.45f);
			Vec2 textInputBoxMaxs = Vec2(m_uiScreenX * 0.70f, m_uiScreenY * 0.55f);
			AABB2 textInputBox = AABB2(textInputBoxMins, textInputBoxMaxs);

			AddVertsForAABB2ToVector(textBoxVerts, textInputBox, textBoxColor);
			g_theRenderer->BindTexture(0, nullptr);
			g_theRenderer->DrawVertexArray((int) textBoxVerts.size(), textBoxVerts.data());
			bitmapFont->AddVertsForTextInAABB2(textVerts, textInputBox, 30.0f, m_splineNameToSave, textColor, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);
		}
		g_theRenderer->BindTexture(0, &bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());

		float playerShipSpriteBoxSize = 20.0f;
		std::vector<Vertex_PCU> playerShipSpriteVerts;
		Vec2 uvMins;
		Vec2 uvMaxs;
		g_gameSpriteSheet->GetSpriteUVs(uvMins, uvMaxs, IntVec2(0, 0));
		AddVertsForAABB2ToVector(playerShipSpriteVerts, AABB2(Vec2::ZERO, playerShipSpriteBoxSize, playerShipSpriteBoxSize), Rgba8::WHITE, uvMins, uvMaxs);
		TransformVertexArrayXY3D((int) playerShipSpriteVerts.size(), playerShipSpriteVerts.data(), 1.0f, 90.0f, Vec2(m_uiWorldSpace.m_mins.x + uiWorldSpaceDims.x * 0.5f, m_uiWorldSpace.m_mins.y + playerShipSpriteBoxSize * 0.5f));

		Texture const& spriteSheetTexture = g_gameSpriteSheet->GetTexture();
		g_theRenderer->BindTexture(0, &spriteSheetTexture);
		g_theRenderer->DrawVertexArray((int) playerShipSpriteVerts.size(), playerShipSpriteVerts.data());
	}
	g_theRenderer->EndCamera(camera);
}


bool SplineEditor::SaveSplineToFile(std::string curveName)
{
	if (m_spline.GetLength() <= 0)
	{
		return false;
	}

	std::string splineDefinitionsFileName = g_gameConfigBlackboard.GetValue("splineDefinitionsFileName", "Data/XMLData/SplineDefinitions.xml");
	XmlDocument splineXml;
	XmlError result = splineXml.LoadFile(splineDefinitionsFileName.c_str());
	XmlElement* rootElement = nullptr;
	if (result == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{
		rootElement = splineXml.NewElement("SplineDefinitions");
		splineXml.InsertFirstChild(rootElement);
	}
	else if (result == tinyxml2::XML_SUCCESS)
	{
		rootElement = splineXml.RootElement();
		GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "SplineDefinitions") == 0, "Spline Definitions file does have the root element SplineDefinitions in it");
	}

	XmlElement* definitionElement = rootElement->InsertNewChildElement("Definition");
	definitionElement->SetAttribute("name", curveName.c_str());
	int curveCount = m_spline.GetLength();
	static float worldX = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
	static float worldY = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
	static Vec2 worldDims = Vec2(worldX, worldY);

	for (int curveIndex = 0; curveIndex < curveCount; curveIndex++)
	{
		//storing the curves as bezier curves instead of hermite because they're the same
		// and bezier curves are easier to transpose to a different space since its all points instead of points and displacements
		CubicHermiteCurve2D hermiteCurveInEditorSpace = m_spline.GetCurveAtIndex(curveIndex);
		CubicBezierCurve2D bezierCurveInEditorSpace = hermiteCurveInEditorSpace.GetCubicBezierCurve();
		CubicBezierCurve2D bezierCurveInGameSpace;
		bezierCurveInGameSpace.m_start		= RangeMap(bezierCurveInEditorSpace.m_start,		m_uiWorldSpace.m_mins, m_uiWorldSpace.m_maxs, Vec2::ZERO, worldDims);
		bezierCurveInGameSpace.m_startGuide = RangeMap(bezierCurveInEditorSpace.m_startGuide,	m_uiWorldSpace.m_mins, m_uiWorldSpace.m_maxs, Vec2::ZERO, worldDims);
		bezierCurveInGameSpace.m_endGuide	= RangeMap(bezierCurveInEditorSpace.m_endGuide,		m_uiWorldSpace.m_mins, m_uiWorldSpace.m_maxs, Vec2::ZERO, worldDims);
		bezierCurveInGameSpace.m_end		= RangeMap(bezierCurveInEditorSpace.m_end,			m_uiWorldSpace.m_mins, m_uiWorldSpace.m_maxs, Vec2::ZERO, worldDims);

		XmlElement* curveElement = definitionElement->InsertNewChildElement("Curve");
		curveElement->SetAttribute("start",			Stringf("%.2f,%.2f", bezierCurveInGameSpace.m_start.x, bezierCurveInGameSpace.m_start.y).c_str());
		curveElement->SetAttribute("startGuide",	Stringf("%.2f,%.2f", bezierCurveInGameSpace.m_startGuide.x, bezierCurveInGameSpace.m_startGuide.y).c_str());
		curveElement->SetAttribute("endGuide",		Stringf("%.2f,%.2f", bezierCurveInGameSpace.m_endGuide.x, bezierCurveInGameSpace.m_endGuide.y).c_str());
		curveElement->SetAttribute("end",			Stringf("%.2f,%.2f", bezierCurveInGameSpace.m_end.x, bezierCurveInGameSpace.m_end.y).c_str());
	}

	result = splineXml.SaveFile(splineDefinitionsFileName.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to save spline to xml. TinyXml Error Code %i", result));

	m_spline.Clear();
	SaveTextBoxExit();

	return true;
}


void SplineEditor::SaveTextBoxExit()
{
	s_isSaving = false;
	s_splineToSaveName.clear();
	s_caratPos = 0;
	m_splineControlPoints.clear();
	g_eventSystem->UnsubscribeEventCallbackFunction("CharDown", HandleCharDownEvent);
}


void SplineEditor::AddCurveAtEndOfSpline()
{
	if (m_splineControlPoints.size() > 0)
	{
		//to avoid duplicate control points in the list for overlapping points in the spline
		m_splineControlPoints.pop_back();
		m_splineControlPoints.pop_back(); 
	}
	
	CubicHermiteCurve2D addedHermiteCurve = m_spline.AddCurveAtEnd();
	int curveIndex = (int) m_spline.GetLength() - 1;
	SplineControlPoint curveStartControlPoint = {};
	curveStartControlPoint.m_controlPointType = ControlPointType::START;
	curveStartControlPoint.m_index = (int) m_splineControlPoints.size();
	curveStartControlPoint.m_curveIndexInSpline = curveIndex;
	curveStartControlPoint.m_position = addedHermiteCurve.m_start;
	m_splineControlPoints.push_back(curveStartControlPoint);

	SplineControlPoint curveStartVelocityControlPoint = {};
	curveStartVelocityControlPoint.m_controlPointType = ControlPointType::START_VELOCITY;
	curveStartVelocityControlPoint.m_index = (int) m_splineControlPoints.size();
	curveStartVelocityControlPoint.m_curveIndexInSpline = curveIndex;
	curveStartVelocityControlPoint.m_position = addedHermiteCurve.GetStartVelocityEndPoint();
	m_splineControlPoints.push_back(curveStartVelocityControlPoint);

	SplineControlPoint curveEndVelocityControlPoint = {};
	curveEndVelocityControlPoint.m_controlPointType = ControlPointType::END_VELOCITY;
	curveEndVelocityControlPoint.m_index = (int) m_splineControlPoints.size();
	curveEndVelocityControlPoint.m_curveIndexInSpline = curveIndex;
	curveEndVelocityControlPoint.m_position = addedHermiteCurve.GetEndVelocityEndPoint();
	m_splineControlPoints.push_back(curveEndVelocityControlPoint);

	SplineControlPoint curveEndControlPoint = {};
	curveEndControlPoint.m_controlPointType = ControlPointType::END;
	curveEndControlPoint.m_index = (int) m_splineControlPoints.size();
	curveEndControlPoint.m_curveIndexInSpline = curveIndex;
	curveEndControlPoint.m_position = addedHermiteCurve.m_end;
	m_splineControlPoints.push_back(curveEndControlPoint);
}


void SplineEditor::DeleteCurveAtEndOfSpline()
{
	int splineLength = m_spline.GetLength();
	if (splineLength <= 0)
		return;

	for (int i = 0; i < 4; i++)
	{
		m_splineControlPoints.pop_back();
	}
	m_spline.DeleteCurveAtEnd();
	splineLength--;

	if (splineLength > 0)
	{
		CubicHermiteCurve2D lastCurve = m_spline.GetLastCurve();
	
		SplineControlPoint curveEndVelocityControlPoint = {};
		curveEndVelocityControlPoint.m_controlPointType = ControlPointType::END_VELOCITY;
		curveEndVelocityControlPoint.m_index = (int) m_splineControlPoints.size();
		curveEndVelocityControlPoint.m_curveIndexInSpline = splineLength - 1;
		curveEndVelocityControlPoint.m_position = lastCurve.GetEndVelocityEndPoint();
		m_splineControlPoints.push_back(curveEndVelocityControlPoint);

		SplineControlPoint curveEndControlPoint = {};
		curveEndControlPoint.m_controlPointType = ControlPointType::END;
		curveEndControlPoint.m_index = (int) m_splineControlPoints.size();
		curveEndControlPoint.m_curveIndexInSpline = splineLength - 1;
		curveEndControlPoint.m_position = lastCurve.m_end;
		m_splineControlPoints.push_back(curveEndControlPoint);
	}
}


int SplineEditor::GetIndexForClosestControlPoint(Vec2 const& refPos, float thresholdDistace /*= 5.0f*/)
{
	int closestControlPtIndex = -1;
	float closestDistanceSquared = 9999999.0f;
	for (int controlPtIndex = 0; controlPtIndex < (int) m_splineControlPoints.size(); controlPtIndex++)
	{
		Vec2 controlPointPos = m_splineControlPoints[controlPtIndex].m_position;
		float distSqrdBwControlAndRef = (refPos - controlPointPos).GetLengthSquared();
		if (distSqrdBwControlAndRef < closestDistanceSquared)
		{
			closestDistanceSquared = distSqrdBwControlAndRef;
			closestControlPtIndex = controlPtIndex;
		}
	}

	if (SqrtFloat(closestDistanceSquared) <= thresholdDistace)
		return closestControlPtIndex;
	
	return -1;
}


bool SplineEditor::IsSaving() const
{
	return s_isSaving;
}


const SplineControlPoint SplineControlPoint::INVALID = SplineControlPoint();
