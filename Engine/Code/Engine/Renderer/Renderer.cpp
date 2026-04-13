#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/D3D11Headers.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Embedded/DefaultShaderSource.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"

#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
#include "dxgidebug.h"
#endif
#pragma comment(lib, "dxguid.lib")


bool OnClearDebugRender(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return false;
}


bool OnToggleVisibilityDebugRender(EventArgs& args)
{
	UNUSED(args);
	DebugRenderToggleVisibility();
	return false;
}


bool OnSetDebugRenderTimeScale(EventArgs& args)
{
	float timeScale = 0.0f;
	args.GetProperty("scale",timeScale, 1.0f);
	DebugRenderSetTimeScale((double) timeScale);
	return true;
}


Renderer::Renderer(RenderConfig const& config)
	: m_renderConfig(config)
{
	m_blendStatesByMode[(int) BlendMode::ADDITIVE] = nullptr;
	m_blendStatesByMode[(int) BlendMode::ALPHA] = nullptr;
	m_blendStatesByMode[(int) BlendMode::OPAQUE] = nullptr;

	m_samplerStatesByMode[(int) SamplingMode::LINEAR_CLAMP] = nullptr;
	m_samplerStatesByMode[(int) SamplingMode::LINEAR_WRAP] = nullptr;
	m_samplerStatesByMode[(int) SamplingMode::POINT_CLAMP] = nullptr;
	m_samplerStatesByMode[(int) SamplingMode::POINT_WRAP] = nullptr;
}


Renderer::~Renderer()
{
}


void Renderer::Startup()
{
	CreateDebugLayer();

	CreateRenderContext();
	CreateBackbufferRenderTargetView();

	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	m_immediateVBO = CreateDynamicVertexBuffer();

	CreateDefaultShader();
	CreateDefaultDepthStencil();
	CreateDefaultDiffuseTexture();
	InitialiseBlendState();
	InitialiseSamplerStates();

	InitialiseDebugRenderSystem();
}


void Renderer::ReportLiveObjects()
{
#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
	if (m_debug != nullptr)
	{
		DXGI_DEBUG_RLO_FLAGS const flags = (DXGI_DEBUG_RLO_FLAGS) (DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
		m_debug->ReportLiveObjects(DXGI_DEBUG_ALL, flags);
	}
#endif
}


void Renderer::CreateDebugLayer()
{
#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
	HMODULE module = ::LoadLibraryA("Dxgidebug.dll");
	if (module == nullptr)
	{
		ERROR_RECOVERABLE("Failed to create debug layer");
		return;
	}

	typedef HRESULT(WINAPI* GetDebugModuleFunc)(REFIID, void**); //CB = callback
	GetDebugModuleFunc getModuleFunc = (GetDebugModuleFunc) ::GetProcAddress(module, "DXGIGetDebugInterface");
	ASSERT_OR_DIE(getModuleFunc != nullptr, "Failed to load get debug interface");

	HRESULT result = getModuleFunc(__uuidof(IDXGIDebug), (void**) &m_debug);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to get debug interface");
	m_debugModule = module;
#endif
}


void Renderer::DestroyDebugLayer()
{
#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
	if (m_debugModule)
	{
		D3X_SAFE_RELEASE(m_debug);
		::FreeLibrary((HMODULE) m_debugModule);
		m_debugModule = nullptr;
	}
#endif
}


void Renderer::InitialiseBlendState()
{
	m_blendStatesByMode[(int) BlendMode::OPAQUE] = CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
	m_blendStatesByMode[(int) BlendMode::ALPHA] = CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
	m_blendStatesByMode[(int) BlendMode::ADDITIVE] = CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);
}


void Renderer::InitialiseSamplerStates()
{
	m_samplerStatesByMode[(int) SamplingMode::POINT_CLAMP]	= CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);
	m_samplerStatesByMode[(int) SamplingMode::POINT_WRAP]	= CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
	m_samplerStatesByMode[(int) SamplingMode::LINEAR_CLAMP] = CreateSamplerState(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);
	m_samplerStatesByMode[(int) SamplingMode::LINEAR_WRAP]	= CreateSamplerState(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
}


void Renderer::CreateDefaultDiffuseTexture()
{
	m_defaultDiffuseTexture = RegisterColorTexture("Default", Rgba8::WHITE);
}


