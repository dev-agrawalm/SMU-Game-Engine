#include "Game/ActorUID.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

int g_indexMask = 0xffff;

ActorUID::ActorUID()
{

}


ActorUID::ActorUID(int index, int salt)
{
	ASSERT_OR_DIE(index <= 0xffff, Stringf("Invalid Index: %i", index));
	ASSERT_OR_DIE(salt < 0xffff, Stringf("Invalid Salt: %i", salt));
	m_data = index | (salt << 16);
}


void ActorUID::Invalidate()
{
	*this = ActorUID::INVALID;
}


bool ActorUID::IsValid() const
{
	return !(*this == ActorUID::INVALID);
}


int ActorUID::GetIndex() const
{
	return m_data & g_indexMask;
}


int ActorUID::GetNextSalt(int curSalt)
{
	int nextSalt = curSalt + 1;
	if (nextSalt == 0xffff)
		return 0;

	return nextSalt;
}

ActorUID const ActorUID::INVALID;
