#include "Game/EntityID.hpp"

EntityID EntityID::s_INVALID_ID = {0xffffffff};

uint32_t EntityID::GetSalt() const
{
	uint32_t saltMask = 0xffff0000;
	uint32_t salt = (saltMask & m_id) >> 16;
	return salt;
}


uint32_t EntityID::GetIndex() const
{
	uint32_t indexMask = 0x0000ffff;
	uint32_t index = (indexMask & m_id);
	return index;
}


bool EntityID::IsValid() const
{
	return *this != s_INVALID_ID;
}


bool EntityID::IsInvalid() const
{
	return *this == s_INVALID_ID;
}


bool EntityID::operator!=(EntityID const& toCompare) const
{
	return m_id != toCompare.m_id;
}


bool EntityID::operator==(EntityID const& toCompare) const
{
	return m_id == toCompare.m_id;
}


uint32_t g_salt = 0x0000;

EntityID EntityID::GetIDForIndex(uint32_t index)
{
	EntityID entityId = {};
	g_salt += 1;
	if (g_salt == 0xffff)
		g_salt = 0x0000;
	entityId.m_id = g_salt << 16 | index;
	return entityId;
}
