#pragma once
#include <string>
#include "Engine/Math/IntVec2.hpp"

class Renderer;
class SpriteSheet;
class Image;

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

class Texture
{
	friend class Renderer;
	friend class SpriteSheet;

	Texture() = default;
	~Texture();
	Texture(Texture const& copy) = delete;

	bool LoadFromPath(Renderer* sourceRenderer, char const* path);
	bool CreateFromImage(Renderer* sourceRenderer, Image const& sourceImage);
	bool CreateAsDepthStencilTarget(Renderer* sourceRenderer, char const* textureName, Image const& sourceImage);
	bool CreateAsRenderTargetView(Renderer* sourceRenderer, char const* rtvName, IntVec2 const& dimensions);
	void ReleaseResources();

	ID3D11ShaderResourceView* CreateOrGetShaderResourceView(Renderer* sourceRenderer) const;
	ID3D11DepthStencilView* CreateOrGetDepthStencilView(Renderer* sourceRenderer);
	ID3D11RenderTargetView* CreateOrGetRenderTargetView(Renderer* sourceRenderer);
	IntVec2 GetDimensions() const;
	std::string GetName() const;
	ID3D11Texture2D* GetHandle() const;

private:
	IntVec2			m_dimensions;
	std::string		m_name;
	ID3D11Texture2D* m_handle = nullptr;

	ID3D11RenderTargetView* m_renderTargetView = nullptr; // A01- specifying that the texture wants to be rendered to in a specific format
	mutable ID3D11ShaderResourceView* m_shaderResourceView = nullptr; // A04 - this texture can be read from in a specific format
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
};
