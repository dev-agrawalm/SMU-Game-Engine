#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>

#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED(applicationInstanceHandle);
	UNUSED( commandLineString );

	g_app = new App();
	g_app->Startup();

	// Program main loop; keep running frames until it's time to quit
	while( !g_app->IsQuitting() )
	{
		g_app->RunFrame();
	}

	g_app->ShutDown();
	delete g_app;
	g_app = nullptr;

	return 0;
}
