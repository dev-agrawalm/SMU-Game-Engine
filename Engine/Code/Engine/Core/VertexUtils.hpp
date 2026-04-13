#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include<vector>

struct AABB3;
struct Capsule2;
struct LineSegment2;
struct OBB2;
struct FloatRange;
struct Mat44;
struct QuadraticBezierCurve2D;
struct CubicBezierCurve2D;
struct CubicHermiteCurve2D;
struct ConvexPoly2;

//scales, rotates and translates the positions of vertexes in a given array of vertexes
void TransformVertexArrayXY3D(int numVertexes, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArrayXY3D(int numVertexes, Vertex_PCU* verts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY);
void TransformVertexArray3D(int numVertexes, Vertex_PCU* verts, Mat44 const& modelMatrix); 

//2d utilities
void AddVertsForAABB2ToVector		 (std::vector<Vertex_PCU>& vertVector, AABB2 const& aabb2, Rgba8 const& color, Vec2 const& uvMins = Vec2::ZERO, Vec2 const& uvMaxs = Vec2::ONE);
void AddVertsForDisk2DToVector		 (std::vector<Vertex_PCU>& vertVector, Vec2 const& diskCenter, float diskRadius, Rgba8 const& color, int numSegments = 30);
void AddVertsForCapsule2DToVector	 (std::vector<Vertex_PCU>& vertVector, Capsule2 const& capsule2d, Rgba8 const color, int numSegmentsPerHemisphere = 16);
void AddVertsForOBB2DToVector		 (std::vector<Vertex_PCU>& vertVector, OBB2 const& obb2, Rgba8 const color);
void AddVertsForLineSegment2DToVector(std::vector<Vertex_PCU>& vertVector, LineSegment2 const& line2d, Rgba8 const& color, float width = 1.0f, float padding = 5.0f);
void AddVertsForArrow2DToVector		 (std::vector<Vertex_PCU>& vertVector, Vec2 const& arrowStart, Vec2 const& arrowEnd, Rgba8 const& color, float width = 1.0f, float arrowTipLength = 1.0f, float padding = 5.0f);
void AddVertsForQuadraticBezierCurve2DToVector(std::vector<Vertex_PCU>& vertVector, QuadraticBezierCurve2D const& qbCurve, Rgba8 const& color = Rgba8::WHITE, float width = 1.0f, float padding = 5.0f, int numSegments = 60.0f);
void AddVertsForCubicBezierCurve2DToVector(std::vector<Vertex_PCU>& vertVector, CubicBezierCurve2D const& cbCurve, Rgba8 const& color = Rgba8::WHITE, float width = 1.0f, float padding = 5.0f, int numSegments = 60.0f);
void AddVertsForCubicHermiteCurve2DToVector(std::vector<Vertex_PCU>& vertVector, CubicHermiteCurve2D const& hermiteCurve, Rgba8 const& color = Rgba8::WHITE, float width = 1.0f, float padding = 5.0f, int numSegments = 60.0f);
void AddVertsForConvexPoly2ToVector(std::vector<Vertex_PCU>& vertVector, ConvexPoly2 const& convexPoly2, Rgba8 const& color);


struct CubeFaceTints
{
	Rgba8 southTint		= Rgba8::WHITE;
	Rgba8 northTint		= Rgba8::WHITE;
	Rgba8 westTint		= Rgba8::WHITE;
	Rgba8 eastTint		= Rgba8::WHITE;
	Rgba8 skywardTint	= Rgba8::WHITE;
	Rgba8 hellwardTint	= Rgba8::WHITE;
};


//3d utilities
void AddVertsForLine3DToVector		(std::vector<Vertex_PCU>& out_vector, Vec3 const& start, Vec3 const& end, float thickness = 1.0f, float padding = 0.05f, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForQuad3DToVector		(std::vector<Vertex_PCU>& out_vector, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABBZ3DToVector		(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABBZ3DToVector		(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, CubeFaceTints const& tints, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABBZ3DToVector		(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, CubeFaceTints const& tints, AABB2 const& topUVs, AABB2 const& sideUVs, AABB2 const& bottomUVs);
void AddVertsForAABBZ3DToVector		(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, Rgba8 const& topTint, Rgba8 const& sideTint, Rgba8 const& bottomTint, AABB2 const& topUVs, AABB2 const& sideUVs, AABB2 const& bottomUVs);
void AddVertsForAABBZ3DToVector		(std::vector<Vertex_PCU>& vertVector, AABB3 const& aabb3, Rgba8 const& tint, AABB2 const& topUVs, AABB2 const& sideUVs, AABB2 const& bottomUVs);
void AddVertsForCylinderZ3DToVector	(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices = 64.0f, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
struct CylinderFaceTints
{
	Rgba8 topFaceTint = Rgba8::WHITE;
	Rgba8 sideFaceTint = Rgba8::WHITE;
	Rgba8 bottomFaceTint = Rgba8::WHITE;
};

void AddVertsForCylinderZ3DToVector(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices = 64.0f, CylinderFaceTints faceTints = {}, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinderZ3DToVector	(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float bottomRadius, float topRadius, float numSlices = 64.0f, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForUVSphereZ3DToVector	(std::vector<Vertex_PCU>& vertVector, Vec3 const& center, float radius, float numSlices = 64.0f, float numStacks = 32.0f, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);

//wireframe utilities
void AddVertsForWireQuad3DToVector		(std::vector<Vertex_PCU>& out_vector, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, float wireThickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForWireAABBZ3DToVector		(std::vector<Vertex_PCU>& out_Vector, AABB3 const& aabb3, Rgba8 color = Rgba8::WHITE);
void AddVertsForWireSphereZ3DToVector	(std::vector<Vertex_PCU>& vertVector, Vec3 const& center, float radius, float numSlices = 64.0f, float numStacks = 32.0f, Rgba8 const& tint = Rgba8::WHITE);
void AddVertsForWireCylinderZ3DToVector	(std::vector<Vertex_PCU>& vertVector, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices = 64.0f, Rgba8 const& tint = Rgba8::WHITE);

//misc utilities
void TransformVertexArrayColor(int numVertexes, Vertex_PCU* verts, Rgba8 const& newColor);
