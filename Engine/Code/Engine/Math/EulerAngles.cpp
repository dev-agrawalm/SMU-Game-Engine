#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"

EulerAngles::EulerAngles()
{
}


EulerAngles::EulerAngles(float yaw, float pitch, float roll) :
	m_yaw(yaw),
	m_pitch(pitch),
	m_roll(roll)
{
}


EulerAngles::EulerAngles(EulerAngles const& copyFrom) :
	m_yaw(copyFrom.m_yaw),
	m_pitch(copyFrom.m_pitch),
	m_roll(copyFrom.m_roll)
{
}


Vec3 EulerAngles::GetFoward_XFwd_YLeft_ZUp() const
{
	float cosYaw = CosDegrees(m_yaw);
	float sinYaw = SinDegrees(m_yaw);
	float cosPitch = CosDegrees(m_pitch);
	float sinPitch = SinDegrees(m_pitch);

	Vec3 forwardVector;
	forwardVector.x = cosYaw * cosPitch;
	forwardVector.y = sinYaw * cosPitch;
	forwardVector.z = -sinPitch;
	return forwardVector;
}


Vec3 EulerAngles::GetLeft_XFwd_YLeft_ZUp() const
{
	float cosYaw = CosDegrees(m_yaw);
	float sinYaw = SinDegrees(m_yaw);
	float cosRoll = CosDegrees(m_roll);
	float sinRoll = SinDegrees(m_roll);
	float cosPitch = CosDegrees(m_pitch);
	float sinPitch = SinDegrees(m_pitch);

	Vec3 leftVector;
	leftVector.x = -sinYaw * cosRoll + cosYaw * sinPitch * sinRoll;
	leftVector.y = cosYaw * cosRoll + sinYaw * sinPitch * sinRoll;
	leftVector.z = cosPitch * sinRoll;
	return leftVector;
}


Vec3 EulerAngles::GetUp_XFwd_YLeft_ZUp() const
{
	float cosYaw = CosDegrees(m_yaw);
	float sinYaw = SinDegrees(m_yaw);
	float cosRoll = CosDegrees(m_roll);
	float sinRoll = SinDegrees(m_roll);
	float cosPitch = CosDegrees(m_pitch);
	float sinPitch = SinDegrees(m_pitch);

	Vec3 upVector;
	upVector.x = sinYaw * sinRoll + cosYaw * sinPitch * cosRoll;
	upVector.y = -cosYaw * sinRoll + sinYaw * sinPitch * cosRoll;
	upVector.z = cosPitch * cosRoll;
	return upVector;
}


void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardVector, Vec3& out_leftVector, Vec3& out_upVector) const
{
	float cosYaw = CosDegrees(m_yaw);
	float sinYaw = SinDegrees(m_yaw);
	float cosRoll = CosDegrees(m_roll);
	float sinRoll = SinDegrees(m_roll);
	float cosPitch = CosDegrees(m_pitch);
	float sinPitch = SinDegrees(m_pitch);

	out_forwardVector.x = cosYaw * cosPitch;
	out_forwardVector.y = sinYaw * cosPitch;
	out_forwardVector.z = -sinPitch;

	out_leftVector.x = -sinYaw * cosRoll + cosYaw * sinPitch * sinRoll;
	out_leftVector.y = cosYaw * cosRoll + sinYaw * sinPitch * sinRoll;
	out_leftVector.z = cosPitch * sinRoll;

	out_upVector.x = sinYaw * sinRoll + cosYaw * sinPitch * cosRoll;
	out_upVector.y = -cosYaw * sinRoll + sinYaw * sinPitch * cosRoll;
	out_upVector.z = cosPitch * cosRoll;
}


Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	Mat44 eulerAngleMatrix;

	float cosYaw = CosDegrees(m_yaw);
	float sinYaw = SinDegrees(m_yaw);
	float cosRoll = CosDegrees(m_roll);
	float sinRoll = SinDegrees(m_roll);
	float cosPitch = CosDegrees(m_pitch);
	float sinPitch = SinDegrees(m_pitch);

	eulerAngleMatrix.m_values[Mat44::Ix] = cosYaw * cosPitch;
	eulerAngleMatrix.m_values[Mat44::Iy] = sinYaw * cosPitch;
	eulerAngleMatrix.m_values[Mat44::Iz] = -sinPitch;

	eulerAngleMatrix.m_values[Mat44::Jx] = -sinYaw * cosRoll + cosYaw * sinPitch * sinRoll;
	eulerAngleMatrix.m_values[Mat44::Jy] = cosYaw * cosRoll + sinYaw * sinPitch * sinRoll;
	eulerAngleMatrix.m_values[Mat44::Jz] = cosPitch * sinRoll;
	
	eulerAngleMatrix.m_values[Mat44::Kx] = sinYaw * sinRoll + cosYaw * sinPitch * cosRoll;
	eulerAngleMatrix.m_values[Mat44::Ky] = -cosYaw * sinRoll + sinYaw * sinPitch * cosRoll;
	eulerAngleMatrix.m_values[Mat44::Kz] = cosPitch * cosRoll;

	return eulerAngleMatrix;
}


EulerAngles EulerAngles::operator+(EulerAngles const& add) const
{
	float yaw = m_yaw + add.m_yaw;
	float pitch = m_pitch + add.m_pitch;
	float roll = m_roll + add.m_roll;

	return EulerAngles(yaw, pitch, roll);
}


EulerAngles EulerAngles::operator*(float scale) const
{
	float yaw = m_yaw * scale;
	float pitch = m_pitch * scale;
	float roll = m_roll * scale;
	return EulerAngles(yaw, pitch, roll);
}


void EulerAngles::operator+=(EulerAngles const& add)
{
	m_yaw += add.m_yaw;
	m_pitch += add.m_pitch;
	m_roll += add.m_roll;
}


EulerAngles::~EulerAngles()
{

}
