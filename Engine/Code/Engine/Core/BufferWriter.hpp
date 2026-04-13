#pragma once
#include <stdint.h>
#include <vector>
#include "Engine/Core/EngineCommon.hpp"

struct Rgba8;
struct Vec2;
struct IntVec2;
struct Vec3;
struct IntVec3;
struct Plane2;
struct AABB2;

class BufferWriter
{
public:
	BufferWriter(unsigned char* fixedSizedBuffer, uint32_t sizeofBuffer);
	BufferWriter(std::vector<unsigned char>* dynamicBuffer);
	~BufferWriter();
	void SetEndianMode(EndinanMode endianMode);

	void AppendNullTerminatedString(std::string const& valueToAppend);
	void AppendStringAfter32BitLength(std::string const& valueToAppend);
	void AppendByte(unsigned char valueToAppend);
	void AppendChar(char valueToAppend);
	void AppendShort(short valueToAppend);
	void AppendUShort(unsigned short valueToAppend);
	void AppendInt(int valueToAppend);
	void AppendUInt(unsigned int valueToAppend);
	void AppendInt32(int32_t valueToAppend);
	void AppendUInt32(uint32_t valueToAppend);
	void AppendInt64(int64_t valueToAppend);
	void AppendUInt64(uint64_t valueToAppend);
	void AppendFloat(float valueToAppend);
	void AppendDouble(double valueToAppend);
	void AppendVec2(Vec2 const& valueToAppend);
	void AppendIntVec2(IntVec2 const& valueToAppend);
	void AppendVec3(Vec3 const& valueToAppend);
	void AppendIntVec3(IntVec3 const& valueToAppend);
	void AppendRgb8(Rgba8 const& valueToAppend);
	void AppendRgba8(Rgba8 const& valueToAppend);
	void AppendPlane2(Plane2 const& valueToAppend);
	void AppendAABB2(AABB2 const& valueToAppend);
	void AppendBytesToBuffer(unsigned char const* bytes, uint32_t numBytes);

	uint32_t GetTotalBufferSize();

	void OverwriteUInt32(uint32_t value, uint32_t offsetFromStart);
	void OverwriteBytesInBuffer(unsigned char const* bytesStart, int numBytes, uint32_t offsetFromStart);
private:
	std::vector<unsigned char>* m_dynamicBuffer = nullptr;
	unsigned char* m_fixedSizedBuffer = nullptr;
	uint32_t m_writeAt = 0;
	uint32_t m_bufferSize = 0;
	bool m_isOppositeNativeEndianMode = false;
};
