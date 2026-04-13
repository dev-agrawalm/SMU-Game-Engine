#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


std::string RemoveWhitespacesFromString(std::string const& string)
{
	std::string resultString = "";
	for (int charIndex = 0; charIndex < string.size(); charIndex++)
	{
		if (isspace(string[charIndex]))
		{
			continue;
		}
		resultString += string[charIndex];
	}

	return resultString;
}


Strings SplitStringOnDelimiter(std::string const& stringToSplit, char delimiter)
{
	Strings strings;
	std::string tempString = "";
	for (int charIndex = 0; charIndex < stringToSplit.length(); charIndex++)
	{
		if (stringToSplit[charIndex] == delimiter)
		{
			strings.push_back(tempString);
			tempString = "";
			continue;
		}
		tempString += stringToSplit[charIndex];
	}
	strings.push_back(tempString);	
	return strings;
}


std::string BoolToString(bool boolVal)
{
	if (boolVal == true)
	{
		return std::string("True");
	}
	else
	{
		return std::string("False");
	}
}


std::string ClockToString(Clock* clock)
{
	double totalTime = clock->GetTotalSeconds();
	double deltaTime = clock->GetFrameDeltaSeconds();
	double timeScale = clock->GetTimeScale();
	bool isPaused = clock->IsPaused();

	int totalTimeMinutes = RoundDownToInt((float) (totalTime * (1.0 / 60.0)));
	int totalTimeHours = RoundDownToInt((float) (totalTime * (1.0 / 3600.0)));
	int leftoverMinutes = totalTimeMinutes - (totalTimeHours * 60);
	double leftoverSeconds = totalTime - (double) totalTimeHours * 3600.0 - (double) leftoverMinutes * 60.0;
	std::string totalTimeString = Stringf("Total Time: %ih, %im, %.2fs", totalTimeHours, leftoverMinutes, leftoverSeconds);
	double frameTimeMiliSeconds = deltaTime * 1000.0;
	std::string deltaTimeString = Stringf("Frame Time: %.2fms", frameTimeMiliSeconds);
	std::string isPausedString = Stringf("Is Paused: %s", BoolToString(isPaused).c_str());
	std::string timeScaleString = Stringf("Time scale: %.1f", timeScale);

	return totalTimeString + " | " + deltaTimeString + " | " + isPausedString + " | " + timeScaleString;
}


