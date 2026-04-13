#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Mat44::Mat44()
{
	m_values[Ix] = 1;
	m_values[Iy] = 0;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = 0;
	m_values[Jy] = 1;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

	m_values[Kx] = 0;
	m_values[Ky] = 0;
	m_values[Kz] = 1;
	m_values[Kw] = 0;

	m_values[Tx] = 0;
	m_values[Ty] = 0;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}


Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;
	
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;
	
	m_values[Kx] = 0;
	m_values[Ky] = 0;
	m_values[Kz] = 1;
	m_values[Kw] = 0;
	
	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}


Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0;
				 
	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0;
				 
	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0;
				 
	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
	m_values[Tw] = 1;
}


Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}


Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	m_values[Ix] = *sixteenValuesBasisMajor;
	m_values[Iy] = *(sixteenValuesBasisMajor + 1);
	m_values[Iz] = *(sixteenValuesBasisMajor + 2);
	m_values[Iw] = *(sixteenValuesBasisMajor + 3);

	m_values[Jx] = *(sixteenValuesBasisMajor + 4);
	m_values[Jy] = *(sixteenValuesBasisMajor + 5);
	m_values[Jz] = *(sixteenValuesBasisMajor + 6);
	m_values[Jw] = *(sixteenValuesBasisMajor + 7);

	m_values[Kx] = *(sixteenValuesBasisMajor + 8);
	m_values[Ky] = *(sixteenValuesBasisMajor + 9);
	m_values[Kz] = *(sixteenValuesBasisMajor + 10);
	m_values[Kw] = *(sixteenValuesBasisMajor + 11);

	m_values[Tx] = *(sixteenValuesBasisMajor + 12);
	m_values[Ty] = *(sixteenValuesBasisMajor + 13);
	m_values[Tz] = *(sixteenValuesBasisMajor + 14);
	m_values[Tw] = *(sixteenValuesBasisMajor + 15);
}


