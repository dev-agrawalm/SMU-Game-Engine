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

	g_theApp = new App();
	g_theApp->Startup();

	// Program main loop; keep running frames until it's time to quit
	while( !g_theApp->IsQuitting() )
	{
		g_theApp->RunFrame();
	}

	g_theApp->ShutDown();
	delete g_theApp;
	g_theApp = nullptr;

	for (int threadIndex = 0; threadIndex < NUM_THREADS; threadIndex++)
	{
		if (g_threads[threadIndex] != nullptr)
		{
			g_threads[threadIndex]->join();
			delete g_threads[threadIndex];
			g_threads[threadIndex] = nullptr;
		}
	}

	return 0;
}
