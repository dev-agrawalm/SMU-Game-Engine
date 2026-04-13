#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"

Stopwatch::Stopwatch()
{
	Clock& systemClock = Clock::GetSystemClock();
	m_sourceClock = &systemClock;
	m_isStopped = true;
}


Stopwatch::Stopwatch(double secondDuration)
{
	Clock& systemClock = Clock::GetSystemClock();
	m_sourceClock = &systemClock;
	Start(secondDuration);
}


Stopwatch::Stopwatch(Clock const& sourceClock, double secondDuration)
{
	Start(sourceClock, secondDuration);
}


void Stopwatch::Start(Clock const& sourceClock, double seconds)
{
	m_sourceClock = &sourceClock;
	m_startTime = sourceClock.GetTotalSeconds();
	m_duration = seconds;
}


void Stopwatch::Start(double seconds)
{
	m_isPaused = false;
	m_isStopped = false;
	m_startTime = m_sourceClock->GetTotalSeconds();
	m_duration = seconds;
}


void Stopwatch::SetClock(Clock const& sourceClock)
{
	m_sourceClock = &sourceClock;
}


void Stopwatch::Restart()
{
	m_isPaused = false;
	m_isStopped = false;
	m_startTime = m_sourceClock->GetTotalSeconds();
}


void Stopwatch::Stop()
{
	m_isStopped = true;
	m_timeWhenStopped = m_sourceClock->GetTotalSeconds();
	Restart();
}


double Stopwatch::GetDuration() const
{
	return m_duration;
}


double Stopwatch::GetElapsedSeconds() const
{
	double elapsedTime = m_sourceClock->GetTotalSeconds() - m_startTime;
	if (IsStopped() || IsPaused())
	{
		elapsedTime = m_timeWhenStopped - m_startTime;
	}
	return elapsedTime;
}


float Stopwatch::GetElapsedFraction() const
{
	double elapsedTime = m_sourceClock->GetTotalSeconds() - m_startTime;
	if (IsStopped() || IsPaused())
	{
		elapsedTime = m_timeWhenStopped - m_startTime;
	}
	float fraction = (float) (elapsedTime / m_duration);
	return fraction;
}


bool Stopwatch::IsStopped() const
{
	return m_isStopped;
}


bool Stopwatch::HasElapsed() const
{
	double elapsedTime = m_sourceClock->GetTotalSeconds() - m_startTime;
	if (IsStopped() || IsPaused())
	{
		elapsedTime = m_timeWhenStopped - m_startTime;
	}
	return elapsedTime >= m_duration;
}


bool Stopwatch::CheckAndDecrement()
{
	double elapsedTime = m_sourceClock->GetTotalSeconds() - m_startTime;
	if (IsStopped() || IsPaused())
	{
		elapsedTime = m_timeWhenStopped - m_startTime;
	}
	bool hasElapsed = elapsedTime >= m_duration;
	if (hasElapsed)
	{
		m_startTime += m_duration;
	}
	return hasElapsed;
}


bool Stopwatch::CheckAndRestart()
{
	double elapsedTime = m_sourceClock->GetTotalSeconds() - m_startTime;
	if (IsStopped() || IsPaused())
	{
		elapsedTime = m_timeWhenStopped - m_startTime;
	}
	bool hasElapsed = elapsedTime >= m_duration;
	if (hasElapsed)
	{
		Restart();
	}
	return hasElapsed;
}


int Stopwatch::DecrementAllAndRestart()
{
	float elapsedFraction = GetElapsedFraction();
	Restart();
	return RoundDownToInt(elapsedFraction);
}


int Stopwatch::DecrementAllAndPause()
{
	float elapsedFraction = GetElapsedFraction();
	m_startTime = m_sourceClock->GetTotalSeconds();
	Pause();
	return RoundDownToInt(elapsedFraction);
}


void Stopwatch::Pause()
{
	m_isPaused = true;
	m_timeWhenStopped = m_sourceClock->GetTotalSeconds();
}


void Stopwatch::Resume()
{
	m_isPaused = false;
	m_isStopped = false;
}


bool Stopwatch::IsPaused() const
{
	return m_isPaused;
}


void Stopwatch::SetDuration(double duration)
{
	m_duration = duration;
}

