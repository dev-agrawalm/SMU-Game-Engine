#pragma once
#include<vector>
#include "Engine/Math/Mat44.hpp"
#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include <map>
#undef OPAQUE

struct Vertex_PCU;
struct Rgba8;
struct AABB3;
class Camera;
class Window;
class Texture;
class BitmapFont;
class Shader;
class ConstantBuffer;
class VertexBuffer;
class Image;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct IDXGIDebug;
struct ID3D11DeviceChild;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;

enum D3D11_BLEND : int;
enum D3D11_BLEND_OP : int;
enum D3D11_FILTER : int;
enum D3D11_TEXTURE_ADDRESS_MODE : int;

enum class BlendMode
{
	OPAQUE = 0,
	ALPHA,
	ADDITIVE,
	NUM_BLEND_MODES
};


enum class CullMode
{
	NONE = 0,
	FRONT,
	BACK 
};


enum class FillMode
{
	SOLID,
	WIREFRAME
};


enum class WindingOrder
{
	COUNTER_CLOCKWISE,
	CLOCKWISE
};


enum class SamplingMode
{
	POINT_CLAMP,
	POINT_WRAP,
	LINEAR_CLAMP,
	LINEAR_WRAP,
	NUM_SAMPLING_MODES
};


enum class DepthTest
{
	ALWAYS, 
	NEVER, 
	EQUAL, 
	NOT_EQUAL, 
	LESS, 
	LESS_EQUAL, 
	GREATER, 
	GREATER_EQUAL
};


struct RenderConfig
{
	Window* m_window = nullptr;
};


struct CameraConstants //constant buffer structs should be multiple of 16 bytes // add padding if its not the case
{
	Mat44 m_projectionMat;
	Mat44 m_viewMat;
};


struct ModelConstants
{
	Mat44 m_modelMat;
	float m_tint[4];
};


struct RenderMaterial
{
	Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	Rgba8 m_color = Rgba8::WHITE;
	AABB2 m_textureUVs = AABB2::ZERO_TO_ONE;
};


class Renderer
{
public: //methods
	Renderer(RenderConfig const& config);
	~Renderer();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	//Render Utilities
	void DrawAABB2D(AABB2 const& quad, RenderMaterial material = {});
	void DrawDisk2D(Vec2 const& center, float radius, RenderMaterial material = {});
	void DrawAABB3(AABB3 const& aabb3, RenderMaterial topMaterial, RenderMaterial sideMaterial, RenderMaterial bottomMaterial);
	void DrawAABB3(AABB3 const& aabb3, RenderMaterial cubeMaterial);
	void DrawQuad(Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, RenderMaterial renderMaterial);


	Texture*	CreateOrGetTexture(char const* filePath); //file path will be used for texture name
	Texture*	CreateOrGetDepthStencil(char const* depthStencilName);
	Texture*	CreateOrGetRenderTargetView(char const* rtvName);
	BitmapFont* CreateOrGetBitmapFont(char const* filePathWithoutExtension);
	Shader*		CreateOrGetShader(char const* shaderPathWithoutExtension);

	void		ClearDepth(Texture* depthStencil, float depthValue, unsigned char stencil);
	void        SetDepthOptions(DepthTest test, bool writeDepth);
	Texture*	GetDefaultDepthStencil() const;

	void			SetModelMatrix(Mat44 const& modelMatrix, Rgba8 const& tint = Rgba8::WHITE);
	ConstantBuffer* CreateConstantBuffer(size_t const size);
	void			DestroyConstantBuffer(ConstantBuffer* cbo);
	VertexBuffer*	CreateDynamicVertexBuffer();
	VertexBuffer*	CreateDynamicVertexBuffer(std::string bufferDebugName);
	void			DestroyVertexBuffer(VertexBuffer* vbo);
	
	void		ClearScreen(Rgba8 const& clearColor);
	void		BeginCamera(Camera const& camera, Texture* rtvTexture = nullptr);
	void		EndCamera(Camera const& camera);
	Camera const& GetCurrentCamera() const;
	
	void		DrawVertexArray(int numVertexes, const Vertex_PCU* vertexes);
	void		DrawVertexBuffer(VertexBuffer* vbo, int numVertexes, int offset = 0);
	void		Draw(int vertexCount, int vertexOffset = 0);
	
	void		SetFillMode(FillMode mode);
	void		SetBlendMode(BlendMode mode);
	void		SetCullMode(CullMode mode);
	void		SetWindingOrder(WindingOrder frontFacingWindingOrder);
	void		SetSamplingMode(SamplingMode mode, int bindPoint = 0);
	
	void		ClearRenderTargetView(Texture* rtvTexture, Rgba8 const& clearColor);
	void		SetRenderTargetView(Texture* rtvTexture);
	void		SetDiffuseTexture(Texture const* texture);
	void		BindTexture(int slot, Texture const* texture);
	void		BindTextureByPath(int slot, char const* textureImagePath);
	void		BindShader(Shader const* shader);
	void		BindShaderByName(char const* shaderName);
	void		BindConstantBuffer(int slot, ConstantBuffer const*cbo);

	void				 SetResourceDebugName(ID3D11DeviceChild* resource, char const* name);
	ID3D11Device*		 GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;

protected:
	Texture*	RegisterTexture(Texture* textureToRegister);
	Texture*	RegisterColorTexture(const char* name, Rgba8 const& textureColor);
	void		ReportLiveObjects();
	void		CreateDebugLayer();
	void		DestroyDebugLayer();
	void		InitialiseBlendState();
	void		InitialiseSamplerStates();
	void		CreateDefaultDiffuseTexture();
	void		SetDefaultRendererState();
	void		ReleaseBlendStates();
	void		ReleaseSamplerStates();
	
	void		CreateDefaultShader();
	void		CreateDefaultDepthStencil();
	Texture*	CreateDepthStencil(const char* depthStencilName);
	void		CreateRenderContext();
	void		CreateBackbufferRenderTargetView();
	Texture*	CreateTextureFromPath(char const* filePath);
	BitmapFont* CreateBitmapFontFromPath(char const* filePathWithoutExtension);
	Shader*		CreateShaderFromPath(char const* shaderPathWithoutExtensions);
	Texture*	CreateRTVTexture(char const* rtvName);
	
	void		UpdateRasterStateIfDirty();
	void		UpdateAndBindDepthStencilStateIfDirty();
	bool		IsRasterStateDirty();
	bool		IsDepthStencilStateDirty();
	ID3D11BlendState* CreateBlendState(D3D11_BLEND srcFactor, D3D11_BLEND destFactor, D3D11_BLEND_OP blendOeration);
	ID3D11SamplerState* CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);
	void		InitialiseDebugRenderSystem();
	void		DeInitialiseDebugRenderSystem();

protected: //members
	RenderConfig m_renderConfig;

	DepthTest				 m_depthTest = DepthTest::ALWAYS;
	DepthTest				 m_desiredDepthTest = DepthTest::ALWAYS;
	bool					 m_writeDepth = false;
	bool					 m_desiredWriteDepth = false;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;

	//Texture*				 m_boundDepthStencil = nullptr;
	Texture*				 m_defaultDepthStencil = nullptr;
	std::map<int, Texture const*>	m_boundTexturesBySlots;
	Texture*				m_defaultDiffuseTexture = nullptr;
	Shader*					m_boundShader	= nullptr;
	Shader*					m_defaultShader = nullptr;
	
	std::vector<Texture*>	m_loadedRtvs;
	std::vector<Texture*>	m_loadedTextures;
	std::vector<BitmapFont*> m_loadedBitmapFonts;
	std::vector<Texture*>	 m_loadedDepthStencils;
	std::vector<Shader*>	m_loadedShaders;

	ID3D11Device* m_device						= nullptr;
	ID3D11DeviceContext* m_context				= nullptr;
	IDXGISwapChain* m_swapchain					= nullptr;
	ID3D11RenderTargetView* m_backBufferRTV		= nullptr; //todo shift it to texture class and store it as a texture
	ID3D11RasterizerState* m_currentRasterState = nullptr;

	ConstantBuffer* m_cameraCBO		= nullptr;
	ConstantBuffer* m_modelCBO		= nullptr;
	VertexBuffer* m_immediateVBO	= nullptr;	

	FillMode m_currentFillMode			= FillMode::SOLID;
	FillMode m_desiredFillMode			= FillMode::SOLID;
	CullMode m_currentCullMode			= CullMode::NONE;
	CullMode m_desiredCullMode			= CullMode::NONE;
	WindingOrder m_currentWindingOrder	= WindingOrder::COUNTER_CLOCKWISE;
	WindingOrder m_desiredWindingOrder	= WindingOrder::COUNTER_CLOCKWISE;
	
	ID3D11BlendState*	m_blendStatesByMode[(int) BlendMode::NUM_BLEND_MODES];
	ID3D11SamplerState* m_samplerStatesByMode[(int) SamplingMode::NUM_SAMPLING_MODES];

	Camera const* m_currentCamera = nullptr;

#if defined(ENGINE_ENABLE_DEBUG_RENDERER)
	void* m_debugModule = nullptr;
	IDXGIDebug* m_debug = nullptr;
#endif
};
