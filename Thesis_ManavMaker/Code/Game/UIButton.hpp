#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/UIWidget.hpp"

class NamedProperties;
typedef NamedProperties ButtonCallbackArgs;
typedef bool (*ButtonCallbackFunctPtr) (ButtonCallbackArgs& args);

struct ButtonCallbackBase
{
public:
	virtual ~ButtonCallbackBase() {};
	virtual bool Execute(ButtonCallbackArgs& args) = 0;

};


struct ButtonCallbackFunction : public ButtonCallbackBase
{
public:
	virtual bool Execute(ButtonCallbackArgs& args) override { return m_funcPtr(args); }

public:
	ButtonCallbackFunctPtr m_funcPtr = nullptr;
};


template <typename T_ObjectType>
struct ButtonCallbackMethod : public ButtonCallbackBase
{
public:
	typedef bool (T_ObjectType::*ButtonCallbackMethodPtr) (ButtonCallbackArgs& args);
	ButtonCallbackMethod(T_ObjectType& object, ButtonCallbackMethodPtr objectMethod)
		: m_object(object)
		, m_objectMethod(objectMethod)
	{
	}
	virtual bool Execute(ButtonCallbackArgs& args) override;

public:
	T_ObjectType& m_object;
	ButtonCallbackMethodPtr m_objectMethod = nullptr;
};


template <typename T_ObjectType>
bool ButtonCallbackMethod<T_ObjectType>::Execute(ButtonCallbackArgs& args)
{
	return (m_object.*m_objectMethod)(args);
}


class UIButton : public UIWidget
{
public:
	static UIButton* CreateButton(Camera const& camera, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs,
		ButtonCallbackBase* onClickCallback = nullptr, Rgba8 const& color = Rgba8::WHITE, SpriteDefinition const* imageSprite = nullptr,
		std::string const& buttonText = "", std::string const& tooltipText = "");
public:
	UIButton(Camera const& camera, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs, ButtonCallbackBase* onClickCallback = nullptr);
	virtual ~UIButton();
	virtual void Render() const override;
	bool OnClick(ButtonCallbackArgs& args);
	bool OnClick();

	void SetHoveredOnFrame(unsigned int frameIndex);
	void SetHoverImage(SpriteDefinition const* imageSprite);
	void SetHoverTint(Rgba8 const& hoverTint);

	void SetClickedOnFrame(unsigned int frameIndex);
	void SetClickedImage(SpriteDefinition const* imageSprite);
	void SetClickedTint(Rgba8 const& clickedTint);

	void SetBorderColor(Rgba8 const& color);

protected:
	ButtonCallbackBase* m_onClickCallback = nullptr;

	SpriteDefinition const* m_hoverSprite = nullptr;
	Rgba8 m_hoverTint = Rgba8(200, 200, 200, 255);
	unsigned int m_hoveredOnFrame = 0;

	SpriteDefinition const* m_clickedSprite = nullptr;
	Rgba8 m_clickedTint = Rgba8(150, 150, 150, 255);
	unsigned int m_clickedOnFrame = 0;

	Rgba8 m_borderColor = Rgba8(0, 0, 0, 0);
};
