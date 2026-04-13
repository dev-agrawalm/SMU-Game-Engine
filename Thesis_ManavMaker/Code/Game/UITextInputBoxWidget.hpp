#pragma once
#include "Game/UIWidget.hpp"
#include "Engine/Core/Stopwatch.hpp"

class UITextInputBoxWidget : public UIWidget
{
public:
	UITextInputBoxWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs);
	void Update();
	bool CheckInput();

private:
	Stopwatch m_caratBlinkTimer;
	bool m_showCarat = false;
	std::string m_text;
};
