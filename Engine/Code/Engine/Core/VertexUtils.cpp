#include "VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/BezierCurve2D.hpp"
#include "Engine/Math/ConvexPoly2.hpp"

void TransformVertexArrayXY3D(int numVertexes, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	Vec2 iBasis = Vec2::MakeFromPolarDegrees(rotationDegreesAboutZ, uniformScaleXY);
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	for (int index = 0; index < numVertexes; index++)
	{
		TransformPosXY3D(verts[index].m_position, iBasis, jBasis, translationXY);
	}
}


void TransformVertexArrayXY3D(int numVertexes, Vertex_PCU* verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	for (int index = 0; index < numVertexes; index++)
	{
		TransformPosXY3D(verts[index].m_position, iBasis, jBasis, translationXY);
	}
}


void TransformVertexArray3D(int numVertexes, Vertex_PCU* verts, Mat44 const& modelMatrix)
{
	for (int vertIndex = 0; vertIndex < numVertexes; vertIndex++)
	{
		Vertex_PCU& vert = verts[vertIndex];
		vert.m_position = modelMatrix.TransformPosition3D(vert.m_position);
	}
}


Mat44 GetModelMatrixFromUpVectorAndTranslation(Vec3 const& upVector, Vec3 const& translation)
{
	Vec3 initialCrossVec = Vec3(1.0f, 0.0f, 0.0f);
	float dotWithInitialVec = DotProduct3D(initialCrossVec, upVector);
	if (dotWithInitialVec == 1.0f || dotWithInitialVec == -1.0f)
	{
		initialCrossVec = Vec3(0.0f, 1.0f, 0.0f);
	}

	Vec3 leftVec = CrossProduct3D(upVector, initialCrossVec);
	leftVec.Normalize();

	Vec3 forwardVec = CrossProduct3D(leftVec, upVector);
	forwardVec.Normalize();

	Mat44 modelMatrix = Mat44(forwardVec, leftVec, upVector, translation);
	return modelMatrix;
}


void AddVertsForLine3DToVector(std::vector<Vertex_PCU>& out_vector, Vec3 const& start, Vec3 const& end, float thickness /*= 1.0f*/, float padding /*= 0.05f*/, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec3 lineForward = (end - start).GetNormalized();
	float lineLength = (end - start).GetLength();
	Vec3 cylinderUp = lineForward;

	int existingVertCount = (int) out_vector.size();
	float cylinderHeight = lineLength + padding * 2.0f;
	AddVertsForCylinderZ3DToVector(out_vector, Vec2(0.0f, 0.0f), FloatRange(-cylinderHeight * 0.5f, cylinderHeight * 0.5f), thickness * 0.5f, 8.0f, color);
	int addVertCount = (int) out_vector.size() - existingVertCount;
	
	Mat44 cylinderModel = GetModelMatrixFromUpVectorAndTranslation(cylinderUp, start + lineForward * lineLength * 0.5f); //Mat44(lineKBasis, lineJBasis, lineIBasis, start + lineForward * lineLength * 0.5f);
	TransformVertexArray3D(addVertCount, &out_vector[existingVertCount], cylinderModel);
}


void AddVertsForQuad3DToVector(std::vector<Vertex_PCU>& out_vector, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, 
							 Rgba8 const& color, AABB2 const& UVs)
{
	Vec2 topRightUV		= UVs.m_maxs;
	Vec2 topLeftUV		= Vec2(UVs.m_mins.x, UVs.m_maxs.y);
	Vec2 bottomRightUV	= Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 bottomLeftUV	= UVs.m_mins;

	out_vector.push_back(Vertex_PCU(bottomLeft,		color, bottomLeftUV));
	out_vector.push_back(Vertex_PCU(bottomRight,	color, bottomRightUV));
	out_vector.push_back(Vertex_PCU(topLeft,		color, topLeftUV));
	
	out_vector.push_back(Vertex_PCU(bottomRight,	color, bottomRightUV));
	out_vector.push_back(Vertex_PCU(topRight,		color, topRightUV));
	out_vector.push_back(Vertex_PCU(topLeft,		color, topLeftUV));
}


void AddVertsForAABB2ToVector(std::vector<Vertex_PCU>& vertVector, AABB2 const& aabb2, Rgba8 const& color, Vec2 const& uvMins, Vec2 const& uvMaxs)
{
	//box corner positions
	Vec3 topRight		= Vec3(aabb2.m_maxs.x, aabb2.m_maxs.y);
	Vec3 topLeft		= Vec3(aabb2.m_mins.x, aabb2.m_maxs.y);
	Vec3 bottomRight	= Vec3(aabb2.m_maxs.x, aabb2.m_mins.y);
	Vec3 bottomLeft		= Vec3(aabb2.m_mins.x, aabb2.m_mins.y);

	//box uv coordinates
	Vec2 topRightUV		= uvMaxs;
	Vec2 topLeftUV		= Vec2(uvMins.x, uvMaxs.y);
	Vec2 bottomRightUV	= Vec2(uvMaxs.x, uvMins.y);
	Vec2 bottomLeftUV	= uvMins;

	vertVector.push_back(Vertex_PCU(bottomLeft,		color, bottomLeftUV));
	vertVector.push_back(Vertex_PCU(bottomRight,	color, bottomRightUV));
	vertVector.push_back(Vertex_PCU(topLeft,		color, topLeftUV));
	
	vertVector.push_back(Vertex_PCU(bottomRight,	color, bottomRightUV));
	vertVector.push_back(Vertex_PCU(topRight,		color, topRightUV));
	vertVector.push_back(Vertex_PCU(topLeft,		color, topLeftUV));
}


