#include "Engine/Core/DebugRender.hpp"
#include<vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <mutex>
#include <atomic>

constexpr float ARROW_HEAD_HEIGHT = 0.15f;
constexpr float ARROW_HEAD_BASE_RADIUS_MULTIPLIER = 1.5f;
constexpr float ARROW_HEAD_TOP_RADIUS = 0.01f;
constexpr float BASIS_ARROW_LENGTH = 1.3f;
constexpr unsigned char XRAY_TRANSPARENCY_ALPHA = 100;


struct DebugRenderingWorldObject
{
	bool m_isText = false;
	bool m_isBillboarded = false;
	Vec3 m_billboardOrigin;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	double m_duration = 0.0;
	Stopwatch m_timer;
	FillMode m_renderFillMode = FillMode::SOLID;
	DebugRenderMode m_renderMode = DebugRenderMode::USE_DEPTH;
	std::vector<Vertex_PCU> m_objectVerts;
};


struct DebugRenderingScreenObject
{
	float m_textHeight = 0.0f;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	double m_duration = 0.0;
	Stopwatch m_timer;
	std::vector<Vertex_PCU> m_textVerts;
	std::vector<Vertex_PCU> m_textBoxVerts;
};


struct DebugRenderingData
{
	DebugRenderConfig m_config = {};
	std::atomic<bool> m_isHidden = false;
	std::vector<DebugRenderingWorldObject> m_worldObjects;
	std::vector<DebugRenderingScreenObject> m_screenMessages;
	std::vector<DebugRenderingScreenObject> m_screenTexts;
};


static Clock s_debugRenderingClock;
static std::mutex s_debugRenderDataMutex;
static DebugRenderingData s_debugRenderingData;
static BitmapFont* s_font = nullptr;


void DebugRenderSystemStartup(DebugRenderConfig const& config)
{
	UNUSED(config);
#if defined(ENABLE_ON_SCREEN_DEBUGGING)
	s_debugRenderDataMutex.lock();
	s_debugRenderingData.m_config = config;
	s_debugRenderingData.m_isHidden = config.m_startHidden;
	s_debugRenderingData.m_worldObjects.clear();
	s_debugRenderingData.m_screenMessages.clear();
	s_debugRenderingData.m_screenTexts.clear();
	s_debugRenderDataMutex.unlock();
	s_font = config.m_renderer->CreateOrGetBitmapFont(config.m_fontName.c_str());
#endif
}


void DebugRenderSystemShutdown()
{
#if defined(ENABLE_ON_SCREEN_DEBUGGING)
	s_debugRenderDataMutex.lock();
	s_debugRenderingData.m_worldObjects.clear();
	s_debugRenderingData.m_screenMessages.clear();
	s_debugRenderingData.m_screenTexts.clear();

	s_debugRenderingData.m_config.m_renderer = nullptr;
	s_debugRenderDataMutex.unlock();
	s_font = nullptr;
#endif
}


void DebugRenderSetVisible()
{
	s_debugRenderingData.m_isHidden = false;
}


void DebugRenderSetHidden()
{
	s_debugRenderingData.m_isHidden = true;
}


void DebugRenderToggleVisibility()
{
	s_debugRenderingData.m_isHidden = !s_debugRenderingData.m_isHidden;
}


void DebugRenderClear()
{
	s_debugRenderDataMutex.lock();
	s_debugRenderingData.m_screenMessages.clear();
	s_debugRenderingData.m_worldObjects.clear();
	s_debugRenderDataMutex.unlock();
}


void DebugRenderSetTimeScale(double scale)
{
	s_debugRenderDataMutex.lock();
	s_debugRenderingClock.SetTimeScale(scale);
	s_debugRenderDataMutex.unlock();
}


