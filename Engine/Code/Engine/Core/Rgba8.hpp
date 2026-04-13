#pragma once
#include<string>

//struct representing 8 bit colors
struct Rgba8
{
	//gray scale
	static Rgba8 const WHITE;
	static Rgba8 const WHITE_TRANSPARENT;
	static Rgba8 const GREY;
	static Rgba8 const GREY_TRANSPARENT;
	static Rgba8 const BLACK;
	static Rgba8 const BLACK_TRANSPARENT;
	//RGB
	static Rgba8 const RED;
	static Rgba8 const RED_TRANSPARENT;
	static Rgba8 const BLUE;
	static Rgba8 const BLUE_TRANSPARENT;
	static Rgba8 const GREEN;
	static Rgba8 const GREEN_TRANSPARENT;
	//YCM
	static Rgba8 const YELLOW;
	static Rgba8 const YELLOW_TRANSPARENT;
	static Rgba8 const CYAN;
	static Rgba8 const CYAN_TRANSPARENT;
	static Rgba8 const MAGENTA;
	static Rgba8 const MAGENTA_TRANSPARENT;

public: // members
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

public: //methods
	Rgba8();// does nothing
	~Rgba8(); // does nothing
	Rgba8(const Rgba8& copyFrom); //copy constructor(from another Rgba8)
	explicit Rgba8(unsigned char R, unsigned char G, unsigned char B, unsigned char A = 255); //explicit constructor for a color (r,g,b,a)

	//mutators
	void SetFromText(std::string text);
	void ScaleRGB(float scale);

	//getters
	void GetColorAsFloats(float* out_colorsAsFloats) const;

	//operators
	bool operator==(Rgba8 const& compare) const;
	bool operator!=(Rgba8 const& compare) const;
	Rgba8 operator*(Rgba8 const& tint) const;
	void operator*=(Rgba8 const& tint);
};