void AddVertsForDisk2DToVector(std::vector<Vertex_PCU>& vertVector, Vec2 const& diskCenter, float diskRadius, Rgba8 const& color, int numSegments)
{
	int		totalVertCount			= numSegments * 3;
	float	thetaInterval			= (360.0f / static_cast<float>(numSegments));
	float	startThetaDegrees		= 0.0f;
	int		initialisedVertexCount	= 0;

	while (initialisedVertexCount < totalVertCount)
	{
		float endThetaDegrees	= startThetaDegrees + thetaInterval;
		float cosStartTheta		= CosDegrees(startThetaDegrees);
		float sinStartTheta		= SinDegrees(startThetaDegrees);
		float cosEndTheta		= CosDegrees(endThetaDegrees);
		float sinEndTheta		= SinDegrees(endThetaDegrees);
		Vec3 centerPosition		= diskCenter.GetVec3();
		Vec3 segmentInnerPoint	= centerPosition + Vec3(diskRadius * cosStartTheta, diskRadius * sinStartTheta);
		Vec3 segmentOuterPoint	= centerPosition + Vec3(diskRadius * cosEndTheta,	diskRadius * sinEndTheta);
		vertVector.push_back(Vertex_PCU(centerPosition,		color));
		vertVector.push_back(Vertex_PCU(segmentInnerPoint,	color));
		vertVector.push_back(Vertex_PCU(segmentOuterPoint,	color));

		initialisedVertexCount	+= 3;
		startThetaDegrees		+= thetaInterval;
	}
}


void AddVertsForCapsule2DToVector(std::vector<Vertex_PCU>& vertVector, Capsule2 const& capsule2d, Rgba8 const color, int numSegmentsPerHemisphere /*= 16*/)
{
	LineSegment2	bone			= capsule2d.m_bone;							//		Capsule assumed to be oriented at 0 degrees and centered at (0,0)
	float			capsuleRadius	= capsule2d.m_radius;						//            	  #	################################ #
	float			boneLength		= bone.GetDistance();						//            	#	#     CapsuleMidSection        #   #
	float			halfBoneLength	= boneLength * 0.5f;						//Hemisphere 2#		#            (0,0)             #	 #  Hemisphere 1
	Vec2			boneDirection	= bone.GetDirection();						//            	#	#                              #   #
	Vec2			capsuleCenter	= capsule2d.GetCenter();					//            	  #	################################ #
	
	std::vector<Vertex_PCU> capsuleVertexes;
	
	//Assume the capsule is at origin(0,0) oriented at 0 degrees pointing in the east direction
	AABB2 capsuleMidSection = AABB2(Vec2::ZERO, boneLength, capsule2d.m_radius * 2.0f);
	AddVertsForAABB2ToVector(capsuleVertexes, capsuleMidSection, color);

	int		totalVertCountPerHemisphere = numSegmentsPerHemisphere * 3;
	float	thetaInterval				= (360.0f / static_cast<float>(numSegmentsPerHemisphere));
	
	//hemisphere 1
	Vec2	hemisphere1Center					= Vec2(halfBoneLength, 0.0f);
	float	startThetaDegreesHemisphere1		= -90.0f;
	int		initialisedVertexCountHemisphere1	= 0;

	while (initialisedVertexCountHemisphere1 < totalVertCountPerHemisphere)
	{
		float endThetaDegreesHemisphere1	= startThetaDegreesHemisphere1 + thetaInterval;
		float cosStartThetaHemisphere1		= CosDegrees(startThetaDegreesHemisphere1);
		float sinStartThetaHemisphere1		= SinDegrees(startThetaDegreesHemisphere1);
		float cosEndThetaHemisphere1		= CosDegrees(endThetaDegreesHemisphere1);
		float sinEndThetaHemisphere1		= SinDegrees(endThetaDegreesHemisphere1);
		Vec3 centerPositionHemisphere1		= hemisphere1Center.GetVec3();
		Vec3 segmentInnerPointHemisphere1	= centerPositionHemisphere1 + Vec3(capsuleRadius * cosStartThetaHemisphere1,	capsuleRadius * sinStartThetaHemisphere1);
		Vec3 segmentOuterPointHemisphere1	= centerPositionHemisphere1 + Vec3(capsuleRadius * cosEndThetaHemisphere1,		capsuleRadius * sinEndThetaHemisphere1);
		capsuleVertexes.push_back(Vertex_PCU(centerPositionHemisphere1,		color));
		capsuleVertexes.push_back(Vertex_PCU(segmentInnerPointHemisphere1,	color));
		capsuleVertexes.push_back(Vertex_PCU(segmentOuterPointHemisphere1,	color));

		initialisedVertexCountHemisphere1	+= 3;
		startThetaDegreesHemisphere1		+= thetaInterval;
	}

	//hemisphere 2
	Vec2	hemisphere2Center					= Vec2(-halfBoneLength, 0.0f);
	float	startThetaDegreesHemisphere2		= 90.0f;
	int		initialisedVertexCountHemisphere2	= 0;

	while (initialisedVertexCountHemisphere2 < totalVertCountPerHemisphere)
	{
		float endThetaDegreesHemisphere2	= startThetaDegreesHemisphere2 + thetaInterval;
		float cosStartThetaHemisphere2		= CosDegrees(startThetaDegreesHemisphere2);
		float sinStartThetaHemisphere2		= SinDegrees(startThetaDegreesHemisphere2);
		float cosEndThetaHemisphere2		= CosDegrees(endThetaDegreesHemisphere2);
		float sinEndThetaHemisphere2		= SinDegrees(endThetaDegreesHemisphere2);
		Vec3 centerPositionHemisphere2		= hemisphere2Center.GetVec3();
		Vec3 segmentInnerPointHemisphere2	= centerPositionHemisphere2 + Vec3(capsuleRadius * cosStartThetaHemisphere2,	capsuleRadius * sinStartThetaHemisphere2);
		Vec3 segmentOuterPointHemisphere2	= centerPositionHemisphere2 + Vec3(capsuleRadius * cosEndThetaHemisphere2,		capsuleRadius * sinEndThetaHemisphere2);
		capsuleVertexes.push_back(Vertex_PCU(centerPositionHemisphere2, color));
		capsuleVertexes.push_back(Vertex_PCU(segmentInnerPointHemisphere2, color));
		capsuleVertexes.push_back(Vertex_PCU(segmentOuterPointHemisphere2, color));

		initialisedVertexCountHemisphere2	+= 3;
		startThetaDegreesHemisphere2		+= thetaInterval;
	}

	//Rotate and translate capsule to its actual position and orientation
	Vec2 iBasis = boneDirection.GetNormalized();
	Vec2 jBasis = boneDirection.GetRotated90Degrees();
	TransformVertexArrayXY3D( (int) capsuleVertexes.size(), capsuleVertexes.data(), iBasis, jBasis, capsuleCenter);

	for (int capsuleVertIndex = 0; capsuleVertIndex < capsuleVertexes.size(); capsuleVertIndex++)
	{
		vertVector.push_back(capsuleVertexes[capsuleVertIndex]);
	}
}


