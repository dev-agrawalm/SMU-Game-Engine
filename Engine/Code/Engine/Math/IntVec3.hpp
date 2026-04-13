#pragma once

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

public:
	IntVec3();
	IntVec3(int x, int y, int z);
	IntVec3(IntVec3 const& copyFrom);

	IntVec3 operator+(IntVec3 const& add);
};
