#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/D3D11Headers.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Texture::~Texture()
{

}


bool Texture::LoadFromPath(Renderer* sourceRenderer, char const* path)
{
	Image image = Image(path);
	return CreateFromImage(sourceRenderer, image);
}


bool Texture::CreateFromImage(Renderer* sourceRenderer, Image const& sourceImage)
{
	ID3D11Device* device = sourceRenderer->GetDevice();

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = sourceImage.GetDimensions().x;
	desc.Height = sourceImage.GetDimensions().y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = sourceImage.GetRawData();
	initialData.SysMemPitch = sourceImage.GetPitch();
	initialData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateTexture2D(&desc, &initialData, &m_handle);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create texture from image: " + sourceImage.GetName());

	m_name = sourceImage.GetName();
	m_dimensions = sourceImage.GetDimensions();
	std::string debugName = std::string("Texture2D: " + m_name);
	sourceRenderer->SetResourceDebugName(m_handle, debugName.c_str());
	return true;
}


bool Texture::CreateAsDepthStencilTarget(Renderer* sourceRenderer, char const* textureName, Image const& sourceImage)
{
	IntVec2 texelDims = sourceImage.GetDimensions();
	ID3D11Device* device = sourceRenderer->GetDevice();

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = texelDims.x;
	desc.Height = texelDims.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &m_handle);
	std::string name(textureName);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create depth stencil: " + name);

	m_name = name;
	m_dimensions = texelDims;
	std::string debugName = std::string("DepthStencil: " + m_name);
	sourceRenderer->SetResourceDebugName(m_handle, debugName.c_str());
	return true;
}


bool Texture::CreateAsRenderTargetView(Renderer* sourceRenderer, char const* rtvName, IntVec2 const& dimensions)
{
	IntVec2 texelDims = dimensions;
	ID3D11Device* device = sourceRenderer->GetDevice();

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = texelDims.x;
	desc.Height = texelDims.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &m_handle);
	std::string name(rtvName);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create render target view: " + name);

	m_name = name;
	m_dimensions = texelDims;
	std::string debugName = std::string("DepthStencil: " + m_name);
	sourceRenderer->SetResourceDebugName(m_handle, debugName.c_str());
	return true;
}


void Texture::ReleaseResources()
{
	D3X_SAFE_RELEASE(m_depthStencilView);
	D3X_SAFE_RELEASE(m_shaderResourceView);
	D3X_SAFE_RELEASE(m_renderTargetView);
	D3X_SAFE_RELEASE(m_handle);
}


ID3D11ShaderResourceView* Texture::CreateOrGetShaderResourceView(Renderer* sourceRenderer) const
{
	if (m_shaderResourceView != nullptr)
		return m_shaderResourceView;

	ID3D11Device* device = sourceRenderer->GetDevice();
	HRESULT hr = device->CreateShaderResourceView(m_handle, nullptr, &m_shaderResourceView);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create srv for texture: " + m_name);

	std::string debugName = std::string("Texture2D SRV: " + m_name);
	sourceRenderer->SetResourceDebugName(m_shaderResourceView, debugName.c_str());
	return m_shaderResourceView;
}


ID3D11DepthStencilView* Texture::CreateOrGetDepthStencilView(Renderer* sourceRenderer)
{
	if (m_depthStencilView != nullptr)
		return m_depthStencilView;

	ID3D11Device* device = sourceRenderer->GetDevice();
	HRESULT hr = device->CreateDepthStencilView(m_handle, nullptr, &m_depthStencilView);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create dsv for texture: " + m_name);

	std::string debugName = std::string("Texture2D DSV: " + m_name);
	sourceRenderer->SetResourceDebugName(m_depthStencilView, debugName.c_str());
	return m_depthStencilView;
}


ID3D11RenderTargetView* Texture::CreateOrGetRenderTargetView(Renderer* sourceRenderer)
{
	if (m_renderTargetView)
		return m_renderTargetView;

	ID3D11Device* device = sourceRenderer->GetDevice();
	D3D11_RENDER_TARGET_VIEW_DESC desc = {};
	HRESULT hr = device->CreateRenderTargetView(m_handle, nullptr, &m_renderTargetView);
	ASSERT_OR_DIE(SUCCEEDED(hr), "Unable to create rtv for texture: " + m_name);

	std::string debugName = std::string("Texture2D RTV: " + m_name);
	sourceRenderer->SetResourceDebugName(m_renderTargetView, debugName.c_str());
	return m_renderTargetView;
}


IntVec2 Texture::GetDimensions() const
{
	return m_dimensions;
}


std::string Texture::GetName() const
{
	return m_name;
}


ID3D11Texture2D* Texture::GetHandle() const
{
	return m_handle;
}
