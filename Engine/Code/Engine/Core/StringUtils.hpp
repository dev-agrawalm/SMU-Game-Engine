#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

typedef std::vector<std::string> Strings;

std::string	RemoveWhitespacesFromString(std::string const& string);
Strings SplitStringOnDelimiter(std::string const& stringToSplit, char delimiter);


class Clock;

//type to string utils
std::string BoolToString(bool boolVal);
std::string ClockToString(Clock* clock);