void DebugRenderBeginFrame()
{
#if defined(ENABLE_ON_SCREEN_DEBUGGING)
	s_debugRenderDataMutex.lock();
	std::vector<DebugRenderingWorldObject>& worldObjects = s_debugRenderingData.m_worldObjects;
	for (int worldObjIndex = 0; worldObjIndex < (int) worldObjects.size(); worldObjIndex++)
	{
		DebugRenderingWorldObject& obj = worldObjects[worldObjIndex];
		if (obj.m_duration >= 0.0f && obj.m_startColor != obj.m_endColor)
		{
			float lerpFactor = RangeMapClamped((float) obj.m_timer.GetElapsedSeconds(), 0.0f, (float) obj.m_duration, 0.0f, 1.0f);
			Rgba8 lerpedColor = Lerp(obj.m_startColor, obj.m_endColor, lerpFactor);
			TransformVertexArrayColor((int) obj.m_objectVerts.size(), obj.m_objectVerts.data(), lerpedColor);
		}
	}

	std::vector<DebugRenderingScreenObject>& screenTexts = s_debugRenderingData.m_screenTexts;
	for (int textIndex = 0; textIndex < (int) screenTexts.size(); textIndex++)
	{
		DebugRenderingScreenObject& obj = screenTexts[textIndex];
		if (obj.m_duration >= 0.0f && obj.m_startColor != obj.m_endColor)
		{
			float lerpFactor = RangeMapClamped((float) obj.m_timer.GetElapsedSeconds(), 0.0f, (float) obj.m_duration, 0.0f, 1.0f);
			Rgba8 lerpedColor = Lerp(obj.m_startColor, obj.m_endColor, lerpFactor);
			TransformVertexArrayColor((int) obj.m_textVerts.size(), obj.m_textVerts.data(), lerpedColor);
		}
	}

	std::vector<DebugRenderingScreenObject>& screenMessages = s_debugRenderingData.m_screenMessages;
	for (int messageIndex = 0; messageIndex < (int) screenMessages.size(); messageIndex++)
	{
		DebugRenderingScreenObject& obj = screenMessages[messageIndex];
		if (obj.m_duration >= 0.0f && obj.m_startColor != obj.m_endColor)
		{
			float lerpFactor = RangeMapClamped((float) obj.m_timer.GetElapsedSeconds(), 0.0f, (float) obj.m_duration, 0.0f, 1.0f);
			Rgba8 lerpedColor = Lerp(obj.m_startColor, obj.m_endColor, lerpFactor);
			TransformVertexArrayColor((int) obj.m_textVerts.size(), obj.m_textVerts.data(), lerpedColor);
		}
	}
	s_debugRenderDataMutex.unlock();
#endif
}


