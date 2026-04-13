#pragma once
#include "Engine/Math/Vec2.hpp"

struct AABB2
{
public:
	static const AABB2 ZERO_TO_ONE;

public: //member variables
	Vec2 m_mins;
	Vec2 m_maxs;

public: //member functions
	AABB2();
	~AABB2();
	AABB2(AABB2 const& copyBox);
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(Vec2 const& mins, Vec2 const& maxs);
	explicit AABB2(Vec2 const& center, float width, float height);

	//getters
	bool		IsPointInside(Vec2 const& point) const;	
	Vec2 const	GetCenter() const;
	Vec2 const	GetDimensions() const;
	Vec2 const	GetNearestPoint(Vec2 const& pointOfReference) const;
	Vec2 const	GetPointAtUV(Vec2 const& uvCoords) const;
	Vec2 const	GetUVForPoint(Vec2 const& point) const;
	
	//mutators
	void	Translate(Vec2 const& byDistance);
	void	SetCenter(Vec2 const& newCenter);
	void	SetDimensions(Vec2 const& newDimensions);
	void	SetDimensions(float newWidth, float newHeight);
	void	SetHeight(float newHeight);
	void	SetWidth(float newWidth);
	void	StretchToIncludePoint(Vec2 const& pointToInclude);
	void	UniformScaleFromCenter(float scalingFactor);
	void	SetFromText(std::string valueText);

	//operator overloads
	void operator= (AABB2 const& box);
};