Mat44 Mat44::CreateTranslation2D(Vec2 const& translation2d)
{
	Mat44 output;

	output.m_values[Ix] = 1;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = 1;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;
	
	output.m_values[Kx] = 0;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = 1;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = translation2d.x;
	output.m_values[Ty] = translation2d.y;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateTranslation3D(Vec3 const& translation3d)
{
	Mat44 output;

	output.m_values[Ix] = 1;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = 1;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = 0;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = 1;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = translation3d.x;
	output.m_values[Ty] = translation3d.y;
	output.m_values[Tz] = translation3d.z;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateUniformScale2D(float uniformScale2d)
{
	Mat44 output;

	output.m_values[Ix] = uniformScale2d;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = uniformScale2d;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = 0;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = 1;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = 0;
	output.m_values[Ty] = 0;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateUniformScale3D(float uniformScale3d)
{
	Mat44 output;

	output.m_values[Ix] = uniformScale3d;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = uniformScale3d;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = 0;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = uniformScale3d;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = 0;
	output.m_values[Ty] = 0;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateNonUniformScale2D(Vec2 const& nonUniformScale2d)
{
	Mat44 output;

	output.m_values[Ix] = nonUniformScale2d.x;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = nonUniformScale2d.y;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = 0;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = 1;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = 0;
	output.m_values[Ty] = 0;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScale3d)
{
	Mat44 output;

	output.m_values[Ix] = nonUniformScale3d.x;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = nonUniformScale3d.y;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = 0;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = nonUniformScale3d.z;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = 0;
	output.m_values[Ty] = 0;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	Mat44 output;

	float rotationCos = CosDegrees(rotationDegreesAboutZ);
	float rotationSin = SinDegrees(rotationDegreesAboutZ);

	output.m_values[Ix] = rotationCos;
	output.m_values[Iy] = rotationSin;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = -rotationSin;
	output.m_values[Jy] = rotationCos;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = 0;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = 1;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = 0;
	output.m_values[Ty] = 0;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	Mat44 output;

	float rotationCos = CosDegrees(rotationDegreesAboutY);
	float rotationSin = SinDegrees(rotationDegreesAboutY);

	output.m_values[Ix] = rotationCos;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = -rotationSin;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = 1;
	output.m_values[Jz] = 0;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = rotationSin;
	output.m_values[Ky] = 0;
	output.m_values[Kz] = rotationCos;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = 0;
	output.m_values[Ty] = 0;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	Mat44 output;

	float rotationCos = CosDegrees(rotationDegreesAboutX);
	float rotationSin = SinDegrees(rotationDegreesAboutX);

	output.m_values[Ix] = 1;
	output.m_values[Iy] = 0;
	output.m_values[Iz] = 0;
	output.m_values[Iw] = 0;

	output.m_values[Jx] = 0;
	output.m_values[Jy] = rotationCos;
	output.m_values[Jz] = rotationSin;
	output.m_values[Jw] = 0;

	output.m_values[Kx] = 0;
	output.m_values[Ky] = -rotationSin;
	output.m_values[Kz] = rotationCos;
	output.m_values[Kw] = 0;

	output.m_values[Tx] = 0;
	output.m_values[Ty] = 0;
	output.m_values[Tz] = 0;
	output.m_values[Tw] = 1;

	return output;
}


Mat44 Mat44::CreateLookAtMatrix_XFwd_YLeft_ZUp(Vec3 const& lookAtNormal)
{
	Vec3 initialUpVec = Vec3(0.0f, 0.0f, 1.0f);
	float dotWithInitialVec = DotProduct3D(initialUpVec, lookAtNormal);
	if (dotWithInitialVec == 1.0f || dotWithInitialVec == -1.0f)
	{
		initialUpVec = Vec3(0.0f, 1.0f, 0.0f);
	}

	Vec3 leftVec = CrossProduct3D(initialUpVec, lookAtNormal);
	leftVec.Normalize();

	Vec3 upVec = CrossProduct3D(lookAtNormal, leftVec);
	upVec.Normalize();

	Mat44 modelMatrix = Mat44(lookAtNormal, leftVec, upVec, Vec3(0.0f, 0.0f, 0.0f));
	return modelMatrix;
}


Mat44 Mat44::CreateOrthoProjectMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Mat44 orthoProjMat;

	orthoProjMat.m_values[Mat44::Ix] = 2 / (right - left);
	orthoProjMat.m_values[Mat44::Jy] = 2 * (1.0f / (top - bottom));
	orthoProjMat.m_values[Mat44::Kz] = 1.0f / (zFar - zNear);

	orthoProjMat.m_values[Mat44::Tx] = (left + right) * (1.0f / (left - right));
	orthoProjMat.m_values[Mat44::Ty] = (bottom + top) * (1.0f / (bottom - top));
	orthoProjMat.m_values[Mat44::Tz] = zNear / (zNear - zFar);

	return orthoProjMat;
}


Mat44 Mat44::CreatePerspectiveProjectionMatrix(float fovDegrees, float aspect, float zNear, float zFar)
{
	Mat44 perspectiveMatrix;
	float thetaRadians = 0.5f * DegreesToRadian(fovDegrees);
	float scaleY = CosRadians(thetaRadians) / SinRadians(thetaRadians);
	float scaleX = scaleY / aspect;
	float scaleZ = zFar / (zFar - zNear);
	float translateZ = (zNear * zFar) / (zNear - zFar);

	perspectiveMatrix.m_values[Ix] = scaleX;
	perspectiveMatrix.m_values[Jy] = scaleY;
	perspectiveMatrix.m_values[Kz] = scaleZ;
	perspectiveMatrix.m_values[Kw] = 1.0f;
	perspectiveMatrix.m_values[Tz] = translateZ;
	perspectiveMatrix.m_values[Tw] = 0.0f;
	return perspectiveMatrix;
}


void Mat44::Transpose()
{
	m_values[Iy] += m_values[Jx];
	m_values[Jx] = m_values[Iy] - m_values[Jx];
	m_values[Iy] = m_values[Iy] - m_values[Jx];

	m_values[Iz] += m_values[Kx];
	m_values[Kx] = m_values[Iz] - m_values[Kx];
	m_values[Iz] = m_values[Iz] - m_values[Kx];

	m_values[Iw] += m_values[Tx];
	m_values[Tx] = m_values[Iw] - m_values[Tx];
	m_values[Iw] = m_values[Iw] - m_values[Tx];

	m_values[Ky] += m_values[Jz];
	m_values[Jz] = m_values[Ky] - m_values[Jz];
	m_values[Ky] = m_values[Ky] - m_values[Jz];

	m_values[Ty] += m_values[Jw];
	m_values[Jw] = m_values[Ty] - m_values[Jw];
	m_values[Ty] = m_values[Ty] - m_values[Jw];

	m_values[Tz] += m_values[Kw];
	m_values[Kw] = m_values[Tz] - m_values[Kw];
	m_values[Tz] = m_values[Tz] - m_values[Kw];
}


void Mat44::OrthoNormalize_XFwd_YLeft_ZUp()
{
	Vec3 forward = Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
	Vec3 left = Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
	Vec3 up = Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);

	forward.Normalize();

	Vec3 correctedLeft = CrossProduct3D(up, forward);
	correctedLeft.Normalize();

	Vec3 correctedUp = CrossProduct3D(forward, correctedLeft);
	correctedUp.Normalize();

	Vec3 matrixTranslation = Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);

	*this = Mat44(forward, correctedLeft, correctedUp, matrixTranslation);
}


Mat44 Mat44::GetOrthoNormalInverse() const
{
	Mat44 orthoNormalInverse = Mat44();
	orthoNormalInverse.m_values[Ix] = m_values[Ix];
	orthoNormalInverse.m_values[Iy] = m_values[Jx];
	orthoNormalInverse.m_values[Iz] = m_values[Kx];

	orthoNormalInverse.m_values[Jx] = m_values[Iy];
	orthoNormalInverse.m_values[Jy] = m_values[Jy];
	orthoNormalInverse.m_values[Jz] = m_values[Ky];

	orthoNormalInverse.m_values[Kx] = m_values[Iz];
	orthoNormalInverse.m_values[Ky] = m_values[Jz];
	orthoNormalInverse.m_values[Kz] = m_values[Kz];

	orthoNormalInverse.m_values[Tx] = -(m_values[Ix] * m_values[Tx]) - (m_values[Iy] * m_values[Ty]) - (m_values[Iz] * m_values[Tz]);
	orthoNormalInverse.m_values[Ty] = -(m_values[Jx] * m_values[Tx]) - (m_values[Jy] * m_values[Ty]) - (m_values[Jz] * m_values[Tz]);
	orthoNormalInverse.m_values[Tz] = -(m_values[Kx] * m_values[Tx]) - (m_values[Ky] * m_values[Ty]) - (m_values[Kz] * m_values[Tz]);

	return orthoNormalInverse;
}


Vec2 Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantity2d) const
{
	Vec2 result;
	result.x = m_values[Ix] * vectorQuantity2d.x + m_values[Jx] * vectorQuantity2d.y;
	result.y = m_values[Iy] * vectorQuantity2d.x + m_values[Jy] * vectorQuantity2d.y;
	return result;
}


