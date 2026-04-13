#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"

NamedStrings g_gameConfigBlackboard;


EndinanMode GetNativeEndianMode()
{
	static uint32_t hexValue = 0x12345678;

	unsigned char* bytePointer = reinterpret_cast<unsigned char*>(&hexValue);
	if (bytePointer[0] == 0x78)
	{
		return EndinanMode::ENDIAN_MODE_LITTLE;
	}
	else
	{
		return EndinanMode::ENDIAN_MODE_BIG;
	}
}


void ReverseTwoBytes(unsigned char* bytes)
{
	uint16_t* bytesAsInt = reinterpret_cast<uint16_t*>(bytes);
	uint16_t reversedBytes = (*bytesAsInt & 0x00ff) << 8 | 
							 (*bytesAsInt & 0xff00) >> 8;
	*bytesAsInt = reversedBytes;
}


void ReverseFourBytes(unsigned char* bytes)
{
	uint32_t* bytesAsInt = reinterpret_cast<uint32_t*>(bytes);
	uint32_t reversedBytes = (*bytesAsInt & 0x000000ff) << 24 | 
							 (*bytesAsInt & 0x0000ff00) << 8  | 
							 (*bytesAsInt & 0x00ff0000) >> 8  | 
							 (*bytesAsInt & 0xff000000) >> 24;
	*bytesAsInt = reversedBytes;
}


void ReverseEightBytes(unsigned char* bytes)
{
	uint64_t* bytesAsInt = reinterpret_cast<uint64_t*>(bytes);
	uint64_t reversedBytes = (*bytesAsInt & 0x00000000000000ff) << 56 |
							 (*bytesAsInt & 0x000000000000ff00) << 40 |
							 (*bytesAsInt & 0x0000000000ff0000) << 24 | 
							 (*bytesAsInt & 0x00000000ff000000) << 8  | 
							 (*bytesAsInt & 0x000000ff00000000) >> 8  | 
							 (*bytesAsInt & 0x0000ff0000000000) >> 24 | 
							 (*bytesAsInt & 0x00ff000000000000) >> 40 | 
							 (*bytesAsInt & 0xff00000000000000) >> 56;
	*bytesAsInt = reversedBytes;
}


// constructor
ScopeTimer::ScopeTimer(const char* msg) 
	: m_message(msg)
	, m_startTime(GetCurrentTimeSeconds())
{
}

// destructor
ScopeTimer::~ScopeTimer()
{
	double duration = GetCurrentTimeSeconds() - m_startTime;
	g_console->AddLine(Rgba8::RED, Stringf("%s: %0.3f ms", m_message, (float) duration * 1000.0f));
}