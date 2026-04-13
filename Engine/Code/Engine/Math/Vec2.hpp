#pragma once
#include<string>

struct Vec3;
struct IntVec2;

//-----------------------------------------------------------------------------------------------
struct Vec2
{
public:
	static const Vec2 ZERO;
	static const Vec2 ONE;

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;

public:
	// Construction/Destruction
	~Vec2();												// destructor (do nothing)
	Vec2();												// default constructor (do nothing)
	Vec2(  Vec2 const& copyFrom );							// copy constructor (from another vec2)
	explicit Vec2( float initialX, float initialY );		// explicit constructor (from x, y)

	//static functions
	static Vec2 const MakeFromPolarDegrees(float degrees, float length = 1.0f);
	static Vec2 const MakeFromPolarRadians(float radians, float length = 1.0f);

	//getters (read-only)
	float		GetLength() const;
	float		GetLengthSquared() const;
	float		GetOrientationDegrees() const;
	float		GetOrientationRadians() const;
	Vec2		GetRotated90Degrees() const;
	Vec2		GetRotatedMinus90Degrees() const;
	Vec2		GetRotatedDegrees(float degrees) const;
	Vec2		GetRotatedRadians(float radians) const;
	Vec2		GetClamped(float maxLength) const;
	Vec2		GetNormalized() const;
	Vec2		GetReflected(Vec2 const& impactSurfaceNormal) const;
	Vec3		GetVec3() const;
	IntVec2		GetIntVec2() const;

	//modifiers
	float		GetLengthAndNormalise();
	void		SetFromText(std::string text);
	void		SetOrientationDegrees(float degrees);
	void		SetOrientationRadians(float radians);
	void		SetPolarDegrees(float degrees, float length = 1.0f);
	void		SetPolarRadians(float radians, float length = 1.0f);
	void		RotateDegrees(float degrees);
	void		RotateRadians(float radians);
	void		Rotate90Degrees();
	void		RotateMinus90Degrees();
	void		SetLength(float length);
	void		ClampLength(float maxLength);
	void		Normalize();
	void		Reflect(Vec2 const& impactSurfaceNormal);
	float		NormalizeAndGetPreviousLength();

	// Operators (const)
	bool		operator==(  Vec2 const& compare ) const;		// vec2 == vec2
	bool		operator!=(  Vec2 const& compare ) const;		// vec2 != vec2
	const Vec2	operator+(  Vec2 const& vecToAdd ) const;		// vec2 + vec2
	const Vec2	operator-(  Vec2 const& vecToSubtract ) const;	// vec2 - vec2
	const Vec2	operator-() const;								// -vec2, i.e. "unary negation"
	const Vec2	operator*( float uniformScale ) const;			// vec2 * float
	const Vec2	operator*(  Vec2 const& vecToMultiply ) const;	// vec2 * vec2
	const Vec2	operator/( float inverseScale ) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=(  Vec2 const& vecToAdd );				// vec2 += vec2
	void		operator-=(  Vec2 const& vecToSubtract );		// vec2 -= vec2
	void		operator*=( const float uniformScale );			// vec2 *= float
	void		operator/=( const float uniformDivisor );		// vec2 /= float
	void		operator=(  Vec2 const& copyFrom );				// vec2 = vec2

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend const Vec2 operator*( float uniformScale,  Vec2 const& vecToScale );	// float * vec2
};


