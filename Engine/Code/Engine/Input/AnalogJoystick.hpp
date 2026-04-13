#pragma once
#include<Engine/Math/Vec2.hpp>

class AnalogJoystick
{
public:
	AnalogJoystick();
	~AnalogJoystick();
	Vec2 GetPosition() const;
	float GetMagnitude() const;
	float GetOrientationDegrees() const;

	Vec2 GetRawUncorrectedPosition() const;
	float GetInnerDeadZoneFraction() const;
	float GetOuterDeadZoneFraction() const;

	//for use by xbox controller, et al
	void Reset();
	void UpdatePosition(float rawNormalizedX, float rawNormalizedY);
	void SetDeadzoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold);
private:
	Vec2 m_rawPosition;
	Vec2 m_correctedPosition;
	float m_correctedOrientationDegrees = 0.0f;
	float m_correctedMagnitude			= 0.0f;
	float m_innerDeadzoneFraction		= 0.00f;
	float m_outerDeadzoneFraction		= 1.00f;
};