Vec3 Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantity3d) const
{
	Vec3 result;
	result.x = m_values[Ix] * vectorQuantity3d.x + m_values[Jx] * vectorQuantity3d.y + m_values[Kx] * vectorQuantity3d.z;
	result.y = m_values[Iy] * vectorQuantity3d.x + m_values[Jy] * vectorQuantity3d.y + m_values[Ky] * vectorQuantity3d.z;
	result.z = m_values[Iz] * vectorQuantity3d.x + m_values[Jz] * vectorQuantity3d.y + m_values[Kz] * vectorQuantity3d.z;
	return result;
}


Vec2 Mat44::TransformPosition2D(Vec2 const& position2d) const
{
	Vec2 result;
	result.x = m_values[Ix] * position2d.x + m_values[Jx] * position2d.y + m_values[Tx];
	result.y = m_values[Iy] * position2d.x + m_values[Jy] * position2d.y + m_values[Ty];
	return result;
}


Vec3 Mat44::TransformPosition3D(Vec3 const& position3d) const
{
	Vec3 result;
	result.x = m_values[Ix] * position3d.x + m_values[Jx] * position3d.y + m_values[Kx] * position3d.z + m_values[Tx];
	result.y = m_values[Iy] * position3d.x + m_values[Jy] * position3d.y + m_values[Ky] * position3d.z + m_values[Ty];
	result.z = m_values[Iz] * position3d.x + m_values[Jz] * position3d.y + m_values[Kz] * position3d.z + m_values[Tz];
	return result;
}


