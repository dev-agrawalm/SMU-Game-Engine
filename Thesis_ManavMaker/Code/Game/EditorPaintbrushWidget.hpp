#pragma once
#include <string>
#include <vector>
#include "Game/Entity.hpp"
#include "Game/MouseData.hpp"
#include "Game/UIButton.hpp"

class Camera;

class EditorPaintbrushWidget : public UIWidget
{
public:
	EditorPaintbrushWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs);
	~EditorPaintbrushWidget();
	bool CheckInput(MouseData* out_mouseData);
	virtual void Render() const override;
	void ShowSavedWidget();

private:
	void AssembleWidget();

private:
	std::vector<UIButton*> m_buttons;
	UIButton* m_saveButton = nullptr;
	UIWidget* m_levelSavedTextWidget = nullptr;
};
