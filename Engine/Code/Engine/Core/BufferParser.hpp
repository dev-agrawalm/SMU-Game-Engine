#pragma once
#include <stdint.h>
#include "Engine/Core/EngineCommon.hpp"

struct Plane2;
struct Vec2;
struct IntVec2;
struct Vec3;
struct IntVec3;
struct Rgba8;
struct AABB2;

class BufferParser
{
public:
	BufferParser(unsigned char const* buffer, uint32_t sizeOfBuffer);
	~BufferParser();

	void JumpToOffset(uint32_t offsetFromStart);
	void MoveReadPointerForward(uint32_t forwardDistance);
	void MoveReadPointerBackward(uint32_t backwardDistance);
	void SetEndianMode(EndinanMode endianMode);
	uint32_t GetReadLocation();

	std::string							ParseNullTerminatedString();
	std::string							ParseStringAfter32BitLength();
	unsigned char						ParseByte();
	char								ParseChar();
	short								ParseShort();
	unsigned short						ParseUShort();
	int									ParseInt();
	unsigned int						ParseUInt();
	int32_t								ParseInt32();
	uint32_t							ParseUInt32();
	int64_t								ParseInt64();
	uint64_t							ParseUInt64();
	float								ParseFloat();
	double								ParseDouble();
	Vec2								ParseVec2();
	Vec3								ParseVec3();
	IntVec2								ParseIntVec2();
	IntVec3								ParseIntVec3();
	Rgba8								ParseRgb8();
	Rgba8								ParseRgba8();
	Plane2								ParsePlane2();
	AABB2								ParseAABB2();

	unsigned char const* GuaranteeSizeOfAndGetPointerToBuffer(uint32_t byteCountToRead);
private:
	unsigned char const* m_buffer = nullptr;
	uint32_t m_bufferSize = 0;
	uint32_t m_readAt = 0;
	bool m_isOppositeNativeEndianMode = false;
};
