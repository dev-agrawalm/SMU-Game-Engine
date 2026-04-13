#include "Camera.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec3.hpp"

void Camera::SetOrthoView(Vec2 const& leftBottom, Vec2 const& rightTop)
{
	SetOrthoView(leftBottom.x, leftBottom.y, rightTop.x, rightTop.y);
}


void Camera::SetOrthoView(float left, float bottom, float right, float top)
{
	m_orthoLeftBottom.x = left;
	m_orthoLeftBottom.y = bottom;
	m_orthoRightTop.x	= right;
	m_orthoRightTop.y	= top;

	m_renderToGameMatrix = Mat44();
	SetCameraModelMatrix(Mat44());
	SetProjectionMatrix(GetOrthoProjectionMatrix());
}


void Camera::DefineGameSpace(Vec3 const& rightVec, Vec3 const& upVec, Vec3 const& awayVec)
{
	m_renderToGameMatrix = Mat44();

	m_renderToGameMatrix.m_values[Mat44::Ix] = rightVec.x;
	m_renderToGameMatrix.m_values[Mat44::Iy] = rightVec.y;
	m_renderToGameMatrix.m_values[Mat44::Iz] = rightVec.z;

	m_renderToGameMatrix.m_values[Mat44::Jx] = upVec.x;
	m_renderToGameMatrix.m_values[Mat44::Jy] = upVec.y;
	m_renderToGameMatrix.m_values[Mat44::Jz] = upVec.z;

	m_renderToGameMatrix.m_values[Mat44::Kx] = awayVec.x;
	m_renderToGameMatrix.m_values[Mat44::Ky] = awayVec.y;
	m_renderToGameMatrix.m_values[Mat44::Kz] = awayVec.z;
}


void Camera::SetProjectionMatrix(Mat44 const& projectionMatrix)
{
	m_projectionMatrix = projectionMatrix;
	Mat44 gameToRender = m_renderToGameMatrix.GetOrthoNormalInverse();
	m_projectionMatrix.Append(gameToRender);
}


void Camera::SetCameraModelMatrix(Mat44 const& modelMatrix)
{
	m_modelMatrix = modelMatrix;
}


void Camera::TranslateCamera2D(Vec2 const& translationXY)
{
	m_orthoLeftBottom	+= translationXY;
	m_orthoRightTop		+= translationXY;
	SetOrthoView(m_orthoLeftBottom, m_orthoRightTop);
}


Vec2 Camera::GetOrthoLeftBottom() const
{
	return m_orthoLeftBottom;
}


Vec2 Camera::GetOrthoRightTop() const
{
	return m_orthoRightTop;
}


AABB2 Camera::GetOrthoCamBoundingBox() const
{
	return AABB2(m_orthoLeftBottom, m_orthoRightTop);
}


Mat44 Camera::GetOrthoProjectionMatrix() const
{
	Mat44 orthoProjMat = Mat44::CreateOrthoProjectMatrix(m_orthoLeftBottom.x, m_orthoRightTop.x, m_orthoLeftBottom.y, m_orthoRightTop.y, 0.0f, 1.0f);
	return orthoProjMat;
}


Mat44 Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}


Mat44 Camera::GetCameraViewMatrix() const
{
	return m_modelMatrix.GetOrthoNormalInverse();
}


Mat44 Camera::GetModelMatrix() const
{
	return m_modelMatrix;
}


AABB2 Camera::GetScreenViewport() const
{
	return m_screenViewport;
}


void Camera::SetPosition(Vec2 const& position)
{
	AABB2 cameraBox = AABB2(m_orthoLeftBottom, m_orthoRightTop);
	cameraBox.SetCenter(position);

	m_orthoLeftBottom	= cameraBox.m_mins;
	m_orthoRightTop		= cameraBox.m_maxs;
	SetOrthoView(m_orthoLeftBottom, m_orthoRightTop);
}


void Camera::SetScreenViewport(AABB2 const& viewport)
{
	m_screenViewport = viewport;
}

