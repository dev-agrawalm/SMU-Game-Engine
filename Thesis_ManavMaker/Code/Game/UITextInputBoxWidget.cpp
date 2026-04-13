#include "Game/UITextInputBoxWidget.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

constexpr double CARAT_BLINK_RATE = 0.3;

std::string UITextInputBoxWidget::s_textBoxText = "";

bool OnCharDownEvent(EventArgs& args)
{
// 	unsigned char input = args.GetValue("InputKey", '\0');
// 	if (input >= 32 && input <= 125)
// 	{
// 		std::string character(1, input);
// 		UITextInputBoxWidget::s_textBoxText.insert(m_caratPos, character);
// 		m_caratPos++;
// 	}
	UNUSED(args);
	return false;
}


UITextInputBoxWidget::UITextInputBoxWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs)
	: UIWidget(renderCam, camSpaceMins, camSpaceMaxs)
{
	m_caratBlinkTimer = Stopwatch(Clock::GetSystemClock(), CARAT_BLINK_RATE);
	g_eventSystem->SubscribeEventCallbackFunction("CharDown", OnCharDownEvent);
}


void UITextInputBoxWidget::Update()
{
	if (m_caratBlinkTimer.CheckAndRestart())
	{
		m_showCarat = !m_showCarat;
	}
}


bool UITextInputBoxWidget::CheckInput()
{
	return false;
}
