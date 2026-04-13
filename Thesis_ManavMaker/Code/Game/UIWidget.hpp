#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

class Camera;
class SpriteDefinition;

struct AABB2;

class UIWidget
{
public:
	UIWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs, Rgba8 const& color = Rgba8::WHITE, SpriteDefinition const* imageSprite = nullptr);
	virtual ~UIWidget();
	virtual void Render() const;

	bool IsMouseOverWidget() const;
	Vec2 GetDimensions() const;

	void SetColor(Rgba8 const& color);
	void SetImage(SpriteDefinition const* imageSprite);
	void SetText(std::string const& buttonText);
	void SetToolTipText(std::string const& tooltipText);
	void SetShowText(bool showText);
	void SetFadeoutDuration(float duration);
	void SetFadeoutStartTime(float startTime);
	void SetTextColor(Rgba8 const& textColor);
	void SetHidden(bool isHidden);
	void SetPreserverImageAspect(float shouldPreserve);

	bool IsHidden() const;
	std::string GetText() const;
	std::string GetToolTipText() const;
	AABB2 GetWidgetBounds() const;

protected:
	void RenderText(AABB2 const& widgetBounds, unsigned char alpha, Vec2 const& textAlignment) const;

protected:
	Vec2 m_camSpaceMins;
	Vec2 m_camSpaceMaxs;
	Camera const& m_renderCam;
	Rgba8 m_color = Rgba8::WHITE;
	SpriteDefinition const* m_imageSprite = nullptr;
	bool m_preserveImageAspect = true;

	std::string m_text = "";
	std::string m_toolTip = "";
	bool m_showText = true;
	Rgba8 m_textColor = Rgba8::WHITE;

	float m_fadeoutStartTime = -1.0f;
	float m_fadeoutDuration = -1.0f;

	bool m_isHidden = false;
};
