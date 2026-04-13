#include "Game/UIWidget.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Time.hpp"

UIWidget::UIWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs, Rgba8 const& color /*= Rgba8::WHITE*/, SpriteDefinition const* imageSprite /*= nullptr*/)
	: m_renderCam(renderCam)
	, m_camSpaceMins(camSpaceMins)
	, m_camSpaceMaxs(camSpaceMaxs)
	, m_color(color)
	, m_imageSprite(imageSprite)
{

}


UIWidget::~UIWidget()
{
	m_imageSprite = nullptr;
}


void UIWidget::Render() const
{
	if (m_isHidden)
		return;

	unsigned char alpha = m_color.a;
	static float originalAlpha = ByteToFloatNormalised(alpha);
	if (m_fadeoutDuration > 0.0f)
	{
		float currentTime = (float) GetCurrentTimeSeconds();
		float alphaFloat = RangeMapClamped(currentTime, m_fadeoutStartTime, m_fadeoutStartTime + m_fadeoutDuration, originalAlpha, 0.0f);
		alpha = FloatToByte(alphaFloat);
	}

	if (alpha == 0)
		return;

	AABB2 camOrtho = m_renderCam.GetOrthoCamBoundingBox();
	AABB2 widgetBounds = AABB2(camOrtho.m_mins + m_camSpaceMins, camOrtho.m_mins + m_camSpaceMaxs);
	std::vector<Vertex_PCU> widgetVerts;
	Vec2 uvMins = Vec2::ZERO;
	Vec2 uvMaxs = Vec2(1.0f, 1.0f);
	Texture const* texture = nullptr;
	if (m_imageSprite)
	{
		m_imageSprite->GetUVs(uvMins, uvMaxs);
		texture = &m_imageSprite->GetTexture();
	}
	Rgba8 widgetColor = m_color;
	widgetColor.a = alpha;
	AddVertsForAABB2ToVector(widgetVerts, widgetBounds, widgetColor, uvMins, uvMaxs);
	g_theRenderer->BindTexture(0, texture);
	g_theRenderer->DrawVertexArray((int) widgetVerts.size(), widgetVerts.data());

	RenderText(widgetBounds, alpha, BitmapFont::ALIGNED_CENTER);
}


void UIWidget::RenderText(AABB2 const& widgetBounds, unsigned char alpha, Vec2 const& textAlignment) const
{
	if (!m_text.empty() && m_showText)
	{
		std::vector<Vertex_PCU> textVerts;
		Vec2 widgetDims = widgetBounds.GetDimensions();
		float cellHeight = widgetDims.y * 0.9f;
		Rgba8 textColor = m_textColor;
		textColor.a = alpha;
		
		g_bitmapFont->AddVertsForTextInAABB2(textVerts, widgetBounds, cellHeight, m_text, textColor, 1.0f, textAlignment);
		g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
	}
}


bool UIWidget::IsMouseOverWidget() const
{
	AABB2 camOrtho = m_renderCam.GetOrthoCamBoundingBox();
	Vec2 camOrthoDims = camOrtho.GetDimensions();
	Vec2 windowDims = g_window->GetWindowDims();
	IntVec2 mousePos = g_inputSystem->GetMousePosition();
	Vec2 mousePosCamSpace = RangeMap(mousePos.GetVec2(), Vec2::ZERO, windowDims, Vec2(0.0f, camOrthoDims.y), Vec2(camOrthoDims.x, 0.0f));

	AABB2 buttonCamSpaceBounds = AABB2(m_camSpaceMins, m_camSpaceMaxs);
	return IsPointInsideAABB2D(mousePosCamSpace, buttonCamSpaceBounds);
}


Vec2 UIWidget::GetDimensions() const
{
	return m_camSpaceMaxs - m_camSpaceMins;
}


void UIWidget::SetColor(Rgba8 const& color)
{
	m_color = color;
}


void UIWidget::SetImage(SpriteDefinition const* imageSprite)
{
	m_imageSprite = imageSprite;
}


void UIWidget::SetText(std::string const& buttonText)
{
	m_text = buttonText;
}


void UIWidget::SetToolTipText(std::string const& tooltipText)
{
	m_toolTip = tooltipText;
}


void UIWidget::SetShowText(bool showText)
{
	m_showText = showText;
}


void UIWidget::SetFadeoutDuration(float duration)
{
	m_fadeoutDuration = duration;
}


void UIWidget::SetFadeoutStartTime(float startTime)
{
	m_fadeoutStartTime = startTime;
}


void UIWidget::SetTextColor(Rgba8 const& textColor)
{
	m_textColor = textColor;
}


void UIWidget::SetHidden(bool isHidden)
{
	m_isHidden = isHidden;
}


void UIWidget::SetPreserverImageAspect(float shouldPreserve)
{
	m_preserveImageAspect = shouldPreserve;
}


bool UIWidget::IsHidden() const
{
	return m_isHidden;
}


std::string UIWidget::GetText() const
{
	return m_text;
}


std::string UIWidget::GetToolTipText() const
{
	return m_toolTip;
}


AABB2 UIWidget::GetWidgetBounds() const
{
	AABB2 camOrtho = m_renderCam.GetOrthoCamBoundingBox();
	return AABB2(camOrtho.m_mins + m_camSpaceMins, camOrtho.m_mins + m_camSpaceMaxs);
}
