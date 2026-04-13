#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/AABB2.hpp"

BufferWriter::BufferWriter(unsigned char* fixedSizedBuffer, uint32_t sizeofBuffer)
	: m_fixedSizedBuffer(fixedSizedBuffer)
	, m_bufferSize(sizeofBuffer)
{

}


BufferWriter::BufferWriter(std::vector<unsigned char>* dynamicBuffer)
	: m_dynamicBuffer(dynamicBuffer)
{

}


BufferWriter::~BufferWriter()
{
	m_dynamicBuffer = nullptr;
	m_fixedSizedBuffer = nullptr;
	m_bufferSize = 0;
	m_writeAt = 0;
}


void BufferWriter::AppendUInt32(uint32_t valueToAppend)
{
	unsigned char* uint32Bytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseFourBytes(uint32Bytes);
	}
	AppendBytesToBuffer(uint32Bytes, sizeof(uint32_t));
}


void BufferWriter::AppendUInt64(uint64_t valueToAppend)
{
	unsigned char* intBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseEightBytes(intBytes);
	}
	AppendBytesToBuffer(intBytes, sizeof(uint64_t));
}


void BufferWriter::AppendInt64(int64_t valueToAppend)
{
	unsigned char* intBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseEightBytes(intBytes);
	}
	AppendBytesToBuffer(intBytes, sizeof(int64_t));
}


void BufferWriter::AppendInt(int valueToAppend)
{
	unsigned char* intBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseFourBytes(intBytes);
	}
	AppendBytesToBuffer(intBytes, sizeof(int));
}


void BufferWriter::AppendUInt(unsigned int valueToAppend)
{
	unsigned char* intBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseFourBytes(intBytes);
	}
	AppendBytesToBuffer(intBytes, sizeof(unsigned int));
}


void BufferWriter::AppendInt32(int32_t valueToAppend)
{
	unsigned char* intBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseFourBytes(intBytes);
	}
	AppendBytesToBuffer(intBytes, sizeof(int32_t));
}


void BufferWriter::AppendVec2(Vec2 const& valueToAppend)
{
	AppendFloat(valueToAppend.x);
	AppendFloat(valueToAppend.y);
}


void BufferWriter::AppendIntVec2(IntVec2 const& valueToAppend)
{
	AppendInt(valueToAppend.x);
	AppendInt(valueToAppend.y);
}


void BufferWriter::AppendVec3(Vec3 const& valueToAppend)
{
	AppendFloat(valueToAppend.x);
	AppendFloat(valueToAppend.y);
	AppendFloat(valueToAppend.z);
}


void BufferWriter::AppendIntVec3(IntVec3 const& valueToAppend)
{
	AppendInt(valueToAppend.x);
	AppendInt(valueToAppend.y);
	AppendInt(valueToAppend.z);
}


void BufferWriter::AppendRgb8(Rgba8 const& valueToAppend)
{
	AppendByte(valueToAppend.r);
	AppendByte(valueToAppend.g);
	AppendByte(valueToAppend.b);
}


void BufferWriter::AppendRgba8(Rgba8 const& valueToAppend)
{
	AppendByte(valueToAppend.r);
	AppendByte(valueToAppend.g);
	AppendByte(valueToAppend.b);
	AppendByte(valueToAppend.a);
}


void BufferWriter::AppendPlane2(Plane2 const& valueToAppend)
{
	AppendVec2(valueToAppend.m_fwdNormal);
	AppendFloat(valueToAppend.m_distanceFromOrigin);
}


void BufferWriter::AppendAABB2(AABB2 const& valueToAppend)
{
	AppendVec2(valueToAppend.m_mins);
	AppendVec2(valueToAppend.m_maxs);
}


void BufferWriter::SetEndianMode(EndinanMode endianMode)
{
	EndinanMode nativeEndianMode = GetNativeEndianMode();
	m_isOppositeNativeEndianMode = nativeEndianMode != endianMode;
}


