#include "Game/UIButton.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

UIButton* UIButton::CreateButton(Camera const& camera, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs, ButtonCallbackBase* onClickCallback /*= nullptr*/, Rgba8 const& color /*= Rgba8::WHITE*/, SpriteDefinition const* imageSprite /*= nullptr*/, std::string const& buttonText /*= ""*/, std::string const& tooltipText /*= ""*/)
{
	UIButton* button = new UIButton(camera, camSpaceMins, camSpaceMaxs, onClickCallback);
	button->SetColor(color);
	button->SetImage(imageSprite);
	button->SetText(buttonText);
	button->SetToolTipText(tooltipText);
	return button;
}


UIButton::UIButton(Camera const& camera, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs, ButtonCallbackBase* onClickCallback)
	: UIWidget(camera, camSpaceMins, camSpaceMaxs)
	, m_onClickCallback(onClickCallback)
{
}


UIButton::~UIButton()
{

}


void UIButton::Render() const
{
	if (m_isHidden)
		return;

	AABB2 camOrtho = m_renderCam.GetOrthoCamBoundingBox();
	AABB2 buttonCosmeticBounds = AABB2(camOrtho.m_mins + m_camSpaceMins, camOrtho.m_mins + m_camSpaceMaxs);
	std::vector<Vertex_PCU> widgetVerts;
	Vec2 uvMins = Vec2::ZERO;
	Vec2 uvMaxs = Vec2(1.0f, 1.0f);
	Texture const* texture = nullptr;
	if (m_imageSprite)
	{
		m_imageSprite->GetUVs(uvMins, uvMaxs);
		if (m_preserveImageAspect)
		{
			float imageAspect = m_imageSprite->GetAspect();
			Vec2 buttonBoundsDims = buttonCosmeticBounds.GetDimensions();
			float buttonBoundsAspect = buttonBoundsDims.x / buttonBoundsDims.y;
			if (imageAspect > buttonBoundsAspect)
			{
				//image is wider than the button; make button wider
				//float scale = imageAspect / buttonBoundsAspect;
				buttonCosmeticBounds.SetDimensions(buttonBoundsDims.y * imageAspect, buttonBoundsDims.y);
			}
			else
			{
				//image is taller than button; make button taller
				//float scale = buttonBoundsAspect / imageAspect;
				buttonCosmeticBounds.SetDimensions(buttonBoundsDims.x, buttonBoundsDims.x / imageAspect);
			}
		}
		texture = &m_imageSprite->GetTexture();
	}
	Rgba8 tint = m_color;
	if (m_hoveredOnFrame == g_currentFrameNumber)
	{
		tint *= m_hoverTint;
		if (m_hoverSprite)
		{
			m_hoverSprite->GetUVs(uvMins, uvMaxs);
			texture = &m_hoverSprite->GetTexture();
		}
	}

	if (m_clickedOnFrame == g_currentFrameNumber)
	{
		tint *= m_clickedTint;
		if (m_clickedSprite)
		{
			m_clickedSprite->GetUVs(uvMins, uvMaxs);
			texture = &m_clickedSprite->GetTexture();
		}
	}

	AddVertsForAABB2ToVector(widgetVerts, buttonCosmeticBounds, tint, uvMins, uvMaxs);
	g_theRenderer->BindTexture(0, texture);
	g_theRenderer->DrawVertexArray((int) widgetVerts.size(), widgetVerts.data());

	RenderText(buttonCosmeticBounds, 255, BitmapFont::ALIGNED_BOTTOM_CENTER);

	if (m_borderColor.a != 0)
	{
		static float borderWidth = g_gameConfigBlackboard.GetValue("paintBrushButtonBorderWidth", 0.0f);
		DebugDrawAABB2Outline(buttonCosmeticBounds, borderWidth, m_borderColor);
	}
}


bool UIButton::OnClick(ButtonCallbackArgs& args)
{
	return m_onClickCallback->Execute(args);
}


bool UIButton::OnClick()
{
	ButtonCallbackArgs args;
	return OnClick(args);
}


void UIButton::SetHoverImage(SpriteDefinition const* imageSprite)
{
	m_hoverSprite = imageSprite;
}


void UIButton::SetHoverTint(Rgba8 const& hoverTint)
{
	m_hoverTint = hoverTint;
}


void UIButton::SetClickedImage(SpriteDefinition const* imageSprite)
{
	m_clickedSprite = imageSprite;
}


void UIButton::SetClickedTint(Rgba8 const& clickedTint)
{
	m_clickedTint = clickedTint;
}


void UIButton::SetHoveredOnFrame(unsigned int frameIndex)
{
	m_hoveredOnFrame = frameIndex;
}


void UIButton::SetClickedOnFrame(unsigned int frameIndex)
{
	m_clickedOnFrame = frameIndex;
}


void UIButton::SetBorderColor(Rgba8 const& color)
{
	m_borderColor = color;
}