Vec4 Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3d) const
{
	Vec4 result;
	result.x = m_values[Ix] * homogeneousPoint3d.x + m_values[Jx] * homogeneousPoint3d.y + m_values[Kx] * homogeneousPoint3d.z + m_values[Tx] * homogeneousPoint3d.w;
	result.y = m_values[Iy] * homogeneousPoint3d.x + m_values[Jy] * homogeneousPoint3d.y + m_values[Ky] * homogeneousPoint3d.z + m_values[Ty] * homogeneousPoint3d.w;
	result.z = m_values[Iz] * homogeneousPoint3d.x + m_values[Jz] * homogeneousPoint3d.y + m_values[Kz] * homogeneousPoint3d.z + m_values[Tz] * homogeneousPoint3d.w;
	result.w = m_values[Iw] * homogeneousPoint3d.x + m_values[Jw] * homogeneousPoint3d.y + m_values[Kw] * homogeneousPoint3d.z + m_values[Tw] * homogeneousPoint3d.w;
	return result;
}


void Mat44::SetTranslation2D(Vec2 const& tranlation2d)
{
	m_values[Tx] = tranlation2d.x;
	m_values[Ty] = tranlation2d.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}


void Mat44::SetTranslation3D(Vec3 const& tranlation3d)
{
	m_values[Tx] = tranlation3d.x;
	m_values[Ty] = tranlation3d.y;
	m_values[Tz] = tranlation3d.z;
	m_values[Tw] = 1;
}


void Mat44::SetIJ2D(Vec2 const& iBasis2d, Vec2 const& jBasis2d)
{
	m_values[Ix] = iBasis2d.x;
	m_values[Iy] = iBasis2d.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2d.x;
	m_values[Jy] = jBasis2d.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;
}


void Mat44::SetIJT2D(Vec2 const& iBasis2d, Vec2 const& jBasis2d, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2d.x;
	m_values[Iy] = iBasis2d.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2d.x;
	m_values[Jy] = jBasis2d.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}


void Mat44::SetIJK3D(Vec3 const& iBasis3d, Vec3 const& jBasis3d, Vec3 const& kBasis3d)
{
	m_values[Ix] = iBasis3d.x;
	m_values[Iy] = iBasis3d.y;
	m_values[Iz] = iBasis3d.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3d.x;
	m_values[Jy] = jBasis3d.y;
	m_values[Jz] = jBasis3d.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3d.x;
	m_values[Ky] = kBasis3d.y;
	m_values[Kz] = kBasis3d.z;
	m_values[Kw] = 0;
}


void Mat44::SetIJKT3D(Vec3 const& iBasis3d, Vec3 const& jBasis3d, Vec3 const& kBasis3d, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3d.x;
	m_values[Iy] = iBasis3d.y;
	m_values[Iz] = iBasis3d.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3d.x;
	m_values[Jy] = jBasis3d.y;
	m_values[Jz] = jBasis3d.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3d.x;
	m_values[Ky] = kBasis3d.y;
	m_values[Kz] = kBasis3d.z;
	m_values[Kw] = 0;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1;
}


void Mat44::SetIJKT4D(Vec4 const& iBasis4d, Vec4 const& jBasis4d, Vec4 const& kBasis4d, Vec4 const& translation4d)
{
	m_values[Ix] = iBasis4d.x;
	m_values[Iy] = iBasis4d.y;
	m_values[Iz] = iBasis4d.z;
	m_values[Iw] = iBasis4d.w;

	m_values[Jx] = jBasis4d.x;
	m_values[Jy] = jBasis4d.y;
	m_values[Jz] = jBasis4d.z;
	m_values[Jw] = jBasis4d.w;

	m_values[Kx] = kBasis4d.x;
	m_values[Ky] = kBasis4d.y;
	m_values[Kz] = kBasis4d.z;
	m_values[Kw] = kBasis4d.w;

	m_values[Tx] = translation4d.x;
	m_values[Ty] = translation4d.y;
	m_values[Tz] = translation4d.z;
	m_values[Tw] = translation4d.w;
}


void Mat44::Append(Mat44 const& matToAppend)
{
	float const* valuesToAppend = &matToAppend.m_values[0];
	Mat44 copyMat = *this;
	float const* originalValues = &copyMat.m_values[0];

	m_values[Ix] = originalValues[Ix] * valuesToAppend[Ix] + originalValues[Jx] * valuesToAppend[Iy] + originalValues[Kx] * valuesToAppend[Iz] + originalValues[Tx] * valuesToAppend[Iw];
	m_values[Iy] = originalValues[Iy] * valuesToAppend[Ix] + originalValues[Jy] * valuesToAppend[Iy] + originalValues[Ky] * valuesToAppend[Iz] + originalValues[Ty] * valuesToAppend[Iw];
	m_values[Iz] = originalValues[Iz] * valuesToAppend[Ix] + originalValues[Jz] * valuesToAppend[Iy] + originalValues[Kz] * valuesToAppend[Iz] + originalValues[Tz] * valuesToAppend[Iw];
	m_values[Iw] = originalValues[Iw] * valuesToAppend[Ix] + originalValues[Jw] * valuesToAppend[Iy] + originalValues[Kw] * valuesToAppend[Iz] + originalValues[Tw] * valuesToAppend[Iw];
	
	m_values[Jx] = originalValues[Ix] * valuesToAppend[Jx] + originalValues[Jx] * valuesToAppend[Jy] + originalValues[Kx] * valuesToAppend[Jz] + originalValues[Tx] * valuesToAppend[Jw];
	m_values[Jy] = originalValues[Iy] * valuesToAppend[Jx] + originalValues[Jy] * valuesToAppend[Jy] + originalValues[Ky] * valuesToAppend[Jz] + originalValues[Ty] * valuesToAppend[Jw];
	m_values[Jz] = originalValues[Iz] * valuesToAppend[Jx] + originalValues[Jz] * valuesToAppend[Jy] + originalValues[Kz] * valuesToAppend[Jz] + originalValues[Tz] * valuesToAppend[Jw];
	m_values[Jw] = originalValues[Iw] * valuesToAppend[Jx] + originalValues[Jw] * valuesToAppend[Jy] + originalValues[Kw] * valuesToAppend[Jz] + originalValues[Tw] * valuesToAppend[Jw];
	
	m_values[Kx] = originalValues[Ix] * valuesToAppend[Kx] + originalValues[Jx] * valuesToAppend[Ky] + originalValues[Kx] * valuesToAppend[Kz] + originalValues[Tx] * valuesToAppend[Kw];
	m_values[Ky] = originalValues[Iy] * valuesToAppend[Kx] + originalValues[Jy] * valuesToAppend[Ky] + originalValues[Ky] * valuesToAppend[Kz] + originalValues[Ty] * valuesToAppend[Kw];
	m_values[Kz] = originalValues[Iz] * valuesToAppend[Kx] + originalValues[Jz] * valuesToAppend[Ky] + originalValues[Kz] * valuesToAppend[Kz] + originalValues[Tz] * valuesToAppend[Kw];
	m_values[Kw] = originalValues[Iw] * valuesToAppend[Kx] + originalValues[Jw] * valuesToAppend[Ky] + originalValues[Kw] * valuesToAppend[Kz] + originalValues[Tw] * valuesToAppend[Kw];
	
	m_values[Tx] = originalValues[Ix] * valuesToAppend[Tx] + originalValues[Jx] * valuesToAppend[Ty] + originalValues[Kx] * valuesToAppend[Tz] + originalValues[Tx] * valuesToAppend[Tw];
	m_values[Ty] = originalValues[Iy] * valuesToAppend[Tx] + originalValues[Jy] * valuesToAppend[Ty] + originalValues[Ky] * valuesToAppend[Tz] + originalValues[Ty] * valuesToAppend[Tw];
	m_values[Tz] = originalValues[Iz] * valuesToAppend[Tx] + originalValues[Jz] * valuesToAppend[Ty] + originalValues[Kz] * valuesToAppend[Tz] + originalValues[Tz] * valuesToAppend[Tw];
	m_values[Tw] = originalValues[Iw] * valuesToAppend[Tx] + originalValues[Jw] * valuesToAppend[Ty] + originalValues[Kw] * valuesToAppend[Tz] + originalValues[Tw] * valuesToAppend[Tw];
}


