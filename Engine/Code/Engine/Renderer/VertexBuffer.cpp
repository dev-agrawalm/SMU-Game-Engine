#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/D3D11Headers.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"



VertexBuffer::VertexBuffer(Renderer* source)
{
	ID3D11Device* device = source->GetDevice();

	D3D11_BUFFER_DESC bufferDesc = {0};
	bufferDesc.ByteWidth = 1;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&bufferDesc, nullptr, &m_gpuBufferHandle);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to create vertex buffer");

	m_maxVertexCount = 1;
	m_sourceRenderer = source;
}


VertexBuffer::~VertexBuffer()
{

}


ID3D11Buffer* VertexBuffer::GetHandle() const
{
	return m_gpuBufferHandle;
}


void VertexBuffer::CopyVertexData(void const* data, size_t byteCount)
{
	if (byteCount > m_maxVertexCount * sizeof(Vertex_PCU))
	{
		D3X_SAFE_RELEASE(m_gpuBufferHandle);
		m_maxVertexCount = byteCount / sizeof(Vertex_PCU);

		D3D11_BUFFER_DESC bufferDesc = {0};
		bufferDesc.ByteWidth = (UINT) byteCount;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		HRESULT result = m_sourceRenderer->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_gpuBufferHandle);
		ASSERT_OR_DIE(SUCCEEDED(result), "Failed to create vertex buffer");
	}

	ID3D11DeviceContext* context = m_sourceRenderer->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE mapping; //essentially the memory on the gpu
	HRESULT result = context->Map(m_gpuBufferHandle,
								  0,
								  D3D11_MAP_WRITE_DISCARD,
								  0,
								  &mapping);

	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to map buffer for write");

	memcpy(mapping.pData, data, byteCount);
	context->Unmap(m_gpuBufferHandle, 0);
}
