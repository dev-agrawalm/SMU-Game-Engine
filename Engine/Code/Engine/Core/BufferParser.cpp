#include "Engine/Core/BufferParser.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/AABB2.hpp"

BufferParser::BufferParser(unsigned char const* buffer, uint32_t sizeOfBuffer)
	: m_buffer(buffer)
	, m_bufferSize(sizeOfBuffer)
{

}


BufferParser::~BufferParser()
{
	m_buffer = nullptr;
	m_readAt = 0;
	m_bufferSize = 0;
}


void BufferParser::JumpToOffset(uint32_t offsetFromStart)
{
	GUARANTEE_OR_DIE(offsetFromStart < m_bufferSize, Stringf("Trying to set read pointer outside buffer bounds. Buffer size: %i | read pointer offset: %i", m_bufferSize, offsetFromStart));
	m_readAt = offsetFromStart;
}


void BufferParser::MoveReadPointerForward(uint32_t forwardDistance)
{
	GUARANTEE_OR_DIE(m_readAt + forwardDistance < m_bufferSize, "Trying to move read pointer out of bounds");
	m_readAt += forwardDistance;
}


void BufferParser::MoveReadPointerBackward(uint32_t backwardDistance)
{
	GUARANTEE_OR_DIE(backwardDistance <= m_readAt, "Trying to move read pointer out of bounds");
	m_readAt -= backwardDistance;
}


void BufferParser::SetEndianMode(EndinanMode endianMode)
{
	EndinanMode nativeEndianMode = GetNativeEndianMode();
	m_isOppositeNativeEndianMode = nativeEndianMode != endianMode;
}


uint32_t BufferParser::GetReadLocation()
{
	return m_readAt;
}


std::string BufferParser::ParseNullTerminatedString()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(unsigned char));
	char const* string = reinterpret_cast<char const*>(bufferPointer);
	std::string parsedString(string);
	return parsedString;
}


std::string BufferParser::ParseStringAfter32BitLength()
{
	uint32_t length = ParseUInt32();
	std::string parsedString;
	parsedString.reserve(length);
	for (int i = 0; i < (int) length; i++)
	{
		char c = ParseChar();
		parsedString.push_back(c);
	}
	return parsedString;
}


unsigned char BufferParser::ParseByte()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(unsigned char));
	unsigned char parsedValue = *bufferPointer;
	return parsedValue;
}


char BufferParser::ParseChar()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(char));
	char parsedValue = *reinterpret_cast<char const*>(bufferPointer);
	return parsedValue;
}


short BufferParser::ParseShort()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(short));
	short parsedValue = *reinterpret_cast<short const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseTwoBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


unsigned short BufferParser::ParseUShort()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(unsigned short));
	unsigned short parsedValue = *reinterpret_cast<unsigned short const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseTwoBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


uint32_t BufferParser::ParseUInt32()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(uint32_t));
	uint32_t parsedValue = *reinterpret_cast<uint32_t const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseFourBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


int64_t BufferParser::ParseInt64()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(int64_t));
	int64_t parsedValue = *reinterpret_cast<int64_t const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseEightBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


uint64_t BufferParser::ParseUInt64()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(uint64_t));
	uint64_t parsedValue = *reinterpret_cast<uint64_t const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseEightBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


float BufferParser::ParseFloat()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(float));
	float parsedValue = *reinterpret_cast<float const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseFourBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


double BufferParser::ParseDouble()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(double));
	double parsedValue = *reinterpret_cast<double const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseEightBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


Vec2 BufferParser::ParseVec2()
{
	Vec2 parsedValue;
	parsedValue.x = ParseFloat();
	parsedValue.y = ParseFloat();
	return parsedValue;
}


int BufferParser::ParseInt()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(int));
	int parsedValue = *reinterpret_cast<int const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseFourBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


unsigned int BufferParser::ParseUInt()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(unsigned int));
	unsigned int parsedValue = *reinterpret_cast<unsigned int const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseFourBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


int32_t BufferParser::ParseInt32()
{
	unsigned char const* bufferPointer = GuaranteeSizeOfAndGetPointerToBuffer(sizeof(int32_t));
	int32_t parsedValue = *reinterpret_cast<int32_t const*>(bufferPointer);
	if (m_isOppositeNativeEndianMode)
	{
		unsigned char* parsedValueAsBytes = reinterpret_cast<unsigned char*>(&parsedValue);
		ReverseFourBytes(parsedValueAsBytes);
	}
	return parsedValue;
}


Vec3 BufferParser::ParseVec3()
{
	Vec3 parsedValue;
	parsedValue.x = ParseFloat();
	parsedValue.y = ParseFloat();
	parsedValue.z = ParseFloat();
	return parsedValue;
}


IntVec2 BufferParser::ParseIntVec2()
{
	IntVec2 parsedValue;
	parsedValue.x = ParseInt();
	parsedValue.y = ParseInt();
	return parsedValue;
}


IntVec3 BufferParser::ParseIntVec3()
{
	IntVec3 parsedValue;
	parsedValue.x = ParseInt();
	parsedValue.y = ParseInt();
	parsedValue.z = ParseInt();
	return parsedValue;
}


Rgba8 BufferParser::ParseRgb8()
{
	Rgba8 parsedValue = {};
	parsedValue.r = ParseByte();
	parsedValue.g = ParseByte();
	parsedValue.b = ParseByte();
	return parsedValue;
}


Rgba8 BufferParser::ParseRgba8()
{
	Rgba8 parsedValue {};
	parsedValue.r = ParseByte();
	parsedValue.g = ParseByte();
	parsedValue.b = ParseByte();
	parsedValue.a = ParseByte();
	return parsedValue;
}


Plane2 BufferParser::ParsePlane2()
{
	Plane2 plane2 = {};
	plane2.m_fwdNormal = ParseVec2();
	plane2.m_distanceFromOrigin = ParseFloat();
	return plane2;
}


AABB2 BufferParser::ParseAABB2()
{
	AABB2 parsedValue;
	parsedValue.m_mins = ParseVec2();
	parsedValue.m_maxs = ParseVec2();
	return parsedValue;
}


unsigned char const* BufferParser::GuaranteeSizeOfAndGetPointerToBuffer(uint32_t byteCountToRead)
{
	GUARANTEE_OR_DIE(m_readAt + byteCountToRead - 1 <= m_bufferSize, Stringf("Trying to read more bytes (%i bytes) than available in buffer(%i bytes) ", byteCountToRead, m_bufferSize));
	unsigned char const* bufferPointer = m_buffer + m_readAt;
	m_readAt += byteCountToRead;
	return bufferPointer;
}