void Renderer::SetDefaultRendererState()
{
	BindShader(nullptr);
	SetDiffuseTexture(m_defaultDiffuseTexture);
	SetSamplingMode(SamplingMode::POINT_CLAMP);
	SetBlendMode(BlendMode::ALPHA);
	SetDepthOptions(DepthTest::ALWAYS, false);
	SetCullMode(CullMode::BACK);
	SetWindingOrder(WindingOrder::COUNTER_CLOCKWISE);
	SetFillMode(FillMode::SOLID);

	ModelConstants modelConstants = {};
	modelConstants.m_modelMat = Mat44();
	Rgba8 tint = Rgba8::WHITE;
	tint.GetColorAsFloats(modelConstants.m_tint);
	m_modelCBO->SetData(modelConstants);
	BindConstantBuffer(3, m_modelCBO);
}


void Renderer::ReleaseBlendStates()
{
	D3X_SAFE_RELEASE(m_blendStatesByMode[(int) BlendMode::ADDITIVE]);
	D3X_SAFE_RELEASE(m_blendStatesByMode[(int) BlendMode::ALPHA]);
	D3X_SAFE_RELEASE(m_blendStatesByMode[(int) BlendMode::OPAQUE]);
}


void Renderer::ReleaseSamplerStates()
{
	D3X_SAFE_RELEASE(m_samplerStatesByMode[(int) SamplingMode::LINEAR_WRAP]);
	D3X_SAFE_RELEASE(m_samplerStatesByMode[(int) SamplingMode::LINEAR_CLAMP]);
	D3X_SAFE_RELEASE(m_samplerStatesByMode[(int) SamplingMode::POINT_WRAP]);
	D3X_SAFE_RELEASE(m_samplerStatesByMode[(int) SamplingMode::POINT_CLAMP]);
}


void Renderer::CreateDefaultShader()
{
	ShaderConfig defaultShaderConfig = {};
	defaultShaderConfig.m_name = "Default";
	defaultShaderConfig.m_sourceCode = g_defaultShaderSource;
	m_defaultShader = new Shader();
	m_defaultShader->Create(this, defaultShaderConfig);
	m_loadedShaders.push_back(m_defaultShader);
}


void Renderer::CreateDefaultDepthStencil()
{
	Image image;
	Vec2 dims = m_renderConfig.m_window->GetWindowDims();
	image.CreateSolidColor((int) dims.x, (int) dims.y, Rgba8::BLACK_TRANSPARENT);
	m_defaultDepthStencil = new Texture();
	m_defaultDepthStencil->CreateAsDepthStencilTarget(this, "Default", image);
	m_loadedDepthStencils.push_back(m_defaultDepthStencil);
}


Texture* Renderer::CreateDepthStencil(const char* depthStencilName)
{
	Image image;
	Vec2 dims = m_renderConfig.m_window->GetWindowDims();
	image.CreateSolidColor((int) dims.x, (int) dims.y, Rgba8::BLACK_TRANSPARENT);
	Texture* depthStencil = new Texture();
	depthStencil->CreateAsDepthStencilTarget(this, depthStencilName, image);
	return depthStencil;
}


void Renderer::CreateBackbufferRenderTargetView()
{
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT hr = m_swapchain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**) &backBuffer
	);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to get the back buffer");

	hr = m_device->CreateRenderTargetView(
		backBuffer,
		nullptr,
		&m_backBufferRTV
	);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create the back buffer render target view");

	D3X_SAFE_RELEASE(backBuffer);
}


void Renderer::BeginFrame()
{
	DebugRenderBeginFrame();
}


void Renderer::EndFrame()
{
	DebugRenderEndFrame();

	m_swapchain->Present(
		0, //sync interval 
		0 //specialized flags //not needed for now
	);
}


