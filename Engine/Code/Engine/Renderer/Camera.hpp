#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"

struct Vec3;

//Class representing a camera in your game
class Camera
{
public: //methods
	void	DefineGameSpace(Vec3 const& rightVec, Vec3 const& upVec, Vec3 const& awayVec );
	void	SetProjectionMatrix(Mat44 const& projectionMatrix);
	void	SetCameraModelMatrix(Mat44 const& modelMatrix);

	//Getters
	Vec2	GetOrthoLeftBottom() const;
	Vec2	GetOrthoRightTop() const;
	AABB2	GetOrthoCamBoundingBox() const;
	Mat44   GetOrthoProjectionMatrix() const;
	Mat44	GetProjectionMatrix() const;
	Mat44	GetCameraViewMatrix() const;
	Mat44	GetModelMatrix() const;
	AABB2	GetScreenViewport() const;

	//Mutators - For 2D Use
	void	SetOrthoView(Vec2 const& leftBottom, Vec2 const& rightTop);
	void	SetOrthoView(float left, float bottom, float right, float top);
	void	TranslateCamera2D(Vec2 const& translationXY);
	void	SetPosition(Vec2 const& position);
	void	SetScreenViewport(AABB2 const& viewport);

protected://members
	Vec2 m_orthoLeftBottom; //contains the bottom most and left most value of your world
	Vec2 m_orthoRightTop; //contains

	AABB2 m_screenViewport;

	Mat44 m_projectionMatrix;
	Mat44 m_modelMatrix;
	Mat44 m_renderToGameMatrix;
};
