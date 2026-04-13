//-----------------------------------------------------------------------------------------------
// Clock.hpp
//
#pragma once

#include <vector>

//-----------------------------------------------------------------------------------------------
// Clocks are a frame based timer.  If you need "real time" that is accurate at point of call, 
// use `Time.hpp` functions.  Clocks instead return a snapshot in time which usually corresponds
// to the beginning of the frame, and includes the frame delta. 
class Clock
{
friend class Clock; // this class will change his relations using private/protected methods

public:
	Clock(); 
	explicit Clock( Clock* parent ); 
	~Clock(); 

	Clock( Clock const& copy ) = delete; // do not allow clocks to be copied; 

	// Parenting
	void SetParent( Clock& parent ); 

	// Controls
	void Pause();
	void Unpause(); 
	void TogglePause(); 
	void StepFrame(); // will set this to unpaused current time scale, but will pause after the next update; 
	void SetTimeScale( double scale);
	double GetTimeScale() const						{ return m_scale; }

	// Resets this clock to start back at 0
	// Warning: Any stopwatch depending on this will not be updated, use with caution
	void Reset( bool resetChildren = true ); 

	// Accessors
	double GetFrameDeltaSeconds() const					{ return m_frameDeltaSeconds; }
	double GetTotalSeconds() const						{ return m_totalSecondsPassed; }
	size_t GetFrameCount() const						{ return m_frameCount; }

	bool IsPaused() const								{ return m_isPaused; }

public:
	static void SystemBeginFrame(); 
	static Clock& GetSystemClock(); 


protected: /** note: method names are merely a suggestion **/

	// Advances a root clock
	// Note: This should be public once you have need for other non-system root clocks.  
	// But to prevent potentially using clock system wrong we'll implement it as protected so only `SystemBeginFrame` can call it. 
	void Tick(); 

	// advanced a clocks time
	void AdvanceTime( double deltaTimeSeconds ); 

	// hierarchy control
	void AddChild( Clock* childClock ); 
	void RemoveChild( Clock* childClock ); 

protected: /** Note: Members are just a suggestion **/
	Clock* m_parent	= nullptr; 
	std::vector<Clock*> m_children; 

	// tracking
	double m_totalSecondsPassed	= 0.0;	// total time passed since a reset
	double m_frameDeltaSeconds = 0.0;	// time passed last frame
	size_t m_frameCount = 0;			// number of frames since a reset

	// control
	double m_scale = 1.0; 
	bool m_isPaused = false; 
	bool m_pauseAfterFrame = false; 

	/** Root clock options **/
	double m_lastUpdateTime	= 0.0; 
}; 




	