void Renderer::ShutDown()
{
	DeInitialiseDebugRenderSystem();

	for (int textureIndex = 0; textureIndex < m_loadedTextures.size(); textureIndex++)
	{
		m_loadedTextures[textureIndex]->ReleaseResources();
		delete m_loadedTextures[textureIndex];
		m_loadedTextures[textureIndex] = nullptr;
	}
	m_loadedTextures.clear();
	m_boundTexturesBySlots.clear();

	m_defaultDepthStencil->ReleaseResources();
	delete m_defaultDepthStencil;
	m_defaultDepthStencil = nullptr;

	for (int bitmapFontIndex = 0; bitmapFontIndex < m_loadedBitmapFonts.size(); bitmapFontIndex++)
	{
		delete m_loadedBitmapFonts[bitmapFontIndex];
		m_loadedBitmapFonts[bitmapFontIndex] = nullptr;
	}
	m_loadedBitmapFonts.clear();

	for (int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); shaderIndex++)
	{
		m_loadedShaders[shaderIndex]->Destroy();
		delete m_loadedShaders[shaderIndex];
		m_loadedShaders[shaderIndex] = nullptr;
	}
	m_loadedShaders.clear();
	m_defaultShader = nullptr;
	m_boundShader = nullptr;

	for (int rtvIndex = 0; rtvIndex < m_loadedRtvs.size(); rtvIndex++)
	{
		m_loadedRtvs[rtvIndex]->ReleaseResources();
		delete m_loadedRtvs[rtvIndex];
		m_loadedRtvs[rtvIndex] = nullptr;
	}
	m_loadedRtvs.clear();

	DestroyConstantBuffer(m_cameraCBO);
	DestroyConstantBuffer(m_modelCBO);
	DestroyVertexBuffer(m_immediateVBO);
	ReleaseBlendStates();
	ReleaseSamplerStates();

	D3X_SAFE_RELEASE(m_depthStencilState);
	D3X_SAFE_RELEASE(m_currentRasterState);
	D3X_SAFE_RELEASE(m_backBufferRTV);
	D3X_SAFE_RELEASE(m_swapchain);
	D3X_SAFE_RELEASE(m_context);
	D3X_SAFE_RELEASE(m_device);

	ReportLiveObjects();
	DestroyDebugLayer();
}


void Renderer::DrawAABB2D(AABB2 const& quad, RenderMaterial material /*= {}*/)
{
	BindShader(material.m_shader);
	BindTexture(0, material.m_texture);

	std::vector<Vertex_PCU> quadVerts;
	AddVertsForAABB2ToVector(quadVerts, quad, material.m_color, material.m_textureUVs.m_mins, material.m_textureUVs.m_maxs);
	DrawVertexArray((int) quadVerts.size(), quadVerts.data());
}


void Renderer::DrawDisk2D(Vec2 const& center, float radius, RenderMaterial material/*= {}*/)
{
	BindShader(material.m_shader);
	BindTexture(0, material.m_texture);

	std::vector<Vertex_PCU> quadVerts;
	AddVertsForDisk2DToVector(quadVerts, center, radius, material.m_color);
	DrawVertexArray((int) quadVerts.size(), quadVerts.data());
}


void Renderer::DrawAABB3(AABB3 const& aabb3, RenderMaterial topMaterial, RenderMaterial sideMaterial, RenderMaterial bottomMaterial)
{
	BindShader(topMaterial.m_shader);
	BindTexture(0, topMaterial.m_texture);

	std::vector<Vertex_PCU> aabb3Verts;
	AddVertsForAABBZ3DToVector(aabb3Verts, aabb3, topMaterial.m_color, sideMaterial.m_color, bottomMaterial.m_color, topMaterial.m_textureUVs, sideMaterial.m_textureUVs, bottomMaterial.m_textureUVs);
	DrawVertexArray((int) aabb3Verts.size(), aabb3Verts.data());
}


void Renderer::DrawAABB3(AABB3 const& aabb3, RenderMaterial cubeMaterial)
{
	BindShader(cubeMaterial.m_shader);
	BindTexture(0, cubeMaterial.m_texture);

	std::vector<Vertex_PCU> aabb3Verts;
	AddVertsForAABBZ3DToVector(aabb3Verts, aabb3, cubeMaterial.m_color, cubeMaterial.m_textureUVs);
	DrawVertexArray((int) aabb3Verts.size(), aabb3Verts.data());
}


void Renderer::DrawQuad(Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, RenderMaterial renderMaterial)
{
	BindShader(renderMaterial.m_shader);
	BindTexture(0, renderMaterial.m_texture);

	std::vector<Vertex_PCU> quadVerts;
	AddVertsForQuad3DToVector(quadVerts, bottomLeft, bottomRight, topLeft, topRight, renderMaterial.m_color);
	DrawVertexArray((int) quadVerts.size(), quadVerts.data());
}


Texture* Renderer::CreateOrGetTexture(char const* filePath)
{
	for (int textureIndex = 0; textureIndex < m_loadedTextures.size(); textureIndex++)
	{
		std::string textureName = m_loadedTextures[textureIndex]->m_name;
		if (_strcmpi(filePath, textureName.c_str()) == 0)
		{
			return m_loadedTextures[textureIndex];
		}
	}

	Texture* newlyCreatedTexture = CreateTextureFromPath(filePath);
	RegisterTexture(newlyCreatedTexture);
	return  newlyCreatedTexture;
}


