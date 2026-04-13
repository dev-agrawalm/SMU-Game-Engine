//-----------------------------------------------------------------------------------------------
// Stopwatch.hpp
//
#pragma once

class Clock; 

//-----------------------------------------------------------------------------------------------
// Used for keeping track of a duration, countdown, or repeating event. 
class Stopwatch
{
public:
	Stopwatch(); 
	explicit Stopwatch( double secondDuration ); 
	Stopwatch( Clock const& sourceClock, double secondDuration ); 

	// starts a timer running for N seconds. 
	void Start( Clock const& sourceClock, double seconds ); 
	void Start( double seconds ); 
	void SetClock( Clock const& sourceClock ); // changes clock, but keeps current elapsed time; 
	void Restart(); 
	void Stop(); 

	double GetDuration() const;
	double GetElapsedSeconds() const; 
	float GetElapsedFraction() const; // normalized to duration (0.5 means stopwatch is half complete.  2.2 would mean it has run 2 full durations, and 20% into a third)
	bool IsStopped() const;

	// Queries
	bool HasElapsed() const;									// has the timer been running for the full duration?
	bool Check() const				{ return HasElapsed(); }	// alias for `HasElapsed`
	bool CheckAndDecrement();									// check if duration has elapsed, and removes that much time from the timer if so (returns true if decrement happened)
	bool CheckAndRestart();										// check if duration has elapsed, and restarts the timer if so (returns true if it had elapsed & reset)
	int DecrementAllAndRestart();											// removes all accrued durations and returns count
	int DecrementAllAndPause();

	// Optional  : Supporting Pause/Resume
	void Pause(); 
	void Resume(); 
	bool IsPaused() const; 
	void SetDuration(double duration);


private: // suggested members
	Clock const* m_sourceClock	= nullptr; 
	double m_startTime			= 0.0; 
	double m_duration			= 0.0; 

	double m_timeWhenStopped	= 0.0;
	bool m_isStopped			= false;
	bool m_isPaused				= false;
}; 
	