void Mat44::AppendTranslation2D(Vec2 const& translation2d)
{
	m_values[Tx] = m_values[Ix] * translation2d.x + m_values[Jx] * translation2d.y + m_values[Tx];
	m_values[Ty] = m_values[Iy] * translation2d.x + m_values[Jy] * translation2d.y + m_values[Ty];
	m_values[Tz] = m_values[Iz] * translation2d.x + m_values[Jz] * translation2d.y + m_values[Tz];
	m_values[Tw] = m_values[Iw] * translation2d.x + m_values[Jw] * translation2d.y + m_values[Tw];
}


void Mat44::AppendTranslation3D(Vec3 const& translation3d)
{
	m_values[Tx] = m_values[Ix] * translation3d.x + m_values[Jx] * translation3d.y + m_values[Kx] * translation3d.z + m_values[Tx];
	m_values[Ty] = m_values[Iy] * translation3d.x + m_values[Jy] * translation3d.y + m_values[Ky] * translation3d.z + m_values[Ty];
	m_values[Tz] = m_values[Iz] * translation3d.x + m_values[Jz] * translation3d.y + m_values[Kz] * translation3d.z + m_values[Tz];
	m_values[Tw] = m_values[Iw] * translation3d.x + m_values[Jw] * translation3d.y + m_values[Kw] * translation3d.z + m_values[Tw];
}


void Mat44::AppendScaleUniform2D(float uniformScale2d)
{
	m_values[Ix] *= uniformScale2d;
	m_values[Iy] *= uniformScale2d;
	m_values[Iz] *= uniformScale2d;
	m_values[Iw] *= uniformScale2d;

	m_values[Jx] *= uniformScale2d;
	m_values[Jy] *= uniformScale2d;
	m_values[Jw] *= uniformScale2d;
	m_values[Jz] *= uniformScale2d;
}


void Mat44::AppendScaleUniform3D(float uniformScale3d)
{
	m_values[Ix] *= uniformScale3d;
	m_values[Iy] *= uniformScale3d;
	m_values[Iz] *= uniformScale3d;
	m_values[Iw] *= uniformScale3d;
								
	m_values[Jx] *= uniformScale3d;
	m_values[Jy] *= uniformScale3d;
	m_values[Jw] *= uniformScale3d;
	m_values[Jz] *= uniformScale3d;
								
	m_values[Kx] *= uniformScale3d;
	m_values[Ky] *= uniformScale3d;
	m_values[Kw] *= uniformScale3d;
	m_values[Kz] *= uniformScale3d;
}


void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScale2d)
{
	m_values[Ix] *= nonUniformScale2d.x;
	m_values[Iy] *= nonUniformScale2d.x;
	m_values[Iz] *= nonUniformScale2d.x;
	m_values[Iw] *= nonUniformScale2d.x;

	m_values[Jx] *= nonUniformScale2d.y;
	m_values[Jy] *= nonUniformScale2d.y;
	m_values[Jw] *= nonUniformScale2d.y;
	m_values[Jz] *= nonUniformScale2d.y;
}


void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScale3d)
{
	m_values[Ix] *= nonUniformScale3d.x;
	m_values[Iy] *= nonUniformScale3d.x;
	m_values[Iz] *= nonUniformScale3d.x;
	m_values[Iw] *= nonUniformScale3d.x;
								   
	m_values[Jx] *= nonUniformScale3d.y;
	m_values[Jy] *= nonUniformScale3d.y;
	m_values[Jw] *= nonUniformScale3d.y;
	m_values[Jz] *= nonUniformScale3d.y;

	m_values[Kx] *= nonUniformScale3d.z;
	m_values[Ky] *= nonUniformScale3d.z;
	m_values[Kw] *= nonUniformScale3d.z;
	m_values[Kz] *= nonUniformScale3d.z;
}