Texture* Renderer::RegisterTexture(Texture* textureToRegister)
{
	m_loadedTextures.push_back(textureToRegister);
	return m_loadedTextures.back();
}


Texture* Renderer::RegisterColorTexture(const char* name, Rgba8 const& textureColor)
{
	Image image;
	image.CreateSolidColor(1, 1, textureColor);
	image.SetName(name);

	Texture* texture = new Texture();
	texture->CreateFromImage(this, image);
	RegisterTexture(texture);
	return texture;
}


BitmapFont* Renderer::CreateOrGetBitmapFont(char const* filePathWithoutExtension)
{
	for (int bitmapFontIndex = 0; bitmapFontIndex < m_loadedBitmapFonts.size(); bitmapFontIndex++)
	{
		std::string bitmapFontPathWithoutExtension	= m_loadedBitmapFonts[bitmapFontIndex]->m_fontFilePathNameWithNoExtension;
		if (_strcmpi(filePathWithoutExtension, bitmapFontPathWithoutExtension.c_str()) == 0)
		{
			return m_loadedBitmapFonts[bitmapFontIndex];
		}
	}

	BitmapFont* newlyCreatedBitmapFont = CreateBitmapFontFromPath(filePathWithoutExtension);
	m_loadedBitmapFonts.push_back(newlyCreatedBitmapFont);
	return newlyCreatedBitmapFont;
}


Shader* Renderer::CreateOrGetShader(char const* shaderPathWithoutExtension)
{
	for (int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); shaderIndex++)
	{
		std::string shaderName = m_loadedShaders[shaderIndex]->m_config.m_name;
		if (_strcmpi(shaderPathWithoutExtension, shaderName.c_str()) == 0)
		{
			return m_loadedShaders[shaderIndex];
		}
	}

	Shader* newlyCreatedShader = CreateShaderFromPath(shaderPathWithoutExtension);
	m_loadedShaders.push_back(newlyCreatedShader);
	return newlyCreatedShader;
}


Texture* Renderer::CreateOrGetDepthStencil(char const* depthStencilName)
{
	for (int textureIndex = 0; textureIndex < m_loadedDepthStencils.size(); textureIndex++)
	{
		std::string textureName = m_loadedDepthStencils[textureIndex]->m_name;
		if (_strcmpi(depthStencilName, textureName.c_str()) == 0)
		{
			return m_loadedDepthStencils[textureIndex];
		}
	}

	Texture* newDepthStencil = CreateDepthStencil(depthStencilName);
	m_loadedDepthStencils.push_back(newDepthStencil);
	return  newDepthStencil;
}


Texture* Renderer::CreateOrGetRenderTargetView(char const* rtvName)
{
	for (int textureIndex = 0; textureIndex < m_loadedRtvs.size(); textureIndex++)
	{
		std::string textureName = m_loadedRtvs[textureIndex]->m_name;
		if (_strcmpi(rtvName, textureName.c_str()) == 0)
		{
			return m_loadedRtvs[textureIndex];
		}
	}

	Texture* newRtv = CreateRTVTexture(rtvName);
	m_loadedRtvs.push_back(newRtv);
	return  newRtv;
}


void Renderer::ClearDepth(Texture* depthStencil, float depthValue, unsigned char stencil)
{
	if (depthStencil)
	{
		ID3D11DepthStencilView* dsv = depthStencil->CreateOrGetDepthStencilView(this);
		if (dsv)
		{
			UINT flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL;
			m_context->ClearDepthStencilView(dsv, flags, depthValue, stencil);
		}

		return;
	}

	ERROR_RECOVERABLE("Depth Stencil to clear was null: ");
}


void Renderer::SetDepthOptions(DepthTest test, bool writeDepth)
{
	m_desiredWriteDepth = writeDepth;
	m_desiredDepthTest = test;
}


Texture* Renderer::GetDefaultDepthStencil() const
{
	return m_defaultDepthStencil;
}


void Renderer::SetModelMatrix(Mat44 const& modelMatrix, Rgba8 const& tint /*= RGBA8::WHITE*/)
{
	ModelConstants modelConstants = {};
	modelConstants.m_modelMat = modelMatrix;
	tint.GetColorAsFloats(modelConstants.m_tint);
	m_modelCBO->SetData(modelConstants);
	BindConstantBuffer(3, m_modelCBO);
}


void Renderer::BindShader(Shader const* shader)
{
	if (shader == nullptr)
		shader = m_defaultShader;
	
	if (m_boundShader && strcmp(shader->m_config.m_name.c_str(), m_boundShader->m_config.m_name.c_str()) == 0)
		return;

	m_context->VSSetShader(shader->m_vertexStage, nullptr, 0);
	m_context->PSSetShader(shader->m_pixelStage, nullptr, 0);
	m_boundShader = (Shader*) shader;
}


void Renderer::BindShaderByName(char const* shaderName)
{
	Shader const* shader = CreateOrGetShader(shaderName);
	BindShader(shader);
}


ConstantBuffer* Renderer::CreateConstantBuffer(size_t const size)
{
	ConstantBuffer* cbo = new ConstantBuffer(this, size);
	return cbo;
}


void Renderer::DestroyConstantBuffer(ConstantBuffer* cbo)
{
	D3X_SAFE_RELEASE(cbo->m_gpuBufferHandle);
	cbo->m_sourceRenderer = nullptr;
	delete cbo;
	cbo = nullptr;
}


void Renderer::BindConstantBuffer(int slot, ConstantBuffer const*cbo)
{
	m_context->VSSetConstantBuffers(slot, 1, &(cbo->m_gpuBufferHandle));
	m_context->PSSetConstantBuffers(slot, 1, &(cbo->m_gpuBufferHandle));
}


void Renderer::SetResourceDebugName(ID3D11DeviceChild* resource, char const* name)
{
#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
	if (resource)
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT) strlen(name) + 1, name);
	}
#else
	UNUSED(resource);
	UNUSED(name);
#endif
}


VertexBuffer* Renderer::CreateDynamicVertexBuffer()
{
	VertexBuffer* vbo = new VertexBuffer(this);
	return vbo;
}


VertexBuffer* Renderer::CreateDynamicVertexBuffer(std::string bufferDebugName)
{
	VertexBuffer* vbo = CreateDynamicVertexBuffer();
	SetResourceDebugName(vbo->m_gpuBufferHandle, bufferDebugName.c_str());
	return vbo;
}


void Renderer::DestroyVertexBuffer(VertexBuffer* vbo)
{
	D3X_SAFE_RELEASE(vbo->m_gpuBufferHandle);
	vbo->m_sourceRenderer = nullptr;
	delete vbo;
	vbo = nullptr;
}




void Renderer::SetFillMode(FillMode mode)
{
	m_desiredFillMode = mode;
}


void Renderer::BindTexture(int slot, Texture const* texture)
{
	texture = texture == nullptr ? m_defaultDiffuseTexture : texture;
	Texture const* boundTextureAtSlot = m_boundTexturesBySlots[slot];
	if (boundTextureAtSlot)
	{
		std::string boundTextureName = boundTextureAtSlot->GetName();
		std::string textureToBindName = texture->GetName();
		if (_strcmpi(boundTextureName.c_str(), textureToBindName.c_str()) == 0)
			return;
	}

	ID3D11ShaderResourceView* srv = texture->CreateOrGetShaderResourceView(this);
	if (srv)
	{
		m_context->PSSetShaderResources(slot, 1, &srv); //doesn't need to be bound only to pixel shader but for our purposes this is fine
		m_boundTexturesBySlots[slot] = texture;
	}
	else
		ERROR_RECOVERABLE("Texture shader resource view was null: " + texture->GetName());
}


void Renderer::BindTextureByPath(int slot, char const* textureImagePath)
{
	Texture* texture = CreateOrGetTexture(textureImagePath);
	BindTexture(slot, texture);
}


void Renderer::ClearScreen(Rgba8 const& clearColor)
{
	float colorsAsFloats[4];
	clearColor.GetColorAsFloats(colorsAsFloats);

	m_context->ClearRenderTargetView(
		m_backBufferRTV,
		colorsAsFloats
	);
}


