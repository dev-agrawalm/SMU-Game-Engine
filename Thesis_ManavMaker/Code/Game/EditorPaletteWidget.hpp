#pragma once
#include <string>
#include <vector>
#include "Game/Entity.hpp"
#include "Game/MouseData.hpp"
#include "Game/UIButton.hpp"

class Camera;

class EditorPaletteWidget : public UIWidget
{
public:
	EditorPaletteWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs);
	~EditorPaletteWidget();
	bool CheckInput(MouseData* out_mouseData);
	virtual void Render() const override;
	void ShowSavedTextWidget();

private:
	void AssembleWidget();

private:
	std::vector<UIButton*> m_buttons;
	UIButton* m_saveButton = nullptr;
	UIWidget* m_buttonNameWidget = nullptr;
	UIWidget* m_buttonTooltipWidget = nullptr;
	UIWidget* m_levelSavedTextWidget = nullptr;
	Vec2 m_saveButtonDims;
};