void DebugRenderWorldToCamera(Camera const& camera)
{
	UNUSED(camera);
#if defined(ENABLE_ON_SCREEN_DEBUGGING)

	s_debugRenderDataMutex.lock();
	Renderer*& renderer = s_debugRenderingData.m_config.m_renderer;
	std::vector<DebugRenderingWorldObject>& worldObjects = s_debugRenderingData.m_worldObjects;
	
	std::vector<Vertex_PCU> depthSolidWorldVerts;
	std::vector<Vertex_PCU> alwaysSolidWorldVerts;
	std::vector<Vertex_PCU> xRaySolidWorldVerts;
	
	std::vector<Vertex_PCU> depthWireWorldVerts;
	std::vector<Vertex_PCU> alwaysWireWorldVerts;
	std::vector<Vertex_PCU> xRayWireWorldVerts;
	
	std::vector<Vertex_PCU> depthWorldTextVerts;
	std::vector<Vertex_PCU> alwaysWorldTextVerts;
	std::vector<Vertex_PCU> xRayWorldTextVerts;

	if (!s_debugRenderingData.m_isHidden)
	{
		renderer->BeginCamera(camera);

		for (int worldObjIndex = 0; worldObjIndex < (int) worldObjects.size(); worldObjIndex++)
		{
			DebugRenderingWorldObject& obj = worldObjects[worldObjIndex];
			DebugRenderMode renderMode = obj.m_renderMode;
			FillMode renderFillMode = obj.m_renderFillMode;
			std::vector<Vertex_PCU> objectVerts = obj.m_objectVerts;

			if (obj.m_isText)
			{
				if (obj.m_isBillboarded)
				{
					Mat44 camModel = camera.GetModelMatrix();
					Vec3 camForward = camModel.GetIBasis3D();
					Vec3 camLeft = camModel.GetJBasis3D();
					Vec3 camUp = camModel.GetKBasis3D();
					Mat44 textModel;
					// the text verts' i basis is its left vector, the j basis is its up vector and k basis is its forward vector 
					// this is because the text verts are calculated assuming a 2d world with x to its left and y to its up 
					textModel.SetIJKT3D(-camLeft, camUp, -camForward, obj.m_billboardOrigin);
					TransformVertexArray3D((int) objectVerts.size(), objectVerts.data(), textModel);
				}

				if (renderMode == DebugRenderMode::USE_DEPTH)
				{
					depthWorldTextVerts.insert(depthWorldTextVerts.end(), objectVerts.begin(), objectVerts.end());
				}
				else if (renderMode == DebugRenderMode::ALWAYS)
				{
					alwaysWorldTextVerts.insert(alwaysWorldTextVerts.end(), objectVerts.begin(), objectVerts.end());
				}
				else if (renderMode == DebugRenderMode::XRAY)
				{
					xRayWorldTextVerts.insert(xRayWorldTextVerts.end(), objectVerts.begin(), objectVerts.end());
				}

				continue;
			}

			if (renderMode == DebugRenderMode::USE_DEPTH && renderFillMode == FillMode::SOLID)
			{
				depthSolidWorldVerts.insert(depthSolidWorldVerts.end(), obj.m_objectVerts.begin(), obj.m_objectVerts.end());
			}
			else if (renderMode == DebugRenderMode::ALWAYS && renderFillMode == FillMode::SOLID)
			{
				alwaysSolidWorldVerts.insert(alwaysSolidWorldVerts.end(), obj.m_objectVerts.begin(), obj.m_objectVerts.end());
			}
			else if (renderMode == DebugRenderMode::XRAY && renderFillMode == FillMode::SOLID)
			{
				xRaySolidWorldVerts.insert(xRaySolidWorldVerts.end(), obj.m_objectVerts.begin(), obj.m_objectVerts.end());
			}

			if (renderMode == DebugRenderMode::USE_DEPTH && renderFillMode == FillMode::WIREFRAME)
			{
				depthWireWorldVerts.insert(depthWireWorldVerts.end(), obj.m_objectVerts.begin(), obj.m_objectVerts.end());
			}
			else if (renderMode == DebugRenderMode::ALWAYS && renderFillMode == FillMode::WIREFRAME)
			{
				alwaysWireWorldVerts.insert(alwaysWireWorldVerts.end(), obj.m_objectVerts.begin(), obj.m_objectVerts.end());
			}
			else if (renderMode == DebugRenderMode::XRAY && renderFillMode == FillMode::WIREFRAME)
			{
				xRayWireWorldVerts.insert(xRayWireWorldVerts.end(), obj.m_objectVerts.begin(), obj.m_objectVerts.end());
			}
		}

		if (depthSolidWorldVerts.size() > 0)
		{
			renderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
			renderer->SetFillMode(FillMode::SOLID);
			renderer->DrawVertexArray((int) depthSolidWorldVerts.size(), depthSolidWorldVerts.data());
		}

		if (depthWireWorldVerts.size() > 0)
		{
			renderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
			renderer->SetFillMode(FillMode::WIREFRAME);
			renderer->DrawVertexArray((int) depthWireWorldVerts.size(), depthWireWorldVerts.data());
		}

		if (alwaysSolidWorldVerts.size() > 0)
		{
			renderer->SetDepthOptions(DepthTest::ALWAYS, false);
			renderer->SetFillMode(FillMode::SOLID);
			renderer->DrawVertexArray((int) alwaysSolidWorldVerts.size(), alwaysSolidWorldVerts.data());
		}

		if (alwaysWireWorldVerts.size() > 0)
		{
			renderer->SetDepthOptions(DepthTest::ALWAYS, false);
			renderer->SetFillMode(FillMode::WIREFRAME);
			renderer->DrawVertexArray((int) alwaysWireWorldVerts.size(), alwaysWireWorldVerts.data());
		}

		if (xRaySolidWorldVerts.size() > 0)
		{
			renderer->SetFillMode(FillMode::SOLID);
			
			renderer->SetDepthOptions(DepthTest::GREATER, false);
			renderer->SetModelMatrix(Mat44(), Rgba8(255, 255, 255, XRAY_TRANSPARENCY_ALPHA));
			renderer->DrawVertexArray((int) xRaySolidWorldVerts.size(), xRaySolidWorldVerts.data());
			
			renderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
			renderer->SetModelMatrix(Mat44(), Rgba8::WHITE);
			renderer->DrawVertexArray((int) xRaySolidWorldVerts.size(), xRaySolidWorldVerts.data());
		}

		if (xRayWireWorldVerts.size() > 0)
		{
			renderer->SetFillMode(FillMode::WIREFRAME);

			renderer->SetDepthOptions(DepthTest::GREATER, true);
			renderer->SetModelMatrix(Mat44(), Rgba8(255, 255, 255, XRAY_TRANSPARENCY_ALPHA));
			renderer->DrawVertexArray((int) xRayWireWorldVerts.size(), xRayWireWorldVerts.data());
			
			renderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
			renderer->SetModelMatrix(Mat44(), Rgba8::WHITE);
			renderer->DrawVertexArray((int) xRayWireWorldVerts.size(), xRayWireWorldVerts.data());
		}

		Texture const& fontTexture = s_font->GetTexture();
		renderer->BindTexture(0, &fontTexture);
		renderer->SetFillMode(FillMode::SOLID);
		renderer->SetCullMode(CullMode::NONE);
		renderer->SetModelMatrix(Mat44());
		if (depthWorldTextVerts.size() > 0)
		{
			renderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
			renderer->DrawVertexArray((int) depthWorldTextVerts.size(), depthWorldTextVerts.data());
		}

		if (alwaysWorldTextVerts.size() > 0)
		{
			renderer->SetDepthOptions(DepthTest::ALWAYS, false);
			renderer->DrawVertexArray((int) alwaysWorldTextVerts.size(), alwaysWorldTextVerts.data());
		}

		if (xRayWorldTextVerts.size() > 0)
		{
			renderer->SetDepthOptions(DepthTest::GREATER, true);
			renderer->SetModelMatrix(Mat44(), Rgba8(255, 255, 255, XRAY_TRANSPARENCY_ALPHA));
			renderer->DrawVertexArray((int) xRayWorldTextVerts.size(), xRayWorldTextVerts.data());

			renderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
			renderer->SetModelMatrix(Mat44(), Rgba8::WHITE);
			renderer->DrawVertexArray((int) xRayWorldTextVerts.size(), xRayWorldTextVerts.data());
		}

		renderer->EndCamera(camera);
	}
	s_debugRenderDataMutex.unlock();
#endif
}


void DebugRenderScreenToCamera(Camera const& camera)
{
	UNUSED(camera);
#if defined(ENABLE_ON_SCREEN_DEBUGGING)
	s_debugRenderDataMutex.lock();
	Renderer*& renderer = s_debugRenderingData.m_config.m_renderer;
	std::vector<DebugRenderingScreenObject>& screenTexts = s_debugRenderingData.m_screenTexts;
	std::vector<DebugRenderingScreenObject>& screenMessages = s_debugRenderingData.m_screenMessages;
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> messageVerts;
	std::vector<Vertex_PCU> textBoxVerts;
	std::vector<Vertex_PCU> messageBoxBoxVerts;

	float cursorTop = camera.GetOrthoRightTop().y;
	float cursorLeft = camera.GetOrthoLeftBottom().x;

	if (!s_debugRenderingData.m_isHidden)
	{
		renderer->BeginCamera(camera);

		for (int textIndex = 0; textIndex < (int) screenTexts.size(); textIndex++)
		{
			DebugRenderingScreenObject& obj = screenTexts[textIndex];
			textVerts.insert(textVerts.end(), obj.m_textVerts.begin(), obj.m_textVerts.end());
			textBoxVerts.insert(textBoxVerts.end(), obj.m_textBoxVerts.begin(), obj.m_textBoxVerts.end());
		}

		for (int messageIndex = (int) screenMessages.size() - 1; messageIndex >=0 ; messageIndex--)
		{
			DebugRenderingScreenObject& obj = screenMessages[messageIndex];
			std::vector<Vertex_PCU> objTextVerts = obj.m_textVerts;
			std::vector<Vertex_PCU> objTextBoxVerts = obj.m_textBoxVerts;
			Mat44 modelMat;
			Vec2 messageBottomLeft = Vec2(cursorLeft, cursorTop - obj.m_textHeight);
			cursorTop = cursorTop - obj.m_textHeight;
			modelMat = Mat44::CreateTranslation2D(messageBottomLeft);
			TransformVertexArray3D((int) objTextVerts.size(), objTextVerts.data(), modelMat);
			TransformVertexArray3D((int) objTextBoxVerts.size(), objTextBoxVerts.data(), modelMat);
			messageVerts.insert(messageVerts.end(), objTextVerts.begin(), objTextVerts.end());
			messageBoxBoxVerts.insert(messageBoxBoxVerts.end(), objTextBoxVerts.begin(), objTextBoxVerts.end());
		}

		Texture const& fontTexture = s_font->GetTexture();
		if ((int) textVerts.size() > 0)
		{
			//screen texts
			renderer->BindTexture(0, nullptr);
			renderer->DrawVertexArray((int) textBoxVerts.size(), textBoxVerts.data());

			renderer->BindTexture(0, &fontTexture);
			renderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
		}
	
		if ((int) messageVerts.size() > 0)
		{
			//screen messages
			renderer->BindTexture(0, nullptr);
			renderer->DrawVertexArray((int) messageBoxBoxVerts.size(), messageBoxBoxVerts.data());

			renderer->BindTexture(0, &fontTexture);
			renderer->DrawVertexArray((int) messageVerts.size(), messageVerts.data());
		}
		renderer->EndCamera(camera);
	}
	s_debugRenderDataMutex.unlock();
#endif
}


