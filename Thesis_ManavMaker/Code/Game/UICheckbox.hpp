#pragma once
#include "Game/UIButton.hpp"

class UICheckbox : public UIButton
{
public:
	UICheckbox(Camera const& camera, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs, bool isChecked = false);
	virtual void Render() const override;

	void SetCheckedState(bool isChecked);
	void ToggleCheckedState();

	bool IsChecked() const;

protected:
	virtual bool OnButtonClicked(ButtonCallbackArgs& args);

private:
	bool m_checked = false;
	Rgba8 m_checkedTint = Rgba8::RED;
	Rgba8 m_uncheckedTint = Rgba8::GREY;
};