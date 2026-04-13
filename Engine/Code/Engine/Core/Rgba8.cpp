#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Core\EngineCommon.hpp"

Rgba8 const Rgba8::WHITE				= Rgba8(255, 255, 255, 255);
Rgba8 const Rgba8::WHITE_TRANSPARENT	= Rgba8(255, 255, 255, 0);
Rgba8 const Rgba8::GREY					= Rgba8(127, 127, 127, 255);
Rgba8 const Rgba8::GREY_TRANSPARENT		= Rgba8(127, 127, 127, 0);
Rgba8 const Rgba8::BLACK				= Rgba8(0, 0, 0, 255);
Rgba8 const Rgba8::BLACK_TRANSPARENT	= Rgba8(0, 0, 0, 0);
Rgba8 const Rgba8::RED					= Rgba8(255, 0, 0, 255);
Rgba8 const Rgba8::RED_TRANSPARENT		= Rgba8(255, 0, 0, 0);
Rgba8 const Rgba8::BLUE					= Rgba8(0, 0, 255, 255);
Rgba8 const Rgba8::BLUE_TRANSPARENT		= Rgba8(0, 0, 255, 0);
Rgba8 const Rgba8::GREEN				= Rgba8(0, 255, 0, 255);
Rgba8 const Rgba8::GREEN_TRANSPARENT	= Rgba8(0, 255, 0, 255);
Rgba8 const Rgba8::YELLOW				= Rgba8(255, 255, 0, 255);
Rgba8 const Rgba8::YELLOW_TRANSPARENT	= Rgba8(255, 255, 0, 0);
Rgba8 const Rgba8::CYAN					= Rgba8(0, 255, 255, 255);
Rgba8 const Rgba8::CYAN_TRANSPARENT		= Rgba8(0, 255, 255, 0);
Rgba8 const Rgba8::MAGENTA				= Rgba8(255, 0, 255, 255);
Rgba8 const Rgba8::MAGENTA_TRANSPARENT	= Rgba8(255, 0, 255, 0);


Rgba8::Rgba8()
{
	//empty
}


void Rgba8::SetFromText(std::string text)
{
	Strings colorStrings = SplitStringOnDelimiter(text, ',');
	GUARANTEE_RECOVERABLE(colorStrings.size() == 4 || colorStrings.size() == 3, "Text for Vec2 has more than 1 comma which is against the acceptable format");

// 	floatStrings[0] = RemoveWhitespacesFromString(floatStrings[0]);
// 	floatStrings[1] = RemoveWhitespacesFromString(floatStrings[1]);

	if (colorStrings.size() == 3)
	{
		r = static_cast<unsigned char>( Clamp(atoi(colorStrings[0].c_str()), 0, 255) );
		g = static_cast<unsigned char>( Clamp(atoi(colorStrings[1].c_str()), 0, 255) );
		b = static_cast<unsigned char>( Clamp(atoi(colorStrings[2].c_str()), 0, 255) );
		a = 255;																	 
	}		   																		 
	else	   																		 
	{		   																		 
		r = static_cast<unsigned char>( Clamp(atoi(colorStrings[0].c_str()), 0, 255) );
		g = static_cast<unsigned char>( Clamp(atoi(colorStrings[1].c_str()), 0, 255) );
		b = static_cast<unsigned char>( Clamp(atoi(colorStrings[2].c_str()), 0, 255) );
		a = static_cast<unsigned char>( Clamp(atoi(colorStrings[3].c_str()), 0, 255) );
	}
}


void Rgba8::ScaleRGB(float scale)
{
	float color[4];
	GetColorAsFloats(color);

	float red = color[0] * scale;
	float green = color[1] * scale;
	float blue = color[2] * scale;
	
	r = FloatToByte(red);
	g = FloatToByte(green);
	b = FloatToByte(blue);
}


void Rgba8::GetColorAsFloats(float* out_colorsAsFloats) const
{
	out_colorsAsFloats[0] = ByteToFloatNormalised(r);
	out_colorsAsFloats[1] = ByteToFloatNormalised(g);
	out_colorsAsFloats[2] = ByteToFloatNormalised(b);
	out_colorsAsFloats[3] = ByteToFloatNormalised(a);
}


bool Rgba8::operator!=(Rgba8 const& compare) const
{
	return !(*this == compare);
}


Rgba8 Rgba8::operator*(Rgba8 const& tint) const
{
	float color[4];
	GetColorAsFloats(color);
	float tintColor[4];
	tint.GetColorAsFloats(tintColor);

	float red = color[0] * tintColor[0];
	float green = color[1] * tintColor[1];
	float blue = color[2] * tintColor[2];
	float alpha = color[3] * tintColor[3];

	return Rgba8(FloatToByte(red),
				 FloatToByte(green),
				 FloatToByte(blue),
				 FloatToByte(alpha));
}


void Rgba8::operator*=(Rgba8 const& tint)
{
	*this = *this * tint;
}


bool Rgba8::operator==(Rgba8 const& compare) const
{
	if (r == compare.r && g == compare.g 
		&& b == compare.b && a == compare.a
		)
	{
		return true;
	}
	
	return false;
}


Rgba8::~Rgba8()
{
	//empty
}


Rgba8::Rgba8(const Rgba8& copyFrom) :
	r(copyFrom.r), g(copyFrom.g),
	b(copyFrom.b), a(copyFrom.a)
{
}


Rgba8::Rgba8(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : //default A = 255
	r(R),	g(G),
	b(B),	a(A)
{
}
