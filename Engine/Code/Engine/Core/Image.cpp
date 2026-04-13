#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

Image::Image()
{

}


Image::Image(const char* imageFilePath)
{
	m_name = std::string(imageFilePath);
	int numComponents = 0; 
	int numComponentsRequested = 0;

	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* imageData = stbi_load(imageFilePath, &m_dimensions.x, &m_dimensions.y, &m_bytesPerPixel, numComponentsRequested);

	GUARANTEE_OR_DIE(imageData, Stringf("Failed to load image \"%s\"", imageFilePath));
	GUARANTEE_OR_DIE(m_bytesPerPixel >= 3 && m_bytesPerPixel <= 4 && m_dimensions.x > 0 && m_dimensions.y > 0, Stringf("ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, m_dimensions.x, m_dimensions.y));
	
	int numTexels = m_dimensions.x * m_dimensions.y;
	m_texelRgba8Data.resize((size_t) (numTexels));

	unsigned char* dataTemp = imageData;
	int texelIndex = 0;
	while (dataTemp && texelIndex < m_texelRgba8Data.size())
	{
		if (m_bytesPerPixel == 4)
		{
			Rgba8 texelRgba8;
			texelRgba8.r = *dataTemp;
			texelRgba8.g = *(dataTemp + 1);
			texelRgba8.b = *(dataTemp + 2);
			texelRgba8.a = *(dataTemp + 3);
			m_texelRgba8Data[texelIndex] = texelRgba8;

			dataTemp = dataTemp + 4;
			texelIndex++;
		}

		if (m_bytesPerPixel == 3)
		{
			Rgba8 texelRgba8;
			texelRgba8.r = *dataTemp;
			texelRgba8.g = *(dataTemp + 1);
			texelRgba8.b = *(dataTemp + 2);
			m_texelRgba8Data[texelIndex] = texelRgba8;

			dataTemp = dataTemp + 3;
			texelIndex++;
		}
	}

	stbi_image_free(imageData);
}


void Image::CreateSolidColor(int width, int height, Rgba8 const& color)
{
	m_name = "SolidColor";
	m_dimensions = IntVec2(width, height);
	int numPixels = width * height;
	m_texelRgba8Data.resize(numPixels, color);
	m_bytesPerPixel = 4;
}


void Image::SetTexelRgba8(int texelX, int texelY, Rgba8 const& newRgba8)
{
	int texelIndex = texelX + texelY * m_dimensions.x;
	GUARANTEE_OR_DIE(texelIndex >= 0 && texelIndex < m_texelRgba8Data.size(), Stringf("Coordinates (%i, %i) out of bounds from the image", texelX, texelY));
	m_texelRgba8Data[texelIndex] = newRgba8;
}


void Image::SetTexelRgba8(IntVec2 const& texelCoords, Rgba8 const& newRgba8)
{
	int texelIndex = texelCoords.x + texelCoords.y * m_dimensions.x;
	GUARANTEE_OR_DIE(texelIndex >= 0 && texelIndex < m_texelRgba8Data.size(), Stringf("Coordinates (%i, %i) out of bounds from the image", texelCoords.x, texelCoords.y));
	m_texelRgba8Data[texelIndex] = newRgba8;
}


std::string Image::GetName() const
{
	return m_name;
}


IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}


Rgba8 Image::GetTexelRgba8(int texelX, int texelY) const
{
	int texelIndex = texelX + texelY * m_dimensions.x;
	GUARANTEE_OR_DIE(texelIndex >= 0 && texelIndex < m_texelRgba8Data.size(), Stringf("Coordinates (%i, %i) out of bounds from the image", texelX, texelY));
	return m_texelRgba8Data[texelIndex];
}


Rgba8 Image::GetTexelRgba8(IntVec2 const& texelCoords) const
{
	int texelIndex = texelCoords.x + texelCoords.y * m_dimensions.x;
	GUARANTEE_OR_DIE(texelIndex >= 0 && texelIndex < m_texelRgba8Data.size(), Stringf("Coordinates (%i, %i) out of bounds from the image", texelCoords.x, texelCoords.y));
	return m_texelRgba8Data[texelIndex];
}


Rgba8 const* Image::GetRawData() const
{
	return m_texelRgba8Data.data();
}


int Image::GetPitch() const
{
	return 4 * m_dimensions.x;
}


void Image::SetName(const char* name)
{
	m_name = std::string(name);
}


Image::~Image()
{

}