void Renderer::BeginCamera(Camera const& camera, Texture* rtvTexture /*= nullptr*/)
{
	m_currentCamera = &camera;

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Vec2 windowDims = m_renderConfig.m_window->GetWindowDims();
	AABB2 camViewport = camera.GetScreenViewport();
	bool useDefaultViewport = false;
	if (camViewport.m_mins == camViewport.m_maxs)
		useDefaultViewport = true;

	Vec2 viewportDims;
	if (!useDefaultViewport)
	{
		viewportDims.x = camViewport.m_maxs.x - camViewport.m_mins.x;
		viewportDims.y = camViewport.m_maxs.y - camViewport.m_mins.y;
	}

	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = useDefaultViewport ? 0.0f : camViewport.m_mins.x;
	viewPort.TopLeftY = useDefaultViewport ? 0.0f : camViewport.m_mins.y;
	viewPort.Width = useDefaultViewport ? windowDims.x : viewportDims.x;
	viewPort.Height = useDefaultViewport ? windowDims.y : viewportDims.y;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	m_context->RSSetViewports(1, &viewPort);

	CameraConstants cameraConstants = {};
	cameraConstants.m_projectionMat = camera.GetProjectionMatrix();
	cameraConstants.m_viewMat = camera.GetCameraViewMatrix();
	m_cameraCBO->SetData(cameraConstants);
	BindConstantBuffer(2, m_cameraCBO);

	SetDefaultRendererState();
	SetRenderTargetView(rtvTexture);
}


void Renderer::EndCamera(Camera const& camera)
{
	UNUSED(camera);
	m_currentCamera = nullptr;
}


Camera const& Renderer::GetCurrentCamera() const
{
	return *m_currentCamera;
}


void Renderer::DrawVertexArray(int numVertexes, const Vertex_PCU* vertexes)
{
	m_immediateVBO->CopyVertexArray(vertexes, numVertexes);
	DrawVertexBuffer(m_immediateVBO, numVertexes);
}


void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int numVertexes, int offset)
{
	ID3D11Buffer* handle = vbo->GetHandle();
	UINT stride = (UINT) vbo->GetStride();
	UINT offsetValue = (UINT) offset;

	m_context->IASetVertexBuffers(0, 1, &handle, &stride,  &offsetValue);

	ID3D11InputLayout* inputLayout = m_boundShader->CreateOrGetInputLayout_VertexPCU(this);
	m_context->IASetInputLayout(inputLayout);

	Draw(numVertexes);
}


void Renderer::Draw(int vertexCount, int vertexOffset /*= 0*/)
{
	UpdateRasterStateIfDirty();
	UpdateAndBindDepthStencilStateIfDirty();
	m_context->Draw(vertexCount, vertexOffset);
}


void Renderer::SetBlendMode(BlendMode mode)
{
	ID3D11BlendState* blendState = m_blendStatesByMode[(int) mode];
	float const blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_context->OMSetBlendState(blendState, blendConstants, 0xffffffff);
}


void Renderer::SetCullMode(CullMode mode)
{
	m_desiredCullMode = mode;
}


void Renderer::SetWindingOrder(WindingOrder frontFacingWindingOrder)
{
	m_desiredWindingOrder = frontFacingWindingOrder;
}


void Renderer::SetSamplingMode(SamplingMode mode, int bindPoint /* = 0 */)
{
	ID3D11SamplerState* sampleState = m_samplerStatesByMode[(int) mode];
	m_context->PSSetSamplers(bindPoint, 1, &sampleState);
}


void Renderer::ClearRenderTargetView(Texture* rtvTexture, Rgba8 const& clearColor)
{
	float colorsAsFloats[4];
	clearColor.GetColorAsFloats(colorsAsFloats);

	m_context->ClearRenderTargetView(
		rtvTexture->CreateOrGetRenderTargetView(this),
		colorsAsFloats
	);
}


void Renderer::SetRenderTargetView(Texture* rtvTexture)
{
	ID3D11DepthStencilView* dsv = m_defaultDepthStencil->CreateOrGetDepthStencilView(this);
	if (rtvTexture == nullptr)
	{
		m_context->OMSetRenderTargets(1, &m_backBufferRTV, dsv);
	}
	else
	{
		ID3D11RenderTargetView* rtv = rtvTexture->CreateOrGetRenderTargetView(this);
		m_context->OMSetRenderTargets(1, &rtv, dsv);
	}
}

//binds to slot 0
void Renderer::SetDiffuseTexture(Texture const* texture)
{
	texture = texture == nullptr ? m_defaultDiffuseTexture : texture;

	if (m_boundTexturesBySlots[0])
	{
		std::string boundTextureName = m_boundTexturesBySlots[0]->GetName();
		std::string textureToBindName = texture->GetName();
		if (_strcmpi(boundTextureName.c_str(), textureToBindName.c_str()) == 0)
			return;
	}

	ID3D11ShaderResourceView* srv = ((Texture*) texture)->CreateOrGetShaderResourceView(this);
	if (srv)
	{
		m_context->PSSetShaderResources(0, 1, &srv); //doesn't need to be bound only to pixel shader but for our purposes this is fine
		m_boundTexturesBySlots[0] = texture;
	}
	else
		ERROR_RECOVERABLE("Texture shader resource view was null: " + texture->GetName());
}