void DebugRenderEndFrame()
{
#if defined(ENABLE_ON_SCREEN_DEBUGGING)
	s_debugRenderDataMutex.lock();
	std::vector<DebugRenderingWorldObject>& worldObjects = s_debugRenderingData.m_worldObjects;
	for (int worldObjIndex = 0; worldObjIndex < (int) worldObjects.size(); worldObjIndex++)
	{
		DebugRenderingWorldObject& obj = worldObjects[worldObjIndex];
		if ( obj.m_timer.HasElapsed() && obj.m_duration >= 0.0f)
		{
			worldObjects[worldObjIndex] = worldObjects.back();
			worldObjects.pop_back();
			worldObjIndex--;
		}
	}

	std::vector<DebugRenderingScreenObject>& screenTexts = s_debugRenderingData.m_screenTexts;
	for (int textIndex = 0; textIndex < (int) screenTexts.size(); textIndex++)
	{
		DebugRenderingScreenObject& obj = screenTexts[textIndex];
		if (obj.m_timer.HasElapsed() && obj.m_duration >= 0.0f)
		{
			screenTexts[textIndex] = screenTexts.back();
			screenTexts.pop_back();
			textIndex--;
		}
	}

	std::vector<DebugRenderingScreenObject>& screenMessages = s_debugRenderingData.m_screenMessages;
	auto it = screenMessages.begin();
	while (it != screenMessages.end())
	{
		if (it->m_timer.HasElapsed() && it->m_duration >= 0.0f)
		{
			it = screenMessages.erase(it);
		}
		else
		{
			it++;
		}
	}
	s_debugRenderDataMutex.unlock();
#endif
}


