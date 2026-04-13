#pragma once

struct Mat44;
struct Vec3;

struct EulerAngles
{
public:
	EulerAngles();
	~EulerAngles();
	EulerAngles(float yaw, float pitch, float roll);
	EulerAngles(EulerAngles const& copyFrom);

	Vec3 GetFoward_XFwd_YLeft_ZUp() const;
	Vec3 GetLeft_XFwd_YLeft_ZUp() const;
	Vec3 GetUp_XFwd_YLeft_ZUp() const;
	void GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardVector, Vec3& out_leftVector, Vec3& out_upVector) const;
	Mat44 GetAsMatrix_XFwd_YLeft_ZUp() const;

	EulerAngles operator+(EulerAngles const& add) const;
	EulerAngles operator*(float scale) const;
	void operator+=(EulerAngles const& add);

public:
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	float m_roll = 0.0f;
};