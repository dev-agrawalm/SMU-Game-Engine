#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

static Clock g_systemClock;

constexpr float FRAME_TIME_60_FPS = (1.0f / 60.0f);
constexpr float FRAME_TIME_10_FPS = (1.0f / 10.0f);

Clock::Clock()
{
	Clock& systemClock = GetSystemClock();
	if (this != &systemClock)
	{
		systemClock.AddChild(this);
	}
}


Clock::Clock(Clock* parent)
{
	m_parent = parent;
	m_parent->AddChild(this);
}


void Clock::SetParent(Clock& parent)
{
	m_parent->RemoveChild(this);
	parent.AddChild(this);
}


void Clock::Pause()
{
	m_isPaused = true;
}


void Clock::Unpause()
{
	m_isPaused = false;
	m_pauseAfterFrame = false;
}


void Clock::TogglePause()
{
	if (m_isPaused)
	{
		Unpause();
	}
	else
	{
		Pause();
	}
}


void Clock::StepFrame()
{
	m_isPaused = false;
	m_pauseAfterFrame = true;
}


void Clock::SetTimeScale(double scale)
{
	m_scale = scale;
}


void Clock::Reset(bool resetChildren /*= true */)
{
	m_totalSecondsPassed = 0.0f;
	m_frameDeltaSeconds = 0.0f;
	m_frameCount = 0;

	if (resetChildren)
	{
		for (int childClockIndex = 0; childClockIndex < (int) m_children.size(); childClockIndex++)
		{
			Clock*& child = m_children[childClockIndex];
			child->Reset(true);
		}
	}
}


void Clock::SystemBeginFrame()
{
	g_systemClock.Tick();
}


Clock& Clock::GetSystemClock()
{
	return g_systemClock;
}


void Clock::Tick()
{
	double currentTime = GetCurrentTimeSeconds();
	double deltaTime = currentTime - m_lastUpdateTime;
	AdvanceTime(deltaTime);
	m_lastUpdateTime = currentTime;
}


void Clock::AdvanceTime(double deltaTimeSeconds)
{
	deltaTimeSeconds *= m_scale;
	if (IsPaused())
	{
		deltaTimeSeconds = 0.0f;
	}

	m_frameDeltaSeconds = deltaTimeSeconds;
	m_totalSecondsPassed += deltaTimeSeconds;
	m_frameCount++;

	if (m_pauseAfterFrame)
	{
		m_isPaused = true;
	}

	for (int childClockIndex = 0; childClockIndex < (int) m_children.size(); childClockIndex++)
	{
		Clock*& child = m_children[childClockIndex];
		child->AdvanceTime(deltaTimeSeconds);
	}
}


void Clock::AddChild(Clock* childClock)
{
	childClock->m_parent = this;
	m_children.push_back(childClock);
}


void Clock::RemoveChild(Clock* childClock)
{
	for (int childClockIndex = 0; childClockIndex < (int) m_children.size(); childClockIndex++)
	{
		Clock*& child = m_children[childClockIndex];
		if (child == childClock)
		{
			Clock*& lastChild = m_children.back();
			m_children[childClockIndex] = lastChild;
			m_children.pop_back();
			break;
		}
	}
}


Clock::~Clock()
{
	if (m_parent)
	{
		m_parent->RemoveChild(this);
		m_parent = nullptr;
	}
	m_children.clear();
}