void BufferWriter::OverwriteBytesInBuffer(unsigned char const* bytes, int numBytes, uint32_t offsetFromStart)
{
	if (m_dynamicBuffer)
	{
		if (offsetFromStart >= m_dynamicBuffer->size())
		{
			for (int i = 0; i < numBytes; i++)
			{
				unsigned char const* byte = bytes + i;
				m_dynamicBuffer->push_back(*byte);
			}
		}
		else
		{
			for (int i = 0; i < numBytes; i++)
			{
				unsigned char const* byte = bytes + i;
				std::vector<unsigned char>& bufferRef = *m_dynamicBuffer;
				bufferRef[offsetFromStart + i] = *byte;
			}
		}
		
	}
	else
	{
		GUARANTEE_OR_DIE(offsetFromStart < m_bufferSize, Stringf("Trying to overwrite outside buffer bounds. Overwrite offset: %i | Buffer Size: %i", offsetFromStart, m_bufferSize).c_str());
		for (int i = 0; i < numBytes; i++)
		{
			unsigned char byte = *(bytes + i);
			*(m_fixedSizedBuffer + offsetFromStart + i) = byte;
		}
	}
}


void BufferWriter::AppendNullTerminatedString(std::string const& valueToAppend)
{
	for (int charIndex = 0; charIndex < (int) valueToAppend.size(); charIndex++)
	{
		AppendChar(valueToAppend[charIndex]);
	}
	AppendChar('\0');
}


void BufferWriter::AppendStringAfter32BitLength(std::string const& valueToAppend)
{
	uint32_t length = (uint32_t) valueToAppend.length();
	AppendUInt32(length);
	for (int charIndex = 0; charIndex < (int) valueToAppend.size(); charIndex++)
	{
		AppendChar(valueToAppend[charIndex]);
	}
}


void BufferWriter::AppendByte(unsigned char valueToAppend)
{
	AppendBytesToBuffer(&valueToAppend, sizeof(unsigned char));
}


void BufferWriter::AppendChar(char valueToAppend)
{
	unsigned char* charByte = reinterpret_cast<unsigned char*>(&valueToAppend);
	AppendByte(*charByte);
}


void BufferWriter::AppendShort(short valueToAppend)
{
	unsigned char* shortBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseTwoBytes(shortBytes);
	}
	AppendBytesToBuffer(shortBytes, sizeof(short));
}


void BufferWriter::AppendUShort(unsigned short valueToAppend)
{
	unsigned char* shortBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseTwoBytes(shortBytes);
	}
	AppendBytesToBuffer(shortBytes, sizeof(unsigned short));
}


void BufferWriter::AppendFloat(float valueToAppend)
{
	unsigned char* floatBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseFourBytes(floatBytes);
	}
	AppendBytesToBuffer(floatBytes, sizeof(float));
}


void BufferWriter::AppendDouble(double valueToAppend)
{
	unsigned char* doubleBytes = reinterpret_cast<unsigned char*>(&valueToAppend);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseEightBytes(doubleBytes);
	}
	AppendBytesToBuffer(doubleBytes, sizeof(double));
}


void BufferWriter::AppendBytesToBuffer(unsigned char const* bytesStart, uint32_t numBytes)
{
	if (m_dynamicBuffer)
	{
		for (int i = 0; i < (int) numBytes; i++)
		{
			unsigned char byte = *(bytesStart + i);
			m_dynamicBuffer->push_back(byte);
		}
	}
	else
	{
		GUARANTEE_OR_DIE(m_writeAt + numBytes - 1 <= m_bufferSize, Stringf("Trying to write more bytes (%i bytes) to buffer than available (% bytes)", numBytes, m_bufferSize).c_str());
		for (int i = 0; i < (int) numBytes; i++)
		{
			unsigned char byte = *(bytesStart + i);
			*(m_fixedSizedBuffer + m_writeAt + i) = byte;
		}
		m_writeAt += numBytes;
	}
}


uint32_t BufferWriter::GetTotalBufferSize()
{
	if (m_dynamicBuffer)
	{
		return (uint32_t) m_dynamicBuffer->size();
	}
	
	return m_bufferSize;
}


void BufferWriter::OverwriteUInt32(uint32_t value, uint32_t offsetFromStart)
{
	unsigned char* uint32Bytes = reinterpret_cast<unsigned char*>(&value);
	if (m_isOppositeNativeEndianMode)
	{
		ReverseFourBytes(uint32Bytes);
	}
	OverwriteBytesInBuffer(uint32Bytes, sizeof(uint32_t), offsetFromStart);
}


