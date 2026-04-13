#pragma once
#include<string>

struct Vec2;

struct IntVec2
{
public: //member variables
	int x = 0;
	int y = 0;

public:
	static const IntVec2 ZERO;

public: //member functions
	IntVec2();
	~IntVec2();
	IntVec2(IntVec2 const& copyVec);
	explicit IntVec2(int fromX, int fromY);
	explicit IntVec2(Vec2 const& convertVec);

	//getters
	float	GetLength() const;
	int		GetLengthSquared() const;
	int		GetTaxiCabLength() const;
	float	GetOrientationDegrees() const;
	float	GetOrientationRadians() const;
	IntVec2 GetRotated90Degrees() const;
	IntVec2	GetRotatedMinus90Degrees() const;
	Vec2	GetVec2() const;

	//mutators
	void	SetFromText(std::string text);
	void	Rotate90Degrees();
	void	RotateMinus90Degrees();

	void	operator= (IntVec2 const& copyFrom);
	void	operator+= (IntVec2 const& vecToAdd);
	void	operator-= (IntVec2 const& vecToSub);
	IntVec2 operator+ (IntVec2 const& vecToAdd);
	IntVec2 operator-(IntVec2 const& vecToSub);
	bool	operator==(IntVec2 const& vecToCompare);
	bool	operator!=(IntVec2 const& vecToCompare);
};
