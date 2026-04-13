#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/NamedStrings.hpp"

#define UNUSED(x) (void)(x);

constexpr float MAX_SHORT_VALUE = 32767.0f;
constexpr float MIN_SHORT_VALUE = -32768.0f;

constexpr float MAX_UNSIGNED_CHAR_VALUE = 255.0f;
constexpr float MIN_UNSIGNED_CHAR_VALUE = 0.0f;

class DevConsole;
class EventSystem;
class JobSystem;

extern DevConsole* g_console;
extern EventSystem* g_eventSystem;
extern NamedStrings g_gameConfigBlackboard;
extern JobSystem* g_jobSystem;

enum class EndinanMode
{
	ENDIAN_MODE_BIG,
	ENDIAN_MODE_LITTLE
};

EndinanMode GetNativeEndianMode();

void ReverseTwoBytes(unsigned char* bytes);
void ReverseFourBytes(unsigned char* bytes);
void ReverseEightBytes(unsigned char* bytes);


struct ScopeTimer
{
	explicit ScopeTimer(const char* msg);
	~ScopeTimer();

	const char* m_message;
	double m_startTime;
};
