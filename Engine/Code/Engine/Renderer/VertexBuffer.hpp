#pragma once
#include "Engine/Core/Vertex_PCU.hpp"


// predefines
struct ID3D11Buffer; 
class Renderer;

//------------------------------------------------------------------------------------------------
class VertexBuffer
{
	friend class Renderer; // Only the Renderer can create new Texture objects!

public:
	void CopyVertexData( void const* data, size_t byteCount ); // a lot like constant buffer, except...

	// note, could be a template
	template <typename STRUCT_TYPE>
	void CopyVertexArray(STRUCT_TYPE const* vertices, int numVertexes)
	{
		m_vertexSize = sizeof(STRUCT_TYPE);
		CopyVertexData(vertices, numVertexes * sizeof(STRUCT_TYPE));
	}

	// size of a single element or vertex (just vertex pcu for now, but if we support different vertex types, this will change); 
	inline size_t GetStride() const			{ return m_vertexSize; }

protected:
	VertexBuffer( Renderer* source ); // can't instantiate directly; must ask Renderer to do it for you
	VertexBuffer( VertexBuffer const& copy ) = delete; // No copying allowed!  This represents GPU memory.
	virtual ~VertexBuffer();

	ID3D11Buffer* GetHandle() const;  

	// again, add other helpers as you see fit...
	// ...

protected:
	Renderer* m_sourceRenderer				= nullptr;	// who created me
	ID3D11Buffer* m_gpuBufferHandle			= nullptr;	// d3d11 handle

	size_t m_vertexSize						= 0; // size of a single vertx
	size_t m_maxVertexCount					= 0; // max number of vertices allowed
};

