#include "Game/UICheckbox.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

UICheckbox::UICheckbox(Camera const& camera, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs, bool isChecked /*= false*/)
	: UIButton(camera, camSpaceMins, camSpaceMaxs)
	, m_checked(isChecked)
{
	ButtonCallbackMethod<UICheckbox>* callbackMethod = new ButtonCallbackMethod(*this, &UICheckbox::OnButtonClicked);
	m_onClickCallback = callbackMethod;
	SetCheckedState(isChecked);
}


void UICheckbox::Render() const
{
	if (m_isHidden)
		return;

	AABB2 camOrtho = m_renderCam.GetOrthoCamBoundingBox();
	AABB2 buttonCosmeticBounds = AABB2(camOrtho.m_mins + m_camSpaceMins, camOrtho.m_mins + m_camSpaceMaxs);
	Vec2 buttonDims = buttonCosmeticBounds.GetDimensions();
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

	if (!m_text.empty() && m_showText)
	{
		std::vector<Vertex_PCU> textVerts;
		AABB2 textBounds = buttonCosmeticBounds;
		textBounds.Translate(Vec2(buttonDims.x * 1.1f, -10.0f));
		textBounds.m_maxs.x += 150.0f;
		Vec2 widgetDims = textBounds.GetDimensions();
		float cellHeight = widgetDims.y * 4.0f;
		Rgba8 textColor = m_textColor;
		textColor.a = 255;

		g_bitmapFont->AddVertsForTextInAABB2(textVerts, textBounds, cellHeight, m_text, textColor, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);
		g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
	}
}


void UICheckbox::SetCheckedState(bool isChecked)
{
	m_checked = isChecked;
	if (m_checked)
	{
		m_color = m_checkedTint;
	}
	else
	{
		m_color = m_uncheckedTint;
	}
}


void UICheckbox::ToggleCheckedState()
{
	SetCheckedState(!m_checked);
}


bool UICheckbox::IsChecked() const
{
	return m_checked;
}


bool UICheckbox::OnButtonClicked(ButtonCallbackArgs& args)
{
	UNUSED(args);
	ToggleCheckedState();
	return false;
}

