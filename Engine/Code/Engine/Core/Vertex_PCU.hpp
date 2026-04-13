#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

//struct represents a single point(vertex) of a simple 3D object intended to be rendered
struct Vertex_PCU
{
public: //members
	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;

public: //methods
	Vertex_PCU(); // does nothing
	~Vertex_PCU(); // does nothing
	explicit Vertex_PCU(Vec3 const& position, Rgba8 const& tint, Vec2 const& uvTexCoords = Vec2(0.0f,0.0f)); //explicit constructor for a vertex
};