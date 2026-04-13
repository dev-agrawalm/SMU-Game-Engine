#pragma once
#include "Game/GameCommon.hpp"

#include <vector>



//------------------------------------------------------------------------------------------------
struct ActorUID
{
public:
	ActorUID(); 
	ActorUID( int index, int salt ); 

	void Invalidate(); 

	bool IsValid() const;
	int GetIndex() const;

	inline bool operator==( ActorUID const& other ) const	{ return m_data == other.m_data; }

public:
	static int GetNextSalt( int curSalt ); 
	static ActorUID const INVALID; 

public:
	int m_data = -1; 
}; 

