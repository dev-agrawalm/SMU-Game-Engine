#include "Engine/Math/IntVec3.hpp"

IntVec3::IntVec3()
{

}


IntVec3::IntVec3(int fromX, int fromY, int fromZ)
	: x(fromX)
	, y(fromY)
	, z(fromZ)
{

}


IntVec3::IntVec3(IntVec3 const& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
	, z(copyFrom.z)
{

}


IntVec3 IntVec3::operator+(IntVec3 const& add)
{
	return IntVec3(x + add.x, y + add.y, z + add.z);
}