void AddVertsForOBB2DToVector(std::vector<Vertex_PCU>& vertVector, OBB2 const& obb2, Rgba8 const color)
{
	Vec2 bottomLeft;
	Vec2 bottomRight;
	Vec2 topLeft;
	Vec2 topRight;
	obb2.GetCornersInWorldSpace(bottomLeft, bottomRight, topLeft, topRight);

	vertVector.push_back(Vertex_PCU(bottomLeft.GetVec3(),	color));
	vertVector.push_back(Vertex_PCU(bottomRight.GetVec3(),	color));
	vertVector.push_back(Vertex_PCU(topLeft.GetVec3(),		color));

	vertVector.push_back(Vertex_PCU(bottomRight.GetVec3(),	color));
	vertVector.push_back(Vertex_PCU(topRight.GetVec3(),		color));
	vertVector.push_back(Vertex_PCU(topLeft.GetVec3(),		color));
}


void AddVertsForLineSegment2DToVector(std::vector<Vertex_PCU>& vertVector, LineSegment2 const& line2d, Rgba8 const& color, float width /*= 1.0f*/, float padding /*= 5.0f*/)
{
	int const NUM_VERTS_LINE = 6;
	std::vector<Vertex_PCU> lineVertVector;
	lineVertVector.reserve(NUM_VERTS_LINE); //6 verts per line (line is 2 triangles)
	Vec2 line = line2d.GetLineVector();
	float originX = 0.0f - padding;
	float originY = 0.0f;
	float halfWidth = width / 2.0f;
	float length = line.GetLength() + padding * 2.0f;													
																								//the line to be drawn
																								//		Top Left                             Top Right
	Vec3 bottomLeft		= Vec3(originX,			 originY - halfWidth);							//		   #######################################
	Vec3 bottomRight	= Vec3(originX + length, originY - halfWidth);							//(origin) #									 # (length)        
	Vec3 topLeft		= Vec3(originX,			 originY + halfWidth);							//		   #######################################
	Vec3 topRight		= Vec3(originX + length, originY + halfWidth);							//		Bottom Left                          Bottom Right
	
	//lower triangle
	lineVertVector.push_back(Vertex_PCU(bottomLeft,		color));
	lineVertVector.push_back(Vertex_PCU(bottomRight,	color));
	lineVertVector.push_back(Vertex_PCU(topRight,		color));
	
	//upper triangle
	lineVertVector.push_back(Vertex_PCU(bottomLeft,		color));
	lineVertVector.push_back(Vertex_PCU(topRight,		color));
	lineVertVector.push_back(Vertex_PCU(topLeft,		color));

	Vec2 iBasis = line.GetNormalized();
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	TransformVertexArrayXY3D(NUM_VERTS_LINE, lineVertVector.data(), iBasis, jBasis, line2d.m_start);

	for (int lineVertIndex = 0; lineVertIndex < lineVertVector.size(); lineVertIndex++)
	{
		vertVector.push_back(lineVertVector[lineVertIndex]);
	}
}


void AddVertsForArrow2DToVector(std::vector<Vertex_PCU>& vertVector, Vec2 const& arrowStart, Vec2 const& arrowEnd, Rgba8 const& color, float width /*= 1.0f*/, float arrowTipLength /*= 1.0f*/, float padding /*= 5.0f*/)
{
	LineSegment2 arrowBone = LineSegment2(arrowStart, arrowEnd);
	AddVertsForLineSegment2DToVector(vertVector, arrowBone, color, width, padding);

	Vec2 arrowNormal = arrowBone.GetLineVector().GetNormalized();

	Vec2 rightArrowTipVec = -arrowNormal.GetRotatedDegrees(45.0f);
	LineSegment2 arrowTipRightHalf = LineSegment2(arrowEnd, arrowEnd + rightArrowTipVec * arrowTipLength);
	AddVertsForLineSegment2DToVector(vertVector, arrowTipRightHalf, color, width, padding);

	Vec2 leftArrowTipVec = -arrowNormal.GetRotatedDegrees(-45.0f);
	LineSegment2 arrowTipLeftHalf = LineSegment2(arrowEnd, arrowEnd + leftArrowTipVec * arrowTipLength);
	AddVertsForLineSegment2DToVector(vertVector, arrowTipLeftHalf, color, width, padding);
}


void AddVertsForQuadraticBezierCurve2DToVector(std::vector<Vertex_PCU>& vertVector, QuadraticBezierCurve2D const& qbCurve, Rgba8 const& color /*= Rgba8::WHITE*/, float width /*= 1.0f*/, float padding /*= 5.0f*/, int numSegments /*= 60.0f*/)
{
	float segmentInterval = 1.0f / numSegments;
	for (int segmentIndex = 0; segmentIndex < numSegments; segmentIndex++)
	{
		Vec2 segmentStart = qbCurve.GetPointAtFractionOfCurve(0.0f + segmentIndex * segmentInterval);
		Vec2 segmentEnd = qbCurve.GetPointAtFractionOfCurve(0.0f + (segmentIndex + 1) * segmentInterval);
		LineSegment2 segmentLine = LineSegment2(segmentStart, segmentEnd);
		AddVertsForLineSegment2DToVector(vertVector, segmentLine, color, width, padding);
	}
}


