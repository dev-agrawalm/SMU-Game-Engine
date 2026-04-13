#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"

AnalogJoystick::AnalogJoystick()
{

}


AnalogJoystick::~AnalogJoystick()
{

}


Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctedPosition;
}


float AnalogJoystick::GetMagnitude() const
{
	return m_correctedMagnitude;
}


float AnalogJoystick::GetOrientationDegrees() const
{
	return m_correctedOrientationDegrees;
}


Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}


float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_innerDeadzoneFraction;
}


float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadzoneFraction;
}


void AnalogJoystick::Reset()
{
	m_rawPosition = Vec2(0.0f, 0.0f);
	m_correctedPosition = Vec2(0.0f, 0.0f);
}


void AnalogJoystick::SetDeadzoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold)
{
	m_innerDeadzoneFraction = normalizedInnerDeadzoneThreshold;
	m_outerDeadzoneFraction = normalizedOuterDeadzoneThreshold;
}


void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	m_rawPosition = Vec2(rawNormalizedX, rawNormalizedY);
	
	float normalizedRawLength		= m_rawPosition.GetLength();
	float orientationAngle			= m_rawPosition.GetOrientationDegrees();
	float correctedLength			= RangeMapClamped(normalizedRawLength, m_innerDeadzoneFraction, m_outerDeadzoneFraction, 0.0f, 1.0f);

	m_correctedPosition				= Vec2::MakeFromPolarDegrees(orientationAngle, correctedLength);
	m_correctedOrientationDegrees	= m_correctedPosition.GetOrientationDegrees();
	m_correctedMagnitude			= correctedLength;
}
