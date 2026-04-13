#pragma once
#include<string>
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Rgba8.hpp"

class Image
{
public:
	Image();
	~Image();
	Image(const char* imageFilePath);

	void CreateSolidColor(int width, int height, Rgba8 const& color);
	void SetTexelRgba8(int texelX, int texelY, Rgba8 const& newRgba8);
	void SetTexelRgba8(IntVec2 const& texelCoords, Rgba8 const& newRgba8);

	std::string  GetName() const;
	IntVec2		 GetDimensions() const;
	Rgba8		 GetTexelRgba8(int texelX, int texelY) const;
	Rgba8		 GetTexelRgba8(IntVec2 const& texelCoords) const;
	Rgba8 const* GetRawData() const;
	int			 GetPitch() const;

	void SetName(const char* name);
private:
	std::string m_name;
	IntVec2 m_dimensions;
	std::vector<Rgba8> m_texelRgba8Data;
	int m_bytesPerPixel;
};