void AddVertsForCubicBezierCurve2DToVector(std::vector<Vertex_PCU>& vertVector, CubicBezierCurve2D const& cbCurve, Rgba8 const& color /*= Rgba8::WHITE*/, float width /*= 1.0f*/, float padding /*= 5.0f*/, int numSegments /*= 60.0f*/)
{
	float segmentInterval = 1.0f / numSegments;
	for (int segmentIndex = 0; segmentIndex < numSegments; segmentIndex++)
	{
		Vec2 segmentStart = cbCurve.GetPointAtFraction(0.0f + segmentIndex * segmentInterval);
		Vec2 segmentEnd = cbCurve.GetPointAtFraction(0.0f + (segmentIndex + 1) * segmentInterval);
		LineSegment2 segmentLine = LineSegment2(segmentStart, segmentEnd);
		AddVertsForLineSegment2DToVector(vertVector, segmentLine, color, width, padding);
	}
}


void AddVertsForCubicHermiteCurve2DToVector(std::vector<Vertex_PCU>& vertVector, CubicHermiteCurve2D const& hermiteCurve, Rgba8 const& color /*= Rgba8::WHITE*/, float width /*= 1.0f*/, float padding /*= 5.0f*/, int numSegments /*= 60.0f*/)
{
	CubicBezierCurve2D bezierCurve = hermiteCurve.GetCubicBezierCurve();
	AddVertsForCubicBezierCurve2DToVector(vertVector, bezierCurve, color, width, padding, numSegments);
}


void AddVertsForConvexPoly2ToVector(std::vector<Vertex_PCU>& vertVector, ConvexPoly2 const& convexPoly2, Rgba8 const& color)
{
	int numberPts = convexPoly2.GetNumPoints();
	GUARANTEE_OR_DIE(numberPts >= 3, "Convex poly should have atleast 3 points for drawing");

	std::vector<Vertex_PCU> convexPolyVerts;
	Vec2 pt0 = convexPoly2.GetPointAtIndex(0);

	for (int i = 1; i <= convexPoly2.GetNumPoints() - 2; i++)
	{
		Vec2 pt1 = convexPoly2.GetPointAtIndex(i);
		Vec2 pt2 = convexPoly2.GetPointAtIndex(i + 1);

		Vertex_PCU vert1(pt0.GetVec3(), color);
		Vertex_PCU vert2(pt1.GetVec3(), color);
		Vertex_PCU vert3(pt2.GetVec3(), color);

		vertVector.push_back(vert1);
		vertVector.push_back(vert2);
		vertVector.push_back(vert3);
	}
}


void AddVertsForAABBZ3DToVector(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, Rgba8 const& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float minX = aabb3.m_mins.x;
	float minY = aabb3.m_mins.y;
	float minZ = aabb3.m_mins.z;
	float maxX = aabb3.m_maxs.x;
	float maxY = aabb3.m_maxs.y;
	float maxZ = aabb3.m_maxs.z;

	//south face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), tint, UVs);
	//north face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), tint, UVs);
	//east face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), tint, UVs);
	//west face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), tint, UVs);
	//sky face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), tint, UVs);
	//hell face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), tint, UVs);
}


void AddVertsForAABBZ3DToVector(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, CubeFaceTints const& tints, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float minX = aabb3.m_mins.x;
	float minY = aabb3.m_mins.y;
	float minZ = aabb3.m_mins.z;
	float maxX = aabb3.m_maxs.x;
	float maxY = aabb3.m_maxs.y;
	float maxZ = aabb3.m_maxs.z;

	//south face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), tints.southTint, UVs);
	//north face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), tints.northTint, UVs);
	//east face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), tints.eastTint, UVs);
	//west face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), tints.westTint, UVs);
	//sky face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), tints.skywardTint, UVs);
	//hell face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), tints.hellwardTint, UVs);
}


void AddVertsForAABBZ3DToVector(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, Rgba8 const& topTint, Rgba8 const& sideTint, Rgba8 const& bottomTint, AABB2 const& topUVs, AABB2 const& sideUVs, AABB2 const& bottomUVs)
{
	float minX = aabb3.m_mins.x;
	float minY = aabb3.m_mins.y;
	float minZ = aabb3.m_mins.z;
	float maxX = aabb3.m_maxs.x;
	float maxY = aabb3.m_maxs.y;
	float maxZ = aabb3.m_maxs.z;

	//south face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), sideTint, sideUVs);
	//north face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), sideTint, sideUVs);
	//east face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), sideTint, sideUVs);
	//west face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), sideTint, sideUVs);
	//sky face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), topTint, topUVs);
	//hell face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), bottomTint, bottomUVs);
}


void AddVertsForAABBZ3DToVector(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, Rgba8 const& tint, AABB2 const& topUVs, AABB2 const& sideUVs, AABB2 const& bottomUVs)
{
	float minX = aabb3.m_mins.x;
	float minY = aabb3.m_mins.y;
	float minZ = aabb3.m_mins.z;
	float maxX = aabb3.m_maxs.x;
	float maxY = aabb3.m_maxs.y;
	float maxZ = aabb3.m_maxs.z;

	//south face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), tint, sideUVs);
	//north face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), tint, sideUVs);
	//east face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), tint, sideUVs);
	//west face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), tint, sideUVs);
	//sky face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), tint, topUVs);
	//hell face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), tint, bottomUVs);
}


void AddVertsForAABBZ3DToVector(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, CubeFaceTints const& tints, AABB2 const& topUVs, AABB2 const& sideUVs, AABB2 const& bottomUVs)
{
	float minX = aabb3.m_mins.x;
	float minY = aabb3.m_mins.y;
	float minZ = aabb3.m_mins.z;
	float maxX = aabb3.m_maxs.x;
	float maxY = aabb3.m_maxs.y;
	float maxZ = aabb3.m_maxs.z;

	//south face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), tints.southTint, sideUVs);
	//north face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), tints.northTint, sideUVs);
	//east face
	AddVertsForQuad3DToVector(vertVector, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), tints.eastTint, sideUVs);
	//west face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), tints.westTint, sideUVs);
	//sky face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), tints.skywardTint, topUVs);
	//hell face
	AddVertsForQuad3DToVector(vertVector, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), tints.hellwardTint, bottomUVs);
}