void Mat44::AppendZRotationDegrees(float rotationAboutZDegrees)
{
	Vec4 newIBasis;
	Vec4 newJBasis;

	float rotationCos = CosDegrees(rotationAboutZDegrees);
	float rotationSin = SinDegrees(rotationAboutZDegrees);

	newIBasis.x = m_values[Ix] * rotationCos + m_values[Jx] * rotationSin;
	newIBasis.y = m_values[Iy] * rotationCos + m_values[Jy] * rotationSin;
	newIBasis.z = m_values[Iz] * rotationCos + m_values[Jz] * rotationSin;
	newIBasis.w = m_values[Iw] * rotationCos + m_values[Jw] * rotationSin;

	newJBasis.x = m_values[Ix] * -rotationSin + m_values[Jx] * rotationCos;
	newJBasis.y = m_values[Iy] * -rotationSin + m_values[Jy] * rotationCos;
	newJBasis.z = m_values[Iz] * -rotationSin + m_values[Jz] * rotationCos;
	newJBasis.w = m_values[Iw] * -rotationSin + m_values[Jw] * rotationCos;

	m_values[Ix] = newIBasis.x;
	m_values[Iy] = newIBasis.y;
	m_values[Iz] = newIBasis.z;
	m_values[Iw] = newIBasis.w;

	m_values[Jx] = newJBasis.x;
	m_values[Jy] = newJBasis.y;
	m_values[Jz] = newJBasis.z;
	m_values[Jw] = newJBasis.w;
}


void Mat44::AppendYRotationDegrees(float rotationAboutYDegrees)
{
	Vec4 newIBasis;
	Vec4 newKBasis;

	float rotationCos = CosDegrees(rotationAboutYDegrees);
	float rotationSin = SinDegrees(rotationAboutYDegrees);

	newIBasis.x = m_values[Ix] * rotationCos + m_values[Kx] * -rotationSin;
	newIBasis.y = m_values[Iy] * rotationCos + m_values[Ky] * -rotationSin;
	newIBasis.z = m_values[Iz] * rotationCos + m_values[Kz] * -rotationSin;
	newIBasis.w = m_values[Iw] * rotationCos + m_values[Kw] * -rotationSin;

	newKBasis.x = m_values[Ix] * rotationSin + m_values[Kx] * rotationCos;
	newKBasis.y = m_values[Iy] * rotationSin + m_values[Ky] * rotationCos;
	newKBasis.z = m_values[Iz] * rotationSin + m_values[Kz] * rotationCos;
	newKBasis.w = m_values[Iw] * rotationSin + m_values[Kw] * rotationCos;

	m_values[Ix] = newIBasis.x;
	m_values[Iy] = newIBasis.y;
	m_values[Iz] = newIBasis.z;
	m_values[Iw] = newIBasis.w;

	m_values[Kx] = newKBasis.x;
	m_values[Ky] = newKBasis.y;
	m_values[Kz] = newKBasis.z;
	m_values[Kw] = newKBasis.w;
}


void Mat44::AppendXRotationDegrees(float rotationAboutXDegrees)
{
	Vec4 newKBasis;
	Vec4 newJBasis;

	float rotationCos = CosDegrees(rotationAboutXDegrees);
	float rotationSin = SinDegrees(rotationAboutXDegrees);

	newJBasis.x = m_values[Jx] * rotationCos + m_values[Kx] * rotationSin;
	newJBasis.y = m_values[Jy] * rotationCos + m_values[Ky] * rotationSin;
	newJBasis.z = m_values[Jz] * rotationCos + m_values[Kz] * rotationSin;
	newJBasis.w = m_values[Jw] * rotationCos + m_values[Kw] * rotationSin;

	newKBasis.x = m_values[Jx] * -rotationSin + m_values[Kx] * rotationCos;
	newKBasis.y = m_values[Jy] * -rotationSin + m_values[Ky] * rotationCos;
	newKBasis.z = m_values[Jz] * -rotationSin + m_values[Kz] * rotationCos;
	newKBasis.w = m_values[Jw] * -rotationSin + m_values[Kw] * rotationCos;

	m_values[Jx] = newJBasis.x;
	m_values[Jy] = newJBasis.y;
	m_values[Jz] = newJBasis.z;
	m_values[Jw] = newJBasis.w;

	m_values[Kx] = newKBasis.x;
	m_values[Ky] = newKBasis.y;
	m_values[Kz] = newKBasis.z;
	m_values[Kw] = newKBasis.w;
}


float* Mat44::GetAsFloatArray()
{
	return m_values;
}


float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}


Vec2 Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}


Vec2 Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}


Vec2 Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}


Vec3 Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}


Vec3 Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}


Vec3 Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}


Vec3 Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}


Vec4 Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}


Vec4 Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}


Vec4 Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}


Vec4 Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}


Mat44::~Mat44()
{

}
