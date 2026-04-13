#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/D3D11Headers.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/EngineBuildPreferences.hpp"
#include<string>

#include<d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


bool Shader::IsValid() const
{
	return m_vertexByteCode.size() > 0;
}


std::string const& Shader::GetName() const
{
	return m_config.m_name;
}


Shader::Shader()
{

}

Shader::~Shader()
{

}


bool Shader::Create(Renderer* renderer, ShaderConfig const& config)
{
	bool result;
	m_config = config;

	if (m_config.m_sourceCode == "NULL")
	{
		std::string shaderPath = m_config.m_name + ".hlsl";
		std::string shaderSourceCode;
		FileReadToString(shaderSourceCode, shaderPath);
		m_config.m_sourceCode = shaderSourceCode;
	}

	result = CompileAndCreateVertexShader(renderer, m_config.m_sourceCode);
	result &= CompileAndCreatePixelShader(renderer, m_config.m_sourceCode);

	return result;
}


void Shader::Destroy()
{
	m_vertexByteCode.clear();

	D3X_SAFE_RELEASE(m_vertexStage);
	D3X_SAFE_RELEASE(m_pixelStage);
	D3X_SAFE_RELEASE(m_inputLayout_VertexPCU);
}


bool Shader::CompileAndCreateVertexShader(Renderer* renderer, std::string const sourceCode)
{
	HRESULT result;

	DWORD compileFlags = 0;

#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
	compileFlags |= D3DCOMPILE_DEBUG;
	compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob* byteCodeBuffer;
	ID3DBlob* errorCodeBuffer;

	//compile shader
	result = D3DCompile
	(
		sourceCode.c_str(),
		sourceCode.size(),
		m_config.m_name.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		m_config.m_vertexEntryPoint.c_str(),
		"vs_5_0",
		compileFlags,
		0,
		&byteCodeBuffer,
		&errorCodeBuffer
	);

	if (SUCCEEDED(result) && byteCodeBuffer != nullptr)
	{
		//create shader
		ID3D11Device* device = renderer->GetDevice();
		result = device->CreateVertexShader
		(
			byteCodeBuffer->GetBufferPointer(),
			byteCodeBuffer->GetBufferSize(),
			nullptr,
			&m_vertexStage
		);

		//store compiled byte data
		size_t bufferSize = byteCodeBuffer->GetBufferSize();
		m_vertexByteCode.resize(bufferSize);
		uint8_t* bufferHead = (uint8_t*) byteCodeBuffer->GetBufferPointer();
		for (int byteIndex = 0; byteIndex < (int) bufferSize; byteIndex++)
		{
			m_vertexByteCode[byteIndex] = *(bufferHead + byteIndex);
		}

		if (SUCCEEDED(result))
			return true;
		else
			ERROR_RECOVERABLE("Unable to create vertex shader: " + m_config.m_name);
	}
	else
	{
		char const* errorString = (const char*) errorCodeBuffer->GetBufferPointer();
		if (errorCodeBuffer != nullptr)
		{
			DebuggerPrintf(errorString);
			ERROR_RECOVERABLE("Unable to compile vertex shader: " + m_config.m_name);
		}
	}

	D3X_SAFE_RELEASE(byteCodeBuffer);
	D3X_SAFE_RELEASE(errorCodeBuffer);
	return false;
}


bool Shader::CompileAndCreatePixelShader(Renderer* renderer, std::string const sourceCode)
{
	HRESULT result;

	DWORD compileFlags = 0;

#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
	compileFlags |= D3DCOMPILE_DEBUG;
	compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob* byteCodeBuffer;
	ID3DBlob* errorCodeBuffer;

	//compile pixel shader
	result = D3DCompile(
		sourceCode.c_str(),
		sourceCode.size(),
		m_config.m_name.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		m_config.m_pixelEntryPoint.c_str(),
		"ps_5_0",
		compileFlags,
		0,
		&byteCodeBuffer,
		&errorCodeBuffer
	);

	if (SUCCEEDED(result) && byteCodeBuffer != nullptr)
	{
		//create pixel shader
		ID3D11Device* device = renderer->GetDevice();
		result = device->CreatePixelShader
		(
			byteCodeBuffer->GetBufferPointer(),
			byteCodeBuffer->GetBufferSize(),
			nullptr,
			&m_pixelStage
		);

		if (SUCCEEDED(result))
			return true;
		else
			ERROR_RECOVERABLE("Unable to create pixel shader: " + m_config.m_name);
	}
	else
	{
		char const* errorString = (const char*) errorCodeBuffer->GetBufferPointer();
		if (errorCodeBuffer != nullptr)
		{
			DebuggerPrintf(errorString);
			ERROR_RECOVERABLE("Unable to compile pixel shader: " + m_config.m_name);
		}
	}

	D3X_SAFE_RELEASE(byteCodeBuffer);
	D3X_SAFE_RELEASE(errorCodeBuffer);
	return false;
}


ID3D11InputLayout* Shader::CreateOrGetInputLayout_VertexPCU(Renderer* renderer)
{
	if (m_inputLayout_VertexPCU)
		return m_inputLayout_VertexPCU;

	ID3D11Device* device = renderer->GetDevice();
	
	D3D11_INPUT_ELEMENT_DESC inputDesc[3];
	inputDesc[0].SemanticName = "POSITION";
	inputDesc[0].SemanticIndex = 0;
	inputDesc[0].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[0].InputSlot = 0;
	inputDesc[0].AlignedByteOffset = offsetof(Vertex_PCU, m_position);
	inputDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputDesc[0].InstanceDataStepRate = 0;

	inputDesc[1].SemanticName = "TINT";
	inputDesc[1].SemanticIndex = 0;
	inputDesc[1].Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	inputDesc[1].InputSlot = 0;
	inputDesc[1].AlignedByteOffset = offsetof(Vertex_PCU, m_color);
	inputDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;

	inputDesc[2].SemanticName = "TEXCOORD";
	inputDesc[2].SemanticIndex = 0;
	inputDesc[2].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
	inputDesc[2].InputSlot = 0;
	inputDesc[2].AlignedByteOffset = offsetof(Vertex_PCU, m_uvTexCoords);
	inputDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputDesc[2].InstanceDataStepRate = 0;

	HRESULT hr = device->CreateInputLayout(inputDesc, 
										   3, 
										   &m_vertexByteCode[0], 
										   m_vertexByteCode.size(), 
										   &m_inputLayout_VertexPCU);

	ASSERT_RECOVERABLE(SUCCEEDED(hr), "Unable to create input layout for shader: " + m_config.m_name);
	return m_inputLayout_VertexPCU;
}