void AddVertsForCylinderZ3DToVector(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, 
									float numSlices /*= 64.0f*/, Rgba8 const& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float	thetaInterval = (DEGREES_360 / numSlices);
	float	startThetaDegrees = 0.0f;
	Vec3	bottomCircleCenterPosition	= Vec3(centerXY.x, centerXY.y, minMaxZ.m_min);
	Vec3	topCircleCenterPosition		= Vec3(centerXY.x, centerXY.y, minMaxZ.m_max);
	Vec2	circleCenterUV = Vec2(0.5f, 0.5f);

	while (startThetaDegrees < DEGREES_360)
	{
		float endThetaDegrees = startThetaDegrees + thetaInterval;
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta	= CosDegrees(endThetaDegrees);
		float sinEndTheta	= SinDegrees(endThetaDegrees);
		
		//top circle slice
		Vec3 topSegmentStartPoint = topCircleCenterPosition + Vec3(radius * cosStartTheta, radius * sinStartTheta);
		Vec2 topSegmentStartPointUV = Vec2(cosStartTheta, sinStartTheta);
		topSegmentStartPointUV.x = RangeMap(topSegmentStartPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		topSegmentStartPointUV.y = RangeMap(topSegmentStartPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		Vec3 topSegmentEndPoint = topCircleCenterPosition + Vec3(radius * cosEndTheta, radius * sinEndTheta);
		Vec2 topSegmentEndPointUV = Vec2(cosEndTheta, sinEndTheta);
		topSegmentEndPointUV.x = RangeMap(topSegmentEndPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		topSegmentEndPointUV.y = RangeMap(topSegmentEndPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);
		
		vertVector.push_back( Vertex_PCU( topCircleCenterPosition,	tint, circleCenterUV) );
		vertVector.push_back( Vertex_PCU( topSegmentStartPoint,		tint, topSegmentStartPointUV ) );
		vertVector.push_back( Vertex_PCU( topSegmentEndPoint,		tint, topSegmentEndPointUV ) );
		
		//bottom circle slice
		Vec3 bottomSegmentStartPoint = bottomCircleCenterPosition + Vec3(radius * cosStartTheta, radius * sinStartTheta);
		Vec2 bottomSegmentStartPointUV = Vec2(cosStartTheta, sinStartTheta);
		bottomSegmentStartPointUV.x = RangeMap(bottomSegmentStartPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		bottomSegmentStartPointUV.y = RangeMap(bottomSegmentStartPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);
		
		Vec3 bottomSegmentEndPoint = bottomCircleCenterPosition + Vec3(radius * cosEndTheta, radius * sinEndTheta);
		Vec2 bottomSegmentEndPointUV = Vec2(cosEndTheta, sinEndTheta);
		bottomSegmentEndPointUV.x = RangeMap(bottomSegmentEndPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		bottomSegmentEndPointUV.y = RangeMap(bottomSegmentEndPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		vertVector.push_back( Vertex_PCU( bottomCircleCenterPosition,	tint, circleCenterUV ) );
		vertVector.push_back( Vertex_PCU( bottomSegmentEndPoint,		tint, bottomSegmentEndPointUV ) );
		vertVector.push_back( Vertex_PCU( bottomSegmentStartPoint,		tint, bottomSegmentStartPointUV ) );
		
		//middle body slice
		Vec3 middleBodySegmentBottomLeft	= bottomSegmentStartPoint;
		Vec3 middleBodySegmentBottomRight	= bottomSegmentEndPoint;
		Vec3 middleBodySegmentTopLeft		= topSegmentStartPoint;
		Vec3 middleBodySegmentTopRight		= topSegmentEndPoint;

		float leftU = RangeMapClamped(startThetaDegrees, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.y);
		float rightU = RangeMapClamped(endThetaDegrees, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.y);
		AABB2 uvs = AABB2(Vec2(leftU, UVs.m_mins.y), Vec2(rightU, UVs.m_maxs.y));

		Vec2 topRightUV = uvs.m_maxs;
		Vec2 topLeftUV = Vec2(uvs.m_mins.x, uvs.m_maxs.y);
		Vec2 bottomRightUV = Vec2(uvs.m_maxs.x, uvs.m_mins.y);
		Vec2 bottomLeftUV = uvs.m_mins;

		AddVertsForQuad3DToVector(vertVector, middleBodySegmentBottomLeft, middleBodySegmentBottomRight, middleBodySegmentTopLeft, middleBodySegmentTopRight, tint, uvs);

		startThetaDegrees += thetaInterval;
	}
}


void AddVertsForCylinderZ3DToVector(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float bottomRadius, float topRadius, float numSlices /*= 64.0f*/, Rgba8 const& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float	thetaInterval = (DEGREES_360 / numSlices);
	float	startThetaDegrees = 0.0f;
	Vec3	bottomCircleCenterPosition = Vec3(centerXY.x, centerXY.y, minMaxZ.m_min);
	Vec3	topCircleCenterPosition = Vec3(centerXY.x, centerXY.y, minMaxZ.m_max);
	Vec2	circleCenterUV = Vec2(0.5f, 0.5f);

	while (startThetaDegrees < DEGREES_360)
	{
		float endThetaDegrees = startThetaDegrees + thetaInterval;
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		//top circle slice
		Vec3 topSegmentStartPoint = topCircleCenterPosition + Vec3(topRadius * cosStartTheta, topRadius * sinStartTheta);
		Vec2 topSegmentStartPointUV = Vec2(cosStartTheta, sinStartTheta);
		topSegmentStartPointUV.x = RangeMap(topSegmentStartPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		topSegmentStartPointUV.y = RangeMap(topSegmentStartPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		Vec3 topSegmentEndPoint = topCircleCenterPosition + Vec3(topRadius * cosEndTheta, topRadius * sinEndTheta);
		Vec2 topSegmentEndPointUV = Vec2(cosEndTheta, sinEndTheta);
		topSegmentEndPointUV.x = RangeMap(topSegmentEndPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		topSegmentEndPointUV.y = RangeMap(topSegmentEndPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		vertVector.push_back(Vertex_PCU(topCircleCenterPosition, tint, circleCenterUV));
		vertVector.push_back(Vertex_PCU(topSegmentStartPoint, tint, topSegmentStartPointUV));
		vertVector.push_back(Vertex_PCU(topSegmentEndPoint, tint, topSegmentEndPointUV));

		//bottom circle slice
		Vec3 bottomSegmentStartPoint = bottomCircleCenterPosition + Vec3(bottomRadius * cosStartTheta, bottomRadius * sinStartTheta);
		Vec2 bottomSegmentStartPointUV = Vec2(cosStartTheta, sinStartTheta);
		bottomSegmentStartPointUV.x = RangeMap(bottomSegmentStartPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		bottomSegmentStartPointUV.y = RangeMap(bottomSegmentStartPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		Vec3 bottomSegmentEndPoint = bottomCircleCenterPosition + Vec3(bottomRadius * cosEndTheta, bottomRadius * sinEndTheta);
		Vec2 bottomSegmentEndPointUV = Vec2(cosEndTheta, sinEndTheta);
		bottomSegmentEndPointUV.x = RangeMap(bottomSegmentEndPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		bottomSegmentEndPointUV.y = RangeMap(bottomSegmentEndPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		vertVector.push_back(Vertex_PCU(bottomCircleCenterPosition, tint, circleCenterUV));
		vertVector.push_back(Vertex_PCU(bottomSegmentEndPoint, tint, bottomSegmentEndPointUV));
		vertVector.push_back(Vertex_PCU(bottomSegmentStartPoint, tint, bottomSegmentStartPointUV));

		//middle body slice
		Vec3 middleBodySegmentBottomLeft = bottomSegmentStartPoint;
		Vec3 middleBodySegmentBottomRight = bottomSegmentEndPoint;
		Vec3 middleBodySegmentTopLeft = topSegmentStartPoint;
		Vec3 middleBodySegmentTopRight = topSegmentEndPoint;

		float leftU = RangeMapClamped(startThetaDegrees, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.y);
		float rightU = RangeMapClamped(endThetaDegrees, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.y);
		AABB2 uvs = AABB2(Vec2(leftU, UVs.m_mins.y), Vec2(rightU, UVs.m_maxs.y));

		Vec2 topRightUV = uvs.m_maxs;
		Vec2 topLeftUV = Vec2(uvs.m_mins.x, uvs.m_maxs.y);
		Vec2 bottomRightUV = Vec2(uvs.m_maxs.x, uvs.m_mins.y);
		Vec2 bottomLeftUV = uvs.m_mins;

		AddVertsForQuad3DToVector(vertVector, middleBodySegmentBottomLeft, middleBodySegmentBottomRight, middleBodySegmentTopLeft, middleBodySegmentTopRight, tint, uvs);

		startThetaDegrees += thetaInterval;
	}
}


void AddVertsForCylinderZ3DToVector(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices /*= 64.0f*/, CylinderFaceTints faceTints /*= {}*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float	thetaInterval = (DEGREES_360 / numSlices);
	float	startThetaDegrees = 0.0f;
	Vec3	bottomCircleCenterPosition = Vec3(centerXY.x, centerXY.y, minMaxZ.m_min);
	Vec3	topCircleCenterPosition = Vec3(centerXY.x, centerXY.y, minMaxZ.m_max);
	Vec2	circleCenterUV = Vec2(0.5f, 0.5f);

	while (startThetaDegrees < DEGREES_360)
	{
		float endThetaDegrees = startThetaDegrees + thetaInterval;
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		//top circle slice
		Vec3 topSegmentStartPoint = topCircleCenterPosition + Vec3(radius * cosStartTheta, radius * sinStartTheta);
		Vec2 topSegmentStartPointUV = Vec2(cosStartTheta, sinStartTheta);
		topSegmentStartPointUV.x = RangeMap(topSegmentStartPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		topSegmentStartPointUV.y = RangeMap(topSegmentStartPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		Vec3 topSegmentEndPoint = topCircleCenterPosition + Vec3(radius * cosEndTheta, radius * sinEndTheta);
		Vec2 topSegmentEndPointUV = Vec2(cosEndTheta, sinEndTheta);
		topSegmentEndPointUV.x = RangeMap(topSegmentEndPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		topSegmentEndPointUV.y = RangeMap(topSegmentEndPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		vertVector.push_back(Vertex_PCU(topCircleCenterPosition, faceTints.topFaceTint, circleCenterUV));
		vertVector.push_back(Vertex_PCU(topSegmentStartPoint, faceTints.topFaceTint, topSegmentStartPointUV));
		vertVector.push_back(Vertex_PCU(topSegmentEndPoint, faceTints.topFaceTint, topSegmentEndPointUV));

		//bottom circle slice
		Vec3 bottomSegmentStartPoint = bottomCircleCenterPosition + Vec3(radius * cosStartTheta, radius * sinStartTheta);
		Vec2 bottomSegmentStartPointUV = Vec2(cosStartTheta, sinStartTheta);
		bottomSegmentStartPointUV.x = RangeMap(bottomSegmentStartPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		bottomSegmentStartPointUV.y = RangeMap(bottomSegmentStartPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		Vec3 bottomSegmentEndPoint = bottomCircleCenterPosition + Vec3(radius * cosEndTheta, radius * sinEndTheta);
		Vec2 bottomSegmentEndPointUV = Vec2(cosEndTheta, sinEndTheta);
		bottomSegmentEndPointUV.x = RangeMap(bottomSegmentEndPointUV.x, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x);
		bottomSegmentEndPointUV.y = RangeMap(bottomSegmentEndPointUV.y, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y);

		vertVector.push_back(Vertex_PCU(bottomCircleCenterPosition, faceTints.bottomFaceTint, circleCenterUV));
		vertVector.push_back(Vertex_PCU(bottomSegmentEndPoint, faceTints.bottomFaceTint, bottomSegmentEndPointUV));
		vertVector.push_back(Vertex_PCU(bottomSegmentStartPoint, faceTints.bottomFaceTint, bottomSegmentStartPointUV));

		//middle body slice
		Vec3 middleBodySegmentBottomLeft = bottomSegmentStartPoint;
		Vec3 middleBodySegmentBottomRight = bottomSegmentEndPoint;
		Vec3 middleBodySegmentTopLeft = topSegmentStartPoint;
		Vec3 middleBodySegmentTopRight = topSegmentEndPoint;

		float leftU = RangeMapClamped(startThetaDegrees, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.y);
		float rightU = RangeMapClamped(endThetaDegrees, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.y);
		AABB2 uvs = AABB2(Vec2(leftU, UVs.m_mins.y), Vec2(rightU, UVs.m_maxs.y));

		Vec2 topRightUV = uvs.m_maxs;
		Vec2 topLeftUV = Vec2(uvs.m_mins.x, uvs.m_maxs.y);
		Vec2 bottomRightUV = Vec2(uvs.m_maxs.x, uvs.m_mins.y);
		Vec2 bottomLeftUV = uvs.m_mins;

		AddVertsForQuad3DToVector(vertVector, middleBodySegmentBottomLeft, middleBodySegmentBottomRight, middleBodySegmentTopLeft, middleBodySegmentTopRight, faceTints.sideFaceTint, uvs);

		startThetaDegrees += thetaInterval;
	}
}


void AddVertsForUVSphereZ3DToVector(std::vector<Vertex_PCU>& vertVector, Vec3 const& center, float radius, 
									float numSlices /*= 64.0f*/, float numStacks /*= 32.0f*/, Rgba8 const& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float yawInterval = DEGREES_360 / numSlices;
	float startingYaw = 0.0f;
	float pitchInterval = DEGREES_180 / numStacks;
	float startingPitch = DEGREES_90;

	for (float yaw = startingYaw; yaw < DEGREES_360; yaw += yawInterval)
	{
		for (float pitch = startingPitch; pitch >= -DEGREES_90; pitch -= pitchInterval)
		{
			float quadBottomPitch = pitch;
			float quadTopPitch = pitch - pitchInterval;
			float quadLeftYaw = yaw;
			float quadRightYaw = yaw + yawInterval;

			float quadLeftU = RangeMap(quadLeftYaw, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.x);
			float quadRightU = RangeMap(quadRightYaw, 0.0f, DEGREES_360, UVs.m_mins.x, UVs.m_maxs.x);
			float quadBottomV = RangeMap(quadBottomPitch, DEGREES_90, -DEGREES_90, UVs.m_mins.y, UVs.m_maxs.y);
			float quadTopV = RangeMap(quadTopPitch, DEGREES_90, -DEGREES_90, UVs.m_mins.y, UVs.m_maxs.y);
			AABB2 quadUVs = AABB2(Vec2(quadLeftU, quadBottomV), Vec2(quadRightU, quadTopV));

			Vec3 bottomLeftFwdVec	= Vec3(CosDegrees(quadBottomPitch) * CosDegrees(quadLeftYaw), SinDegrees(quadLeftYaw) * CosDegrees(quadBottomPitch), -SinDegrees(quadBottomPitch));
			Vec3 bottomRightFwdVec	= Vec3(CosDegrees(quadBottomPitch) * CosDegrees(quadRightYaw), SinDegrees(quadRightYaw) * CosDegrees(quadBottomPitch), -SinDegrees(quadBottomPitch));;
			Vec3 topLeftFwdVec		= Vec3(CosDegrees(quadTopPitch) * CosDegrees(quadLeftYaw), SinDegrees(quadLeftYaw) * CosDegrees(quadTopPitch), -SinDegrees(quadTopPitch));;
			Vec3 topRightFwdVec		= Vec3(CosDegrees(quadTopPitch) * CosDegrees(quadRightYaw), SinDegrees(quadRightYaw) * CosDegrees(quadTopPitch), -SinDegrees(quadTopPitch));;

			Vec3 bottomLeft	 = center + radius * bottomLeftFwdVec;
			Vec3 bottomRight = center + radius * bottomRightFwdVec;
			Vec3 topLeft	 = center + radius * topLeftFwdVec;
			Vec3 topRight	 = center + radius * topRightFwdVec;

			AddVertsForQuad3DToVector(vertVector, bottomLeft, bottomRight, topLeft, topRight, tint, quadUVs);
		}
	}
}


constexpr float WIRE_LINE_THICKNESS = 0.008f;
constexpr float WIRE_LINE_PADDING = 0.00f;

void AddVertsForWireQuad3DToVector(std::vector<Vertex_PCU>& out_vector, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, float wireThickness, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	AddVertsForLine3DToVector(out_vector, bottomLeft,	bottomRight,	wireThickness, WIRE_LINE_PADDING, color);
	AddVertsForLine3DToVector(out_vector, bottomLeft,	topLeft,		wireThickness, WIRE_LINE_PADDING, color);
	AddVertsForLine3DToVector(out_vector, bottomRight,	topRight,		wireThickness, WIRE_LINE_PADDING, color);
	AddVertsForLine3DToVector(out_vector, topLeft,		topRight,		wireThickness, WIRE_LINE_PADDING, color);
}


void AddVertsForWireAABBZ3DToVector(std::vector<Vertex_PCU>& out_Vector, AABB3 const& aabb3, Rgba8 color /*= Rgba8::WHITE*/)
{
	//cube mins and maxs
	float minX = aabb3.m_mins.x;
	float minY = aabb3.m_mins.y;
	float minZ = aabb3.m_mins.z;
	float maxX = aabb3.m_maxs.x;
	float maxY = aabb3.m_maxs.y;
	float maxZ = aabb3.m_maxs.z;

	//south face
	AddVertsForWireQuad3DToVector(out_Vector, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), WIRE_LINE_THICKNESS, color);

	//east face											  
	AddVertsForWireQuad3DToVector(out_Vector, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), WIRE_LINE_THICKNESS, color);

	//west face			  			  						  
	AddVertsForWireQuad3DToVector(out_Vector, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), WIRE_LINE_THICKNESS, color);

	//upward face			  			  						  
	AddVertsForWireQuad3DToVector(out_Vector, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), WIRE_LINE_THICKNESS, color);

	//down face		  			  						  
	AddVertsForWireQuad3DToVector(out_Vector, Vec3(maxX, minY, minZ), Vec3(minX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), WIRE_LINE_THICKNESS, color);

	//north face			  			  						  
	AddVertsForWireQuad3DToVector(out_Vector, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), WIRE_LINE_THICKNESS, color);
}


void AddVertsForWireSphereZ3DToVector(std::vector<Vertex_PCU>& vertVector, Vec3 const& center, float radius, float numSlices /*= 64.0f*/, float numStacks /*= 32.0f*/, Rgba8 const& tint /*= Rgba8::WHITE*/)
{
	float yawInterval = DEGREES_360 / numSlices;
	float startingYaw = 0.0f;
	float pitchInterval = DEGREES_180 / numStacks;
	float startingPitch = DEGREES_90;

	for (float yaw = startingYaw; yaw < DEGREES_360; yaw += yawInterval)
	{
		for (float pitch = startingPitch; pitch >= -DEGREES_90; pitch -= pitchInterval)
		{
			float quadBottomPitch = pitch;
			float quadTopPitch = pitch - pitchInterval;
			float quadLeftYaw = yaw;
			float quadRightYaw = yaw + yawInterval;

			Vec3 bottomLeftFwdVec = Vec3(CosDegrees(quadBottomPitch) * CosDegrees(quadLeftYaw), SinDegrees(quadLeftYaw) * CosDegrees(quadBottomPitch), -SinDegrees(quadBottomPitch));
			Vec3 bottomRightFwdVec = Vec3(CosDegrees(quadBottomPitch) * CosDegrees(quadRightYaw), SinDegrees(quadRightYaw) * CosDegrees(quadBottomPitch), -SinDegrees(quadBottomPitch));;
			Vec3 topLeftFwdVec = Vec3(CosDegrees(quadTopPitch) * CosDegrees(quadLeftYaw), SinDegrees(quadLeftYaw) * CosDegrees(quadTopPitch), -SinDegrees(quadTopPitch));;
			Vec3 topRightFwdVec = Vec3(CosDegrees(quadTopPitch) * CosDegrees(quadRightYaw), SinDegrees(quadRightYaw) * CosDegrees(quadTopPitch), -SinDegrees(quadTopPitch));;

			Vec3 bottomLeft = center + radius * bottomLeftFwdVec;
			Vec3 bottomRight = center + radius * bottomRightFwdVec;
			Vec3 topLeft = center + radius * topLeftFwdVec;
			Vec3 topRight = center + radius * topRightFwdVec;

			AddVertsForWireQuad3DToVector(vertVector, bottomLeft, bottomRight, topLeft, topRight, WIRE_LINE_THICKNESS, tint);
		}
	}
}


void AddVertsForWireCylinderZ3DToVector(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices /*= 64.0f*/, Rgba8 const& tint /*= Rgba8::WHITE*/)
{
	float	thetaInterval = (DEGREES_360 / numSlices);
	float	startThetaDegrees = 0.0f;
	Vec3	bottomCircleCenterPosition = Vec3(centerXY.x, centerXY.y, minMaxZ.m_min);
	Vec3	topCircleCenterPosition = Vec3(centerXY.x, centerXY.y, minMaxZ.m_max);
	Vec2	circleCenterUV = Vec2(0.5f, 0.5f);

	while (startThetaDegrees < DEGREES_360)
	{
		float endThetaDegrees = startThetaDegrees + thetaInterval;
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		//top circle slice
		Vec3 topSegmentStartPoint = topCircleCenterPosition + Vec3(radius * cosStartTheta, radius * sinStartTheta);
		Vec3 topSegmentEndPoint = topCircleCenterPosition + Vec3(radius * cosEndTheta, radius * sinEndTheta);

		AddVertsForLine3DToVector(vertVector, topCircleCenterPosition,	topSegmentStartPoint,	WIRE_LINE_THICKNESS, WIRE_LINE_PADDING, tint);
		AddVertsForLine3DToVector(vertVector, topCircleCenterPosition,	topSegmentEndPoint,		WIRE_LINE_THICKNESS, WIRE_LINE_PADDING, tint);
		AddVertsForLine3DToVector(vertVector, topSegmentEndPoint,		topSegmentStartPoint,	WIRE_LINE_THICKNESS, WIRE_LINE_PADDING, tint);

		//bottom circle slice
		Vec3 bottomSegmentStartPoint = bottomCircleCenterPosition + Vec3(radius * cosStartTheta, radius * sinStartTheta);
		Vec3 bottomSegmentEndPoint = bottomCircleCenterPosition + Vec3(radius * cosEndTheta, radius * sinEndTheta);
		
		AddVertsForLine3DToVector(vertVector, bottomCircleCenterPosition,	bottomSegmentStartPoint,	WIRE_LINE_THICKNESS, WIRE_LINE_PADDING, tint);
		AddVertsForLine3DToVector(vertVector, bottomCircleCenterPosition,	bottomSegmentEndPoint,		WIRE_LINE_THICKNESS, WIRE_LINE_PADDING, tint);
		AddVertsForLine3DToVector(vertVector, bottomSegmentEndPoint,		bottomSegmentStartPoint,	WIRE_LINE_THICKNESS, WIRE_LINE_PADDING, tint);

		//middle body slice
		Vec3 middleBodySegmentBottomLeft = bottomSegmentStartPoint;
		Vec3 middleBodySegmentBottomRight = bottomSegmentEndPoint;
		Vec3 middleBodySegmentTopLeft = topSegmentStartPoint;
		Vec3 middleBodySegmentTopRight = topSegmentEndPoint;

		AddVertsForWireQuad3DToVector(vertVector, middleBodySegmentBottomLeft, middleBodySegmentBottomRight, middleBodySegmentTopLeft, middleBodySegmentTopRight, WIRE_LINE_THICKNESS, tint);

		startThetaDegrees += thetaInterval;
	}
}


void TransformVertexArrayColor(int numVertexes, Vertex_PCU* verts, Rgba8 const& newColor)
{
	for (int vertIndex = 0; vertIndex < numVertexes; vertIndex++)
	{
		Vertex_PCU& vert = verts[vertIndex];
		vert.m_color = newColor;
	}
}
