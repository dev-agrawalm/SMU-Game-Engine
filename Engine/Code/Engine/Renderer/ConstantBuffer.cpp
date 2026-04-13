#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/D3D11Headers.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

bool ConstantBuffer::SetData(void const* data, size_t byteCount)
{
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
	return true;
}


ConstantBuffer::ConstantBuffer(Renderer* source, size_t const maxSize)
{
	ID3D11Device* device = source->GetDevice();

	D3D11_BUFFER_DESC bufferDesc = {0};
	bufferDesc.ByteWidth = (UINT) maxSize;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&bufferDesc, nullptr, &m_gpuBufferHandle);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to create constant buffer");

	m_sourceRenderer = source;
	m_maxSize = maxSize;
}


ConstantBuffer::~ConstantBuffer()
{

}


ID3D11Buffer* ConstantBuffer::GetHandle()
{
	return m_gpuBufferHandle;
}
