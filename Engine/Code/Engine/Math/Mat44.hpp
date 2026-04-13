#pragma once

struct Vec2;
struct Vec3;
struct Vec4;

struct Mat44
{
public:
	enum {Ix, Iy, Iz, Iw, Jx, Jy, Jz, Jw, Kx, Ky, Kz, Kw, Tx, Ty, Tz, Tw}; //index nicknames to avoid confusion when operating on these values
	float m_values[16];

public:
	Mat44();
	~Mat44();
	explicit Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D);
	explicit Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D);
	explicit Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D);
	explicit Mat44(float const* sixteenValuesBasisMajor);

	static Mat44 CreateTranslation2D(Vec2 const& translation2d);
	static Mat44 CreateTranslation3D(Vec3 const& translation3d);
	static Mat44 CreateUniformScale2D(float uniformScale2d);
	static Mat44 CreateUniformScale3D(float uniformScale3d);
	static Mat44 CreateNonUniformScale2D(Vec2 const& nonUniformScale2d);
	static Mat44 CreateNonUniformScale3D(Vec3 const& nonUniformScale3d);
	static Mat44 CreateZRotationDegrees(float rotationDegreesAboutZ);
	static Mat44 CreateYRotationDegrees(float rotationDegreesAboutY);
	static Mat44 CreateXRotationDegrees(float rotationDegreesAboutX);

	static Mat44 CreateLookAtMatrix_XFwd_YLeft_ZUp(Vec3 const& lookAtNormal);
	static Mat44 CreateOrthoProjectMatrix(float left, float right, float bottom, float top, float zNear, float zFar);
	static Mat44 CreatePerspectiveProjectionMatrix(float fovDegrees, float aspect, float zNear, float zFar);

	void  Transpose();
	void  OrthoNormalize_XFwd_YLeft_ZUp();
	Mat44 GetOrthoNormalInverse() const;

	Vec2 TransformVectorQuantity2D(Vec2 const& vectorQuantity2d) const;
	Vec3 TransformVectorQuantity3D(Vec3 const& vectorQuantity3d) const;
	Vec2 TransformPosition2D(Vec2 const& position2d) const;
	Vec3 TransformPosition3D(Vec3 const& position3d) const;
	Vec4 TransformHomogeneous3D(Vec4 const& homogeneousPoint3d) const;

	void SetTranslation2D(Vec2 const& tranlation2d);
	void SetTranslation3D(Vec3 const& tranlation3d);
	void SetIJ2D(Vec2 const& iBasis2d, Vec2 const& jBasis2d);
	void SetIJT2D(Vec2 const& iBasis2d, Vec2 const& jBasis2d, Vec2 const& translationXY);
	void SetIJK3D(Vec3 const& iBasis3d, Vec3 const& jBasis3d, Vec3 const& kBasis3d);
	void SetIJKT3D(Vec3 const& iBasis3d, Vec3 const& jBasis3d, Vec3 const& kBasis3d, Vec3 const& translationXYZ);
	void SetIJKT4D(Vec4 const& iBasis4d, Vec4 const& jBasis4d, Vec4 const& kBasis4d, Vec4 const& translation4d);

	void Append(Mat44 const& matToAppend);
	void AppendTranslation2D(Vec2 const& translation2d);
	void AppendTranslation3D(Vec3 const& translation3d);
	void AppendScaleUniform2D(float uniformScale2d);
	void AppendScaleUniform3D(float uniformScale3d);
	void AppendScaleNonUniform2D(Vec2 const& nonUniformScale2d);
	void AppendScaleNonUniform3D(Vec3 const& nonUniformScale3d);
	void AppendZRotationDegrees(float rotationAboutZDegrees);
	void AppendYRotationDegrees(float rotationAboutYDegrees);
	void AppendXRotationDegrees(float rotationAboutXDegrees);

	float*		 GetAsFloatArray();
	float const* GetAsFloatArray()	const;
	Vec2		 GetIBasis2D()		const;
	Vec2		 GetJBasis2D()		const;
	Vec2		 GetTranslation2D() const;
	Vec3		 GetIBasis3D()		const;
	Vec3		 GetJBasis3D()		const;
	Vec3		 GetKBasis3D()		const;
	Vec3		 GetTranslation3D() const;
	Vec4		 GetIBasis4D()		const;
	Vec4		 GetJBasis4D()		const;
	Vec4		 GetKBasis4D()		const;
	Vec4		 GetTranslation4D() const;
};