void DebugAddWorldPoint(Vec3 const& pos, float size, Rgba8 const& startColor, Rgba8 const& endColor, float duration, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugRenderingWorldObject point = {};
	AABB3 pointCube = AABB3(pos, size, size, size);
	AddVertsForAABBZ3DToVector(point.m_objectVerts, pointCube, startColor);
	point.m_renderFillMode = FillMode::SOLID;
	point.m_renderMode = mode;
	point.m_duration = (double) duration;
	point.m_startColor = startColor;
	point.m_endColor = endColor;

	s_debugRenderDataMutex.lock();
	point.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(point);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldPoint(Vec3 const& pos, float size, Rgba8 const& color, float duration /*= 0.0f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldPoint(pos, size, color, color, duration, mode);
}


void DebugAddWorldPoint(Vec3 const& pos, Rgba8 const& color, float duration /*= 0.0f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldPoint(pos, 1.0f, color, color, duration, mode);
}


void DebugAddWorldLine(Vec3 const& start, Vec3 const& end, Rgba8 const& startColor, Rgba8 const& endColor, float thickness, float duration, DebugRenderMode mode)
{
	DebugRenderingWorldObject line = {};
	line.m_renderFillMode = FillMode::SOLID;
	line.m_renderMode = mode;
	line.m_duration = (double) duration;
	line.m_startColor = startColor;
	line.m_endColor = endColor;

	Vec3 lineUpVector = (end - start).GetNormalized();
	Vec3 lineCenter = (end + start) * 0.5f;
	float lineLength = (end - start).GetLength();
	float halfLineLength = lineLength * 0.5f;
	float lineRadius = thickness * 0.5f;

	Mat44 lineModelMatrix = GetModelMatrixFromUpVector(lineUpVector, lineCenter);
	AddVertsForCylinderZ3DToVector(line.m_objectVerts, Vec2(0.0f, 0.0f), FloatRange(-halfLineLength, halfLineLength), lineRadius, 4.0f, startColor);
	TransformVertexArray3D((int) line.m_objectVerts.size(), line.m_objectVerts.data(), lineModelMatrix);
	
	s_debugRenderDataMutex.lock();
	line.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(line);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldLine(Vec3 const& start, Vec3 const& end, Rgba8 const& color /*= Rgba8::WHITE*/, float thickness /*= 0.125f*/, float duration /*= 0.0f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldLine(start, end, color, color, thickness, duration, mode);
}


void DebugAddWorldWireCylinder(Vec3 const& base, Vec3 const& top, float radius, float duration, Rgba8 const& color, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldWireCylinder(base, top, radius, duration, color, color, mode);
}


void DebugAddWorldWireCylinder(Vec3 const& base, Vec3 const& top, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderingWorldObject cylinder = {};
	cylinder.m_renderFillMode = FillMode::WIREFRAME;
	cylinder.m_renderMode = mode;
	cylinder.m_duration = (double) duration;
	cylinder.m_startColor = startColor;
	cylinder.m_endColor = endColor;

	Vec3 cylinderBoneVec = top - base;
	Vec3 cylinderUpVec = cylinderBoneVec.GetNormalized();
	Vec3 cylinderCenter = (top + base) * 0.5f;
	float cylinderLength = cylinderBoneVec.GetLength();
	float halfCylinderLength = cylinderLength * 0.5f;

	Mat44 cylinderModelMatrix = GetModelMatrixFromUpVector(cylinderUpVec, cylinderCenter);
	AddVertsForCylinderZ3DToVector(cylinder.m_objectVerts, Vec2(0.0f, 0.0f), FloatRange(-halfCylinderLength, halfCylinderLength), radius, 16.0f, startColor);
	TransformVertexArray3D((int) cylinder.m_objectVerts.size(), cylinder.m_objectVerts.data(), cylinderModelMatrix);

	s_debugRenderDataMutex.lock();
	cylinder.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(cylinder);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldWireAABB3(AABB3 const& aabb3, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderingWorldObject aabb3Object = {};
	aabb3Object.m_renderFillMode = FillMode::WIREFRAME;
	aabb3Object.m_renderMode = mode;
	aabb3Object.m_duration = (double) duration;
	aabb3Object.m_startColor = startColor;
	aabb3Object.m_endColor = endColor;

	AddVertsForAABBZ3DToVector(aabb3Object.m_objectVerts, aabb3, startColor);

	s_debugRenderDataMutex.lock();
	aabb3Object.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(aabb3Object);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldWireAABB3(AABB3 const& aabb3, float duration /*= 0.0f*/, Rgba8 const& color /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugAddWorldWireAABB3(aabb3, duration, color, color, mode);
}


void DebugAddWorldWireSphere(Vec3 const& center, float radius, float duration, Rgba8 const& color, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldWireSphere(center, radius, duration, color, color, mode);
}


void DebugAddWorldWireSphere(Vec3 const& center, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderingWorldObject sphere = {};
	sphere.m_renderFillMode = FillMode::WIREFRAME;
	sphere.m_renderMode = mode;
	sphere.m_startColor = startColor;
	sphere.m_endColor = endColor;
	sphere.m_duration = duration;

	AddVertsForUVSphereZ3DToVector(sphere.m_objectVerts, center, radius, 16.0f, 8.0f, startColor);

	s_debugRenderDataMutex.lock();
	sphere.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(sphere);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldSolidSphere(Vec3 const& center, float radius, float duration /*= 0.0f*/, Rgba8 const& color /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugAddWorldSolidSphere(center, radius, duration, color, color, mode);
}


void DebugAddWorldSolidSphere(Vec3 const& center, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderingWorldObject sphere = {};
	sphere.m_renderFillMode = FillMode::SOLID;
	sphere.m_renderMode = mode;
	sphere.m_startColor = startColor;
	sphere.m_endColor = endColor;
	sphere.m_duration = duration;

	AddVertsForUVSphereZ3DToVector(sphere.m_objectVerts, center, radius, 32.0f, 12.0f, startColor);

	s_debugRenderDataMutex.lock();
	sphere.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(sphere);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldArrow(Vec3 const& start, Vec3 const& end, float duration /*= 0.0f*/, Rgba8 const& color /*= Rgba8::WHITE*/, float thickness /*= 0.125f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldArrow(start, end, color, color, duration, thickness, mode);
}


void DebugAddWorldArrow(Vec3 const& start, Vec3 const& end, Rgba8 const& startColor, Rgba8 const& endColor, float duration, float thickness, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugRenderingWorldObject arrow = {};
	arrow.m_renderFillMode = FillMode::SOLID;
	arrow.m_renderMode = mode;
	arrow.m_duration = (double) duration;
	arrow.m_startColor = startColor;
	arrow.m_endColor = endColor;

	Vec3 lineUpVector = (end - start).GetNormalized();
	Vec3 lineCenter = (end + start) * 0.5f;
	float lineLength = (end - start).GetLength();
	float halfLineLength = lineLength * 0.5f;
	float lineRadius = thickness * 0.5f;

	Mat44 lineModelMatrix = GetModelMatrixFromUpVector(lineUpVector, lineCenter);
	AddVertsForCylinderZ3DToVector(arrow.m_objectVerts, Vec2(0.0f, 0.0f), FloatRange(-halfLineLength, halfLineLength - ARROW_HEAD_HEIGHT), lineRadius, 4.0f, startColor);
	AddVertsForCylinderZ3DToVector(arrow.m_objectVerts, Vec2(0.0f, 0.0f), FloatRange(halfLineLength - ARROW_HEAD_HEIGHT, halfLineLength + ARROW_HEAD_HEIGHT),
								   lineRadius * ARROW_HEAD_BASE_RADIUS_MULTIPLIER, ARROW_HEAD_TOP_RADIUS, 4.0f, startColor);
	TransformVertexArray3D((int) arrow.m_objectVerts.size(), arrow.m_objectVerts.data(), lineModelMatrix);

	s_debugRenderDataMutex.lock();
	arrow.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(arrow);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldBasis(Mat44 const& basis, Rgba8 const& startTint, Rgba8 const& endTint, float duration, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	Vec3 basisStartPoint = basis.GetTranslation3D();
	Vec3 upVec = basis.GetKBasis3D();
	Vec3 upArrowEndPoint = basisStartPoint + upVec * BASIS_ARROW_LENGTH;
	Rgba8 upArrowStartColor = Rgba8::BLUE * startTint;
	Rgba8 upArrowEndColor = Rgba8::BLUE * endTint;
	DebugAddWorldArrow(basisStartPoint, upArrowEndPoint, upArrowStartColor, upArrowEndColor, duration, 0.125f, mode);

	Vec3 forwardVec = basis.GetIBasis3D();
	Vec3 forwardVecEndPoint = basisStartPoint + forwardVec * BASIS_ARROW_LENGTH;
	Rgba8 forwardArrowStartColor = Rgba8::RED * startTint;
	Rgba8 forwardArrowEndColor = Rgba8::RED * endTint;
	DebugAddWorldArrow(basisStartPoint, forwardVecEndPoint, forwardArrowStartColor, forwardArrowEndColor, duration, 0.125f, mode);

	Vec3 leftVec = basis.GetJBasis3D();
	Vec3 leftVecEndPoint = basisStartPoint + leftVec * BASIS_ARROW_LENGTH;
	Rgba8 leftArrowStartColor = Rgba8::GREEN * startTint;
	Rgba8 leftArrowEndColor = Rgba8::GREEN * endTint;
	DebugAddWorldArrow(basisStartPoint, leftVecEndPoint, leftArrowStartColor, leftArrowEndColor, duration, 0.125f, mode);
}


void DebugAddWorldBasis(Mat44 const& basis, float duration /*= 0.0f*/, Rgba8 const& tint /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldBasis(basis, tint, tint, duration, mode);
}


void DebugAddWorldText(std::string const& text, float textHeight, Vec2 const& alignment, Rgba8 const& startColor, Rgba8 const& endColor, Mat44 const& transform /*= Mat44()*/, float duration /*= 0.0f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugRenderingWorldObject worldText = {};
	float aspect = 1.0f;
	float charWidth = aspect * textHeight;
	float textWidth = text.length() * charWidth;
	Vec2 textBoxMins = Vec2(-alignment.x * textWidth, -alignment.y * textHeight);
	Vec2 textBoxMaxs = Vec2((1 - alignment.x) * textWidth, (1 - alignment.y) * textHeight);
	AABB2 textBox = AABB2(textBoxMins, textBoxMaxs);
	if (s_font)
		s_font->AddVertsForTextInAABB2(worldText.m_objectVerts, textBox, textHeight, text, startColor, aspect, BitmapFont::ALIGNED_BOTTOM_LEFT, TextDrawMode::SHRINK_TO_FIT);
	
	Mat44 orientToWorldTransform = Mat44(Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f));
	Mat44 textTransform = transform;
	textTransform.Append(orientToWorldTransform);
	TransformVertexArray3D((int) worldText.m_objectVerts.size(), worldText.m_objectVerts.data(), textTransform);

	worldText.m_duration = duration;
	worldText.m_endColor = endColor;
	worldText.m_startColor = startColor;
	worldText.m_renderFillMode = FillMode::SOLID;
	worldText.m_renderMode = mode;
	worldText.m_isText = true;

	s_debugRenderDataMutex.lock();
	worldText.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(worldText);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldText(std::string const& text, float textHeight /*= 0.25f*/, Vec2 const& alignment /*= Vec2(0.5f, 0.5f)*/, Rgba8 const& color /*= Rgba8::WHITE*/, Mat44 const& transform /*= Mat44()*/, float duration /*= 0.0f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldText(text, textHeight, alignment, color, color, transform, duration, mode);
}


void DebugAddWorldBillboardText(Vec3 const& origin, std::string const& text, float textHeight, Vec2 const& alignment, Rgba8 const& startcolor, Rgba8 const& endColor, float duration /*= 0.0f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugRenderingWorldObject billboardedWorldText = {};
	billboardedWorldText.m_duration = duration;
	billboardedWorldText.m_endColor = endColor;
	billboardedWorldText.m_startColor = startcolor;
	billboardedWorldText.m_renderFillMode = FillMode::SOLID;
	billboardedWorldText.m_renderMode = mode;
	billboardedWorldText.m_isText = true;
	billboardedWorldText.m_isBillboarded = true;
	billboardedWorldText.m_billboardOrigin = origin;

	float aspect = 1.0f;
	float charWidth = aspect * textHeight;
	float textWidth = text.length() * charWidth;
	Vec2 textBoxMins = Vec2(-alignment.x * textWidth, -alignment.y * textHeight);
	Vec2 textBoxMaxs = Vec2((1 - alignment.x) * textWidth, (1 - alignment.y) * textHeight);
	AABB2 textBox = AABB2(textBoxMins, textBoxMaxs);
	if (s_font)
		s_font->AddVertsForTextInAABB2(billboardedWorldText.m_objectVerts, textBox, textHeight, text, startcolor, aspect, BitmapFont::ALIGNED_BOTTOM_LEFT, TextDrawMode::SHRINK_TO_FIT);

	s_debugRenderDataMutex.lock();
	billboardedWorldText.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_worldObjects.push_back(billboardedWorldText);
	s_debugRenderDataMutex.unlock();
}


void DebugAddWorldBillboardText(Vec3 const& origin, std::string const& text, float textHeight /*= 0.5f*/, Vec2 const& alignment /*= Vec2(0.5f, 0.5f)*/, Rgba8 const& color /*= Rgba8::WHITE*/, float duration /*= 0.0f*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */)
{
	DebugAddWorldBillboardText(origin, text, textHeight, alignment, color, color, duration, mode);
}


void DebugAddScreenText(std::string const& text, Vec2 const& position, float duration, Vec2 const& alignment, float textHeight, Rgba8 const& startColor, Rgba8 const& endColor)
{
	DebugRenderingScreenObject obj = {};
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_duration = duration;
	
	float aspect = 1.0f;
	float charWidth = aspect * textHeight;
	float textWidth = text.length() * charWidth;
	float textBoxHeight = textHeight * 1.3f;
	Vec2 textBoxMins = Vec2(-alignment.x * textWidth, -alignment.y * textBoxHeight);
	Vec2 textBoxMaxs = Vec2((1 - alignment.x) * textWidth, (1 - alignment.y) * textBoxHeight);
	AABB2 textBox = AABB2(textBoxMins, textBoxMaxs);
	textBox.Translate(position);
	AddVertsForAABB2ToVector(obj.m_textBoxVerts, textBox, Rgba8::BLACK);
	if (s_font)
		s_font->AddVertsForTextInAABB2(obj.m_textVerts, textBox, textHeight, text, startColor, aspect, BitmapFont::ALIGNED_BOTTOM_LEFT, TextDrawMode::OVERRUN);

	s_debugRenderDataMutex.lock();
	obj.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_screenTexts.push_back(obj);
	s_debugRenderDataMutex.unlock();
}


void DebugAddScreenText(std::string const& text, Vec2 const& position, float duration /*= 0.0f*/, Vec2 const& alignment /*= Vec2(0.0f, 0.0f)*/, float textHeight /*= 32.0f*/, Rgba8 const& color /*= Rgba8::WHITE */)
{
	DebugAddScreenText(text, position, duration, alignment, textHeight, color, color);
}


void DebugAddScreenMessage(std::string const& text, float duration, float textHeight, Rgba8 const& startColor, Rgba8 const& endColor)
{
	DebugRenderingScreenObject obj = {};
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_duration = duration;
	obj.m_textHeight = textHeight;

	float aspect = 1.0f;
	float charWidth = aspect * textHeight;
	Rgba8 textBoxColor = Rgba8(113, 58, 135, 210);
	Vec2 textBoxMins = Vec2(0.0f, 0.0f);
	Vec2 textBoxMaxs = Vec2((float) text.length() * charWidth, textHeight * 1.2f);
	AABB2 textBox = AABB2(textBoxMins, textBoxMaxs);
	AddVertsForAABB2ToVector(obj.m_textBoxVerts, textBox, textBoxColor);
	if (s_font)
		s_font->AddVertsForTextInAABB2(obj.m_textVerts, textBox, textHeight, text, startColor, aspect, BitmapFont::ALIGNED_BOTTOM_LEFT, TextDrawMode::SHRINK_TO_FIT);

	s_debugRenderDataMutex.lock();
	obj.m_timer = Stopwatch(s_debugRenderingClock, (double) duration);
	s_debugRenderingData.m_screenMessages.push_back(obj);
	s_debugRenderDataMutex.unlock();
}


void DebugAddScreenMessage(std::string const& text, float duration /*= 0.0f*/, float textHeight, Rgba8 const& color /*= Rgba8::WHITE */)
{
	DebugAddScreenMessage(text, duration, textHeight, color, color);
}
