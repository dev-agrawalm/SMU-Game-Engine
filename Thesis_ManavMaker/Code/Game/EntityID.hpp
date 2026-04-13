#pragma once
#include <stdint.h>

struct EntityID
{
public:
	static EntityID GetIDForIndex(uint32_t index);
	static EntityID s_INVALID_ID;

public:
	uint32_t GetSalt() const;
	uint32_t GetIndex() const;

	bool IsValid() const;
	bool IsInvalid() const;

	bool operator==(EntityID const& toCompare) const;
	bool operator!=(EntityID const& toCompare) const;

public:
	uint32_t m_id = 0xffffffff;
};