ID3D11Device* Renderer::GetDevice() const
{
	return m_device;
}


ID3D11DeviceContext* Renderer::GetDeviceContext() const
{
	return m_context;
}


void Renderer::CreateRenderContext()
{
	DWORD flags = 0;
	flags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_SINGLETHREADED;
#if _DEBUG
	flags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

	Vec2 windowDims = m_renderConfig.m_window->GetWindowDims();

	DXGI_SWAP_CHAIN_DESC scDesc = {0};
	scDesc.BufferDesc.Width = (int) windowDims.x; //0 defaults to the client window width
	scDesc.BufferDesc.Height = (int) windowDims.y; //0 defaults to the client window height
	scDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2; //front and back buffer
	scDesc.OutputWindow = (HWND) m_renderConfig.m_window->GetWindowHandle();
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&scDesc,
		&m_swapchain,
		&m_device,
		nullptr,
		&m_context
	);

	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create device context for renderer");
}


Texture* Renderer::CreateTextureFromPath(char const* filePath)
{
	Texture* texture = new Texture();
	texture->LoadFromPath(this, filePath);
	return texture;
}


BitmapFont* Renderer::CreateBitmapFontFromPath(char const* filePathWithoutExtension)
{
	std::string filePath = std::string(filePathWithoutExtension) + ".png";
	Texture* bitmapFontTex = CreateOrGetTexture(filePath.c_str());
	BitmapFont* newBitmapFont = new BitmapFont(filePathWithoutExtension, *bitmapFontTex);
	return newBitmapFont;
}


Shader* Renderer::CreateShaderFromPath(char const* shaderPathWithoutExtensions)
{
	ShaderConfig config = {};
	config.m_name = shaderPathWithoutExtensions;

	Shader* shader = new Shader();
	shader->Create(this, config);
	return shader;
}


Texture* Renderer::CreateRTVTexture(char const* rtvName)
{
	Vec2 dims = m_renderConfig.m_window->GetWindowDims();
	Texture* depthStencil = new Texture();
	depthStencil->CreateAsRenderTargetView(this, rtvName, dims.GetIntVec2());
	return depthStencil;
}


void Renderer::UpdateRasterStateIfDirty()
{
	if (IsRasterStateDirty())
	{
		D3X_SAFE_RELEASE(m_currentRasterState);

		D3D11_RASTERIZER_DESC rasterDesc = {};

		switch (m_desiredCullMode)
		{
			case CullMode::NONE:
				rasterDesc.CullMode = D3D11_CULL_NONE;
				break;
			case CullMode::FRONT:
				rasterDesc.CullMode = D3D11_CULL_FRONT;
				break;
			case CullMode::BACK:
				rasterDesc.CullMode = D3D11_CULL_BACK;
				break;
			default:
				break;
		}

		rasterDesc.FrontCounterClockwise = m_desiredWindingOrder == WindingOrder::COUNTER_CLOCKWISE;

		switch (m_desiredFillMode)
		{
			case  FillMode::SOLID:
				rasterDesc.FillMode = D3D11_FILL_SOLID;
				break;
			case  FillMode::WIREFRAME:
				rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
				break;
			default:
				break;
		}

		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		rasterDesc.DepthClipEnable = true;
		rasterDesc.ScissorEnable = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.AntialiasedLineEnable = true;

		HRESULT hr = m_device->CreateRasterizerState(&rasterDesc, &m_currentRasterState);
		
		if (SUCCEEDED(hr))
		{
			m_context->RSSetState(m_currentRasterState);

			m_currentCullMode = m_desiredCullMode;
			m_currentWindingOrder = m_desiredWindingOrder;
			m_currentFillMode = m_desiredFillMode;
		}
		else
		{
			ERROR_AND_DIE("Unable to create a valid raster state");
		}
	}
}


void Renderer::UpdateAndBindDepthStencilStateIfDirty()
{
	if (IsDepthStencilStateDirty())
	{
		D3X_SAFE_RELEASE(m_depthStencilState);

		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = true;
		if (m_depthTest == DepthTest::ALWAYS && !m_desiredWriteDepth)
		{
			desc.DepthEnable = false;
		}
		desc.DepthWriteMask = m_desiredWriteDepth ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

		switch (m_desiredDepthTest)
		{
			case DepthTest::ALWAYS:
				desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
				break;
			case DepthTest::NEVER:
				desc.DepthFunc = D3D11_COMPARISON_NEVER;
				break;
			case DepthTest::EQUAL:
				desc.DepthFunc = D3D11_COMPARISON_EQUAL;
				break;
			case DepthTest::NOT_EQUAL:
				desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;
				break;
			case DepthTest::LESS:
				desc.DepthFunc = D3D11_COMPARISON_LESS;
				break;
			case DepthTest::LESS_EQUAL:
				desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
				break;
			case DepthTest::GREATER:
				desc.DepthFunc = D3D11_COMPARISON_GREATER;
				break;
			case DepthTest::GREATER_EQUAL:
				desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
				break;
			default:
				break;
		}

		desc.StencilEnable = false;
		HRESULT hr = m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
		if (SUCCEEDED(hr))
		{
			m_context->OMSetDepthStencilState(m_depthStencilState, 0);

			m_depthTest = m_desiredDepthTest;
			m_writeDepth = m_desiredWriteDepth;
		}
		else
		{
			ERROR_AND_DIE("Unable to create a valid depth stencil state");
		}
	}
}


bool Renderer::IsRasterStateDirty()
{
	return m_currentRasterState == nullptr || m_currentCullMode != m_desiredCullMode || m_currentWindingOrder != m_desiredWindingOrder
		|| m_currentFillMode != m_desiredFillMode;
}


bool Renderer::IsDepthStencilStateDirty()
{
	return m_depthStencilState == nullptr || m_desiredDepthTest != m_depthTest || m_desiredWriteDepth != m_writeDepth;
}


ID3D11BlendState* Renderer::CreateBlendState(D3D11_BLEND srcFactor, D3D11_BLEND destFactor, D3D11_BLEND_OP blendOeration)
{
	ID3D11BlendState* blendSate = nullptr;
	D3D11_BLEND_DESC blendStateDesc = {};
	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.IndependentBlendEnable = false;

	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	
	blendStateDesc.RenderTarget[0].SrcBlend		= srcFactor;
	blendStateDesc.RenderTarget[0].DestBlend	= destFactor;
	blendStateDesc.RenderTarget[0].BlendOp		= blendOeration;

	//todo use correct equations
	//this is wrong - but not important for our purposes
	blendStateDesc.RenderTarget[0].SrcBlendAlpha	= srcFactor;
	blendStateDesc.RenderTarget[0].DestBlendAlpha	= destFactor;
	blendStateDesc.RenderTarget[0].BlendOpAlpha		= blendOeration;
	
	HRESULT hr = m_device->CreateBlendState(&blendStateDesc, &blendSate);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create blend state")
	return blendSate;
}


ID3D11SamplerState* Renderer::CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode)
{
	ID3D11SamplerState* samplerState = nullptr;

	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = filter;
	desc.AddressU = addressMode;
	desc.AddressV = addressMode;
	desc.AddressW = addressMode;

	desc.MipLODBias = 0.0f;
	desc.MinLOD = 0;
	desc.MaxLOD = 0;
	desc.MaxAnisotropy = 0;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

	HRESULT hr = m_device->CreateSamplerState(&desc, &samplerState);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to make sampler state ");
	return samplerState;
}


void Renderer::InitialiseDebugRenderSystem()
{
	DebugRenderConfig config = {};
	config.m_renderer = this;
	config.m_startHidden = false;
	DebugRenderSystemStartup(config);

#if defined(ENABLE_ON_SCREEN_DEBUGGING)
	if (g_eventSystem)
	{
		g_eventSystem->SubscribeEventCallbackFunction("DebugRenderClear", OnClearDebugRender);
		g_eventSystem->SubscribeEventCallbackFunction("DebugRenderToggleVisibility", OnToggleVisibilityDebugRender);
		g_eventSystem->SubscribeEventCallbackFunction("DebugRenderSetTimeScale", OnSetDebugRenderTimeScale);
	}
#endif
}


void Renderer::DeInitialiseDebugRenderSystem()
{
	DebugRenderSystemShutdown();

#if defined(ENABLE_ON_SCREEN_DEBUGGING)
	if (g_eventSystem)
	{
		g_eventSystem->UnsubscribeEventCallbackFunction("DebugRenderClear", OnClearDebugRender);
		g_eventSystem->UnsubscribeEventCallbackFunction("DebugRenderToggleVisibility", OnToggleVisibilityDebugRender);
		g_eventSystem->UnsubscribeEventCallbackFunction("DebugRenderSetTimeScale", OnSetDebugRenderTimeScale);
	}
#endif
}
