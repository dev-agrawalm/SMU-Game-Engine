#pragma once
#include<string>

struct Vec2;

//-----------------------------------------------------------------------------------------------
struct Vec3
{
public:
	static const Vec3 ZERO;

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.0f;

public:
	// Construction/Destruction
	~Vec3();												// destructor (do nothing)
	Vec3();												// default constructor (do nothing)
	Vec3(const Vec3& copyFrom);							// copy constructor (from another vec3)
	explicit Vec3(float initialX, float initialY, float initalZ);		// explicit constructor (from x, y, z)
	explicit Vec3(float initialX, float initialY);						// explicit constructor (from x,y)

	//getters (read-only)
	float		GetLength() const;
	float		GetLengthXY() const;
	float		GetLengthSquared() const;
	float		GetLengthXYSquared() const;
	float		GetAngleAboutZDegrees() const;
	float		GetAngleAboutZRadians() const;
	Vec3		GetRotatedAboutZDegrees(float degrees) const;
	Vec3		GetRotatedAboutZRadians(float radians) const;
	Vec2		GetVec2() const;
	Vec3		GetNormalized() const;
	Vec3		GetXYVec3() const;
	Vec3		GetReflected(Vec3 const& impactSurfaceNormal) const;

	float		GetLengthAndNormalise();
	void		Normalize();
	void		SetFromText(std::string text);

	// Operators (const)
	bool		operator==(const Vec3& compare) const;		// vec3 == vec3
	bool		operator!=(const Vec3& compare) const;		// vec3 != vec3
	const Vec3	operator+(const Vec3& vecToAdd) const;		// vec3 + vec3
	const Vec3	operator-(const Vec3& vecToSubtract) const;	// vec3 - vec3
	const Vec3	operator-() const;								// -vec3, i.e. "unary negation"
	const Vec3	operator*(float uniformScale) const;			// vec3 * float
	const Vec3	operator*(const Vec3& vecToMultiply) const;	// vec3 * vec3
	const Vec3	operator/(float inverseScale) const;			// vec3 / float

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec3& vecToAdd);				// vec3 += vec2
	void		operator-=(const Vec3& vecToSubtract);		// vec3 -= vec2
	void		operator*=(const float uniformScale);			// vec3 *= float
	void		operator/=(const float uniformDivisor);		// vec3 /= float
	void		operator=(const Vec3& copyFrom);				// vec3 = vec3

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend const Vec3 operator*(float uniformScale, const Vec3& vecToScale);	// float * vec3
};
