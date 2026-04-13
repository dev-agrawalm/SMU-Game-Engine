#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Entity.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include<vector>
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/Image.hpp"

RandomNumberGenerator* g_rng = nullptr;
BitmapFont* g_bitmapFontTier1 = nullptr;
BitmapFont* g_bitmapFontTier2 = nullptr;

Game::Game()
{

}


// bool Test1(EventArgs& args)
// {
// 	g_console->AddLine(DevConsole::WARNING_MESSAGE, args.GetValue("Weird Name1", "")
// 					   + "\n" + args.GetValue("Weird Name2", "")
// 					   + "\n" + args.GetValue("Weird Name3", ""));
// 	args.SetValue("Weird Name1", "Rohit New");
// 	return false;
// }
// 
// bool Test2(EventArgs& args)
// {
// 	g_console->AddLine(DevConsole::WARNING_MESSAGE, args.GetValue("Weird Name1", "")
// 					   + "\n" + args.GetValue("Weird Name2", "")
// 					   + "\n" + args.GetValue("Weird Name3", ""));
// 	args.SetValue("Weird Name2", "Varun New");
// 	return true;
// }
// 
// bool Test3(EventArgs& args)
// {
// 	g_console->AddLine(DevConsole::WARNING_MESSAGE, args.GetValue("Weird Name1", "")
// 					   + "\n" + args.GetValue("Weird Name2", "")
// 					   + "\n" + args.GetValue("Weird Name3", ""));
// 	args.SetValue("Weird Name3", "Manav");
// 	return false;
// }
// 
// 
// void EventSystemTest()
// {
// 	EventArgs args;
// 	args.SetValue("Weird Name1", "Rohit");
// 	args.SetValue("Weird Name2", "Varun");
// 	args.SetValue("Weird Name3", "Manav");
// 
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->SubscribeEventCallbackFunction("test", Test1);
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->SubscribeEventCallbackFunction("test", Test2);
// 	g_eventSystem->SubscribeEventCallbackFunction("test", Test3);
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->UnsubscribeEventCallbackFunction("test", Test3);
// 	g_eventSystem->FireEvent("test");
// 	g_eventSystem->FireEvent("test", args);
// }



Game::~Game()
{
}

void SpecialSortArray_TimeComplexityN(int* intArr, int numInts)
{
	//merge sort
	std::vector<int> nonZeroIntArr;
	std::vector<int> zeroIntArr;
	nonZeroIntArr.resize(numInts);
	zeroIntArr.resize(numInts);
	int nonZeroIntCount = 0;
	int zeroIntCount = 0;
	for (int index = 0; index < numInts; index++)
	{
		int refInt = intArr[index];
		if (refInt == 0)
		{
			zeroIntArr[zeroIntCount++] = refInt;
		}
		else
		{
			nonZeroIntArr[nonZeroIntCount++] = refInt;
		}
	}

	for (int index = 0; index < nonZeroIntCount; index++)
	{
		intArr[index] = nonZeroIntArr[index];
	}

	for (int index = 0; index < zeroIntCount; index++)
	{
		intArr[nonZeroIntCount + index] = zeroIntArr[index];
	}
}


void SpecialSortArray_TimeComplexityNSquared(int* intArr, int numInts)
{
	//bubble sort algorithm
	for (int lastIntIndex = numInts - 1; lastIntIndex > 0; lastIntIndex--)
	{
		for (int intIndex = 0; intIndex < lastIntIndex; intIndex++)
		{
			int curInt = intArr[intIndex];
			int nextInt = intArr[intIndex + 1];
			if (curInt == 0)
			{
				intArr[intIndex] = nextInt;
				intArr[intIndex + 1] = curInt;
			}
		}
	}
}


int MultiplyBy321(int value)
{
	// 321 = 0001 0100 0001 = 2^8 + 2^6 + 2^0
	// x*321 = x*2^8 + x*2^6 + x*2^0
	// x*2^n = x << n
	int rslt = (value << 8) + (value << 6) + value; //value << 0 = value
	return rslt;
}


void Game::Startup()
{

	g_rng = new RandomNumberGenerator();
	m_gameMode = GameMode::ATTRACT;
	InitAttractMode();
	DebugRenderSetVisible();
//	EventSystemTest();

	std::vector<uint8_t> byteBuffer;
	std::string charBuffer;

	FileReadToBuffer(byteBuffer, "Data/Test.txt");
	FileReadToString(charBuffer, "Data/Test.txt");

// 	int result = MultiplyBy321(6);
// 	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("6*321 = %i", result));
// 	result = MultiplyBy321(11);
// 	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("11*321 = %i", result));
// 	result = MultiplyBy321(576);
// 	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("576*321 = %i", result));
// 	result = MultiplyBy321(1);
// 	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("1*321 = %i", result));	
// 	result = MultiplyBy321(20);
// 	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("20*321 = %i", result));
// 	result = MultiplyBy321(-3);
// 	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("-3*321 = %i", result));
// 	result = MultiplyBy321(-73);
// 	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("-73*321 = %i", result));


// 	for (int i = 0; i < 10; i++)
// 	{
// 		int arr[120] = {7, 20, 29, 0, 0, 64, 98, 5, 2, 8, 7, 20, 29, 0, 0, 64, 98, 5, 2, 8, 7, 20, 29, 0, 0, 64, 98, 5, 2, 8, 7, 20, 29, 0, 0, 64, 98, 5, 2, 8
// 		, 2, 9, 29, 10, 0, 64, 0, 5, 2, 18, 29, 0, 19, 0, 0, 64, 2, 0, 2, 3, 4, 64, 98, 0, 2, 0, 98, 5, 2, 8, 7, 20, 29, 0, 0, 64, 98, 5, 2, 8
// 		, 7, 20, 0, 5, 2, 18, 29, 5, 2, 8, 7, 20, 29, 7, 20, 29, 0, 0, 2, 8, 7, 20, 29, 0, 0, 2, 0, 2, 3, 4, 64, 98, 0, 2, 0, 64, 98, 5, 2, 8};
// 		int arr1[10] = {2, 0, 2, 3, 4, 64, 98, 0, 2, 0};
// 
// 		float time1 = (float) GetCurrentTimeSeconds();
// 		SpecialSortArray_TimeComplexityN(arr1, 10);
// 		float time2 = (float) GetCurrentTimeSeconds();
// 		float timeTakenV1 = (time2 - time1) * 1000.0f;
// 		g_console->AddLine(DevConsole::MINOR_INFO, Stringf("Time Taken V1 (Merge):  %0.5fms", timeTakenV1));
// 
// 		float time3 = (float) GetCurrentTimeSeconds();
// 		SpecialSortArray_TimeComplexityNSquared(arr1, 10);
// 		float time4 = (float) GetCurrentTimeSeconds();
// 		float timeTakenV2 = (time4 - time3) * 1000.0f;
// 		g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Time Taken V2 (Bubble): %0.5fms", timeTakenV2));
// 	  	int numInts = 10;
//  		for (int j = 0; j < numInts; j++)
//  		{
//  			if (j == numInts - 1)
//  			{
//  				g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("%i ", arr1[j]));
//  			}
//  			else
//  			{
//  				g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("%i, ", arr1[j]));
//  			}
//  		}
// 	}

	LoadAssets();
}


void ReverseString(const char* pString, char* pResult)
{
	int charIndex = 0;
	char c = *pString;
	while (c != '\0')
	{
		c = *(pString + charIndex + 1);
		charIndex++;
	}

	int nullCharIndex = charIndex;
	charIndex--;
	int numCharacters = charIndex;
	*(pResult + nullCharIndex) = '\0';
	for (charIndex; charIndex >= 0; charIndex--)
	{
		int reversedStringCharIndex = numCharacters - charIndex;
		*(pResult + reversedStringCharIndex) = *(pString + charIndex);
	}
}



void Game::LoadAssets()
{
	//load audio
	g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");

	g_bitmapFontTier1 = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	g_bitmapFontTier2 = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelPix_MedBoldProp");
	Image fontImage = Image("Data/Fonts/SquirrelPix_MedBoldProp.png");
	g_bitmapFontTier2->InitialiseUsingImage(fontImage);
}


void Game::ShutDown()
{
	switch (m_gameMode)
	{
		case GameMode::ATTRACT:
			DeInitAttractMode();
			break;
		case GameMode::PLAY:
			DeInitPlayMode();
			break;
		case GameMode::NONE:
		default:
			break;
	}

	delete g_rng;
	g_rng = nullptr;
}


void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void Game::CheckInputDeveloperCheats()
{
	//enable debug renderer
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugMode = !m_debugMode;
	}

	if (g_inputSystem->IsKeyPressed('O'))
	{
		m_runSingleStep = true;
		m_isPaused = false;
	}

	//pause game
	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_isPaused = !m_isPaused;
	}

	//manipulate time scale
	if (g_inputSystem->IsKeyPressed('T'))
	{
		m_timeScale = 0.2f;
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		m_timeScale = 5.0f;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}

	if (g_inputSystem->WasKeyJustPressed('L'))
	{
		g_console->AddLine(DevConsole::MINOR_INFO, "L was just pressed");
	}
}


void Game::Update(float deltaSeconds)
{
	m_timeScale = 1.0f;
	CheckInputDeveloperCheats();
	deltaSeconds *= m_timeScale;

	if (m_isPaused)
		return;

	//check input
	switch (m_gameMode)
	{
		case GameMode::ATTRACT:	CheckInputAttractMode();			break;
		case GameMode::PLAY:	CheckInputPlayMode(deltaSeconds);	break;
		case GameMode::NONE:
		default:
			break;
	}

	//update game mode
	switch (m_gameMode)
	{
		case GameMode::ATTRACT:	UpdateAttractMode(deltaSeconds);	break;
		case GameMode::PLAY:	UpdatePlayMode(deltaSeconds);		break;
		case GameMode::NONE:
		default:
			break;
	}

	UpdateCameras(deltaSeconds);

	if (m_runSingleStep)
	{
		m_runSingleStep = false;
		m_isPaused = true;
	}

}


void Game::UpdateCameras(float deltaSeconds)
{
	float worldSizeX	= g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
	float worldSizeY	= g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
	UNUSED(worldSizeX);
	UNUSED(worldSizeY);

	float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);

	//m_worldCamera.SetOrthoView(0.0f, 0.0f, worldSizeX, worldSizeY);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiCanvasSizeX, uiCanvasSizeY);

	CameraData activeCamData = m_camData[m_activeCamIndex];
	Mat44 modelMat;
	modelMat.SetTranslation3D(activeCamData.m_camPosition);
	Mat44 orientationMat = activeCamData.m_camOrientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMat.Append(orientationMat);
	m_worldCamera.SetCameraModelMatrix(modelMat);

	float camFov = activeCamData.m_camFov;
	float camAspect = g_window->GetWindowConfig().m_aspectRatio;
	if (activeCamData.m_isFreeAspect)
	{
		float camHeight = activeCamData.m_camHeight;
		float camWidth = activeCamData.m_camWidth;
		camAspect = camWidth / camHeight;
	}
	float camNear = 0.1f;
	float camAway = 1000.0f;
	Mat44 perspectiveMatrix = Mat44::CreatePerspectiveProjectionMatrix(camFov, camAspect, camNear, camAway);
	m_worldCamera.SetProjectionMatrix(perspectiveMatrix);

	if (m_shouldShakeWorldCam)
	{
		m_worldCamTimeSinceScreenShakeStart += deltaSeconds;
		float shakeMagnitude = RangeMapClamped(m_worldCamTimeSinceScreenShakeStart, 0.0f, m_worldCamScreenShakeDuration, m_worldCamScreenShakeMagnitude, 0.0f);
		float shakeX = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		float shakeY = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		m_worldCamera.TranslateCamera2D(Vec2(shakeX, shakeY));
		m_shouldShakeWorldCam = shakeMagnitude > 0.0f ? true : false;
	}

	if (m_shouldShakeUICam)
	{
		m_uiCamTimeSinceScreenShakeStart += deltaSeconds;
		float shakeMagnitude = RangeMapClamped(m_uiCamTimeSinceScreenShakeStart, 0.0f, m_uiCamScreenShakeDuration, m_uiCamScreenShakeMagnitude, 0.0f);
		float shakeX = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		float shakeY = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		m_uiCamera.TranslateCamera2D(Vec2(shakeX, shakeY));
		m_shouldShakeUICam = shakeMagnitude > 0.0f ? true : false;
	}
}


void Game::Render() const
{
	switch (m_gameMode)
	{
		case GameMode::ATTRACT:
			RenderAttractMode();
			break;
		case GameMode::PLAY:
			RenderPlayMode();
			break;
		case GameMode::NONE:
		default:
			break;
	}

	DebugRenderWorldToCamera(m_worldCamera);
}


void Game::InitAttractMode()
{
	m_animationTime = 0.0f;
}


void Game::DeInitAttractMode()
{
}


void Game::CheckInputAttractMode()
{
	//keep quitting code in here and shift rest to their respective places
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER))
	{
		SetGameMode(GameMode::PLAY);
	}
}


void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_animationTime += deltaSeconds;
}


void Game::RenderAttractMode() const
{
	g_theRenderer->ClearScreen(Rgba8::GREEN);

	g_theRenderer->BeginCamera(m_uiCamera);
 	//g_theRenderer->BindShaderByName("Data/Shaders/Default");

	static float s_uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float s_uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
 	BitmapFontTest(s_uiCanvasSizeX, s_uiCanvasSizeY);
// 	SpriteAnimationTest(s_uiCanvasSizeX, s_uiCanvasSizeY);

// 	std::vector<Vertex_PCU> verts;
// 	AABB2 textureBox = AABB2(Vec2(800.0f, 400.0f), 400.0f, 400.0f);
// 	AddVertsForAABB2ToVector(verts, textureBox, Rgba8::WHITE);
// 	g_theRenderer->BindTextureByPath(0, "Data/Images/Test_StbiFlippedAndOpenGL.png");
// 	g_theRenderer->DrawVertexArray(verts.size(), verts.data());

	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::SpriteAnimationTest(float uiCanvasSizeX, float uiCanvasSizeY) const
{
	static Texture* s_texture = g_theRenderer->CreateOrGetTexture("Data/Images/Explosion_5x5.png");
	static SpriteSheet s_spriteSheet(*s_texture, IntVec2(5, 5));
	static SpriteAnimDefinition animation(s_spriteSheet, 0, 24, 1.0f, SpriteAnimPlaybackMode::LOOP);

	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);

	SpriteDefinition spriteDef = animation.GetSpriteDefAtTime(m_animationTime);
	Vec2 uvMins;
	Vec2 uvMaxs;
	spriteDef.GetUVs(uvMins, uvMaxs);
	float textBoxWidth = 600.0f;
	float textBoxHeight = 400.0f;
	Vec2 textBoxCenter = Vec2(uiCanvasSizeX * 0.5f, uiCanvasSizeY * 0.5f);
	AABB2 textBox = AABB2(textBoxCenter, textBoxWidth, textBoxHeight);
	std::vector<Vertex_PCU> renderVertexes;
	AddVertsForAABB2ToVector(renderVertexes, textBox, Rgba8::WHITE, uvMins, uvMaxs);
	g_theRenderer->BindTexture(0, &spriteDef.GetTexture());
	g_theRenderer->DrawVertexArray((int) renderVertexes.size(), renderVertexes.data());

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	int spriteIndex = spriteDef.GetSpriteSheetIndex();
	char indexText[100];
	_itoa_s(spriteIndex, indexText, 10);
	std::string text = indexText;
	renderVertexes.clear();
	//g_bitmapFont->AddVertsForTextInAABB2(renderVertexes, textBox, 10.f, text, Rgba8::BLACK, 1.0f, BitmapFont::ALIGNED_TOP_RIGHT, TextDrawMode::SHRINK_TO_FIT);
	g_bitmapFontTier2->AddVertsForText2D(renderVertexes, Vec2(60.0f, 50.0f), 10.f, text, Rgba8::BLACK, 1.0f);
	Texture const& bitmapFontTexture = g_bitmapFontTier2->GetTexture();
	g_theRenderer->BindTexture(0, &bitmapFontTexture);
	g_theRenderer->DrawVertexArray((int) renderVertexes.size(), renderVertexes.data());
}


void Game::BitmapFontTest(float uiCanvasSizeX, float uiCanvasSizeY) const
{
	static float time = (float) GetCurrentTimeSeconds();
	if (!m_isPaused)
	{
		time = (float) GetCurrentTimeSeconds();
		time *= 0.5f;
	}

	float textBoxWidth = uiCanvasSizeX * 0.8f /** abs(SinRadians(time))*/;
	float textBoxHeight = uiCanvasSizeY * 0.5f /** abs(CosRadians(time))*/;
	Vec2 textBoxCenter = Vec2(uiCanvasSizeX * 0.5f, uiCanvasSizeY * 0.5f) /*+ Vec2(50.0f * SinRadians(time), 50.0f * CosRadians(time))*/;
	AABB2 textBox = AABB2(textBoxCenter, textBoxWidth, textBoxHeight);
	std::vector<Vertex_PCU> renderVertexes;
	AddVertsForAABB2ToVector(renderVertexes, textBox, Rgba8::RED);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) renderVertexes.size(), renderVertexes.data());

 	std::vector<Vertex_PCU> textVertexes;
	Vec2 textAlignment = g_gameConfigBlackboard.GetValue("textAlignment", BitmapFont::ALIGNED_CENTER);
	std::string text = g_gameConfigBlackboard.GetValue("text", "");
	float cellHeight = g_gameConfigBlackboard.GetValue("cellHeight", 0.0f);
	float cellAspect = g_gameConfigBlackboard.GetValue("cellAspect", 0.0f);
	cellHeight = 200.0f;
	cellAspect = 1.0f;
	text = "The quick brown fox jumped over the lazy dog";
	//text = "iiiiiiiiiiiiiiiiiiiiii";

	static bool test = true;
	if (test)
	{
		g_console->AddLine(Rgba8::RED, Stringf("AABB2 Mins: %.2f, %.2f | Maxs: %.2f, %.2f", textBox.m_mins.x, textBox.m_mins.y, textBox.m_maxs.x, textBox.m_maxs.y), true);
		test = false;
	}
	g_bitmapFontTier2->AddVertsForTextInAABB2(textVertexes, textBox, cellHeight, text, Rgba8::CYAN, cellAspect, Vec2(0.5f, 0.5f), TextDrawMode::SHRINK_TO_FIT);
	Texture const& bitmapFontTexture2 = g_bitmapFontTier2->GetTexture();
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) textVertexes.size(), textVertexes.data());	
	g_theRenderer->BindTexture(0, &bitmapFontTexture2);
	g_theRenderer->SetModelMatrix(Mat44(), Rgba8::MAGENTA);
	g_theRenderer->DrawVertexArray((int) textVertexes.size(), textVertexes.data());	

// 	textVertexes.clear();
// 	g_bitmapFontTier1->AddVertsForTextInAABB2(textVertexes, textBox, cellHeight, text, Rgba8::BLACK, cellAspect, Vec2(0.5f, 0.5f), TextDrawMode::SHRINK_TO_FIT);
// 	Texture const& bitmapFontTexture1 = g_bitmapFontTier1->GetTexture();
// 	g_theRenderer->BindTexture(0, &bitmapFontTexture1);
// 	g_theRenderer->DrawVertexArray((int) textVertexes.size(), textVertexes.data());
	
}


void Game::GenerateRandomShapes()
{
	m_shapeBounds.m_mins = Vec3(5.0f, 5.0f, 0.0f);
	m_shapeBounds.m_maxs = Vec3(10.0f, -5.0f, 5.0f);
	m_shapes3D.clear();
	
	float cubeSide = 3.0f;
	float radius = 3.0f;

	for (int i = 0; i < m_numShapes; i++)
	{
		int shapeToken = g_rng->GetRandomIntLessThan(3);
		Shape3D shape;
		switch (shapeToken)
		{
			case 0: //cube
			{
				Vec3 cubeCenter = g_rng->GetRandomPositionInAABB3(m_shapeBounds.m_mins, m_shapeBounds.m_maxs);
				shape.m_cube = AABB3(cubeCenter, cubeSide);
				break;
			}
			case 1: // sphere
			{
				shape.m_isSphere = true;
				shape.m_radius = radius;
				break;
			}
			case 2: //zcylinder
			{	
				shape.m_isCylinder = true;
				shape.m_radius = radius;
				Vec2 centerXY = g_rng->GetRandomPositionInAABB2(m_shapeBounds.m_mins.GetVec2(), m_shapeBounds.m_maxs.GetVec2());
				shape.m_zCylinderBottom = Vec3(centerXY.x, centerXY.y, 0.5f);
				shape.m_zCylinderBottom = Vec3(centerXY.x, centerXY.y, 0.5f + radius);
				break;
			}
			default:
				break;
		}
		m_shapes3D.push_back(shape);
	}
}


void Game::RenderAttractModeDebugger() const
{

}


// bool PlaySound(EventArgs& args)
// {
// 	std::string audioFilePath = "Data/Audio/" + args.GetValue("fileName", "");
// 	SoundID soundId = g_audioSystem->CreateOrGetSound(audioFilePath.c_str()/*"Data/Audio/Click.mp3"*/);
// 	g_audioSystem->StartSound(soundId);
// 	return false;
// }
class TestJob : public Job
{

public:
	void Execute() override
	{
		for (int i = 0; i < 10; i++)
		{
			g_console->AddLine(m_color, Stringf("%s: i = %i", m_name.c_str(), i));
		}
	}

	void OnClaimCallback() override
	{
		g_console->AddLine(m_color, Stringf("%s: Job has been claimed", m_name.c_str()));
	}

	Rgba8 m_color;
};


void Game::InitPlayMode()
{
	m_isPaused = false;

	TestJob* jobA = new TestJob;
	jobA->m_name = "Test Job 1";
	jobA->m_color = Rgba8::BLUE;
	TestJob* jobB = new TestJob;
	jobB->m_name = "Test Job 2";
	jobB->m_color = Rgba8::MAGENTA;
	g_jobSystem->AddJobToQueue(jobA);
	g_jobSystem->AddJobToQueue(jobB);
	
	//g_jobSystem->DestroyWorkerThread("WorkerThread#1");
	//g_jobSystem->ClaimAllCompletedJobs();

	m_worldCamera.DefineGameSpace(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));

	m_camSpeed = g_gameConfigBlackboard.GetValue("camSpeed", 10.0f);
	m_camSprintSpeedModifier = g_gameConfigBlackboard.GetValue("sprintModifier", 1.75f);

	//g_eventSystem->SubscribeEventCallbackFunction("PlaySound", PlaySound);

	MouseConfig config = {};
	config.m_isHidden = true;
	config.m_isRelative = true;
	config.m_isLocked = true;
	config.m_priority = 340;
	g_inputSystem->PushMouseConfig(config);

	GenerateRandomShapes();
}


void Game::DeInitPlayMode()
{
	g_inputSystem->PopMouseConfigOfPriority(340);
	//g_eventSystem->UnsubscribeEventCallbackFunction("PlaySound", PlaySound);
}


void Game::CheckInputPlayMode(float deltaSeconds)
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		SetGameMode(GameMode::ATTRACT);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER))
	{
		g_console->ExecuteCommand("PlaySound fileName=Click.mp3");
	}

	CameraData& activeCamData = m_camData[m_activeCamIndex];
	float speed = m_camSpeed;
	Vec3 forward = activeCamData.m_camOrientation.GetFoward_XFwd_YLeft_ZUp();
	Vec3 left = activeCamData.m_camOrientation.GetLeft_XFwd_YLeft_ZUp();
	if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
	{
		speed *= m_camSprintSpeedModifier;
	}

	if (g_inputSystem->IsKeyPressed('W'))
	{
		activeCamData.m_camPosition += forward.GetXYVec3() * speed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('A'))
	{
		activeCamData.m_camPosition += left.GetXYVec3() * speed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('S'))
	{
		activeCamData.m_camPosition -= forward.GetXYVec3() * speed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('D'))
	{
		activeCamData.m_camPosition -= left.GetXYVec3() * speed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('E'))
	{
		activeCamData.m_camPosition += Vec3(0.0f, 0.0f, 1.0f) * speed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('Q'))
	{
		activeCamData.m_camPosition += Vec3(0.0f, 0.0f, -1.0f) * speed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('F'))
	{
		activeCamData.m_camFov += 0.2f * activeCamData.m_camFov * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('C'))
	{
		activeCamData.m_camFov -= 0.2f * activeCamData.m_camFov * deltaSeconds;
	}
	activeCamData.m_camFov = Clamp(activeCamData.m_camFov, 1.0f, 179.0f);

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		activeCamData.m_isFreeAspect = !activeCamData.m_isFreeAspect;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
	{
		GenerateRandomShapes();
	}

	float camDimSpeed = g_gameConfigBlackboard.GetValue("camDimSpeed", 10.0f);
	if (g_inputSystem->WasMouseButtonJustPressed(1))
	{
		m_activeCamIndex++;
		m_activeCamIndex %= NUM_CAMERAS;
	}

	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_activeCamIndex--;
		m_activeCamIndex = m_activeCamIndex < 0 ? NUM_CAMERAS - 1 : m_activeCamIndex;
	}

	if (g_inputSystem->IsKeyPressed('K'))
	{
		activeCamData.m_camWidth += camDimSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('J'))
	{
		activeCamData.m_camWidth -= camDimSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('M'))
	{
		activeCamData.m_camHeight += camDimSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('N'))
	{
		activeCamData.m_camHeight -= camDimSpeed * deltaSeconds;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_UP_ARROW))
	{
		m_numShapes++;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_DOWN_ARROW))
	{
		m_numShapes--;
	}

	activeCamData.m_camHeight = Clamp(activeCamData.m_camHeight, 0.1f, 10'000.0f);
	activeCamData.m_camWidth = Clamp(activeCamData.m_camWidth, 0.1f, 10'000.0f);

	IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
	Vec2 mouseDeltaFloat = mouseDelta.GetVec2();
	static float mouseSensitivity = g_gameConfigBlackboard.GetValue("mouseSensitivity", 0.1f);
	activeCamData.m_camOrientation.m_yaw -= mouseDelta.x * mouseSensitivity;
	activeCamData.m_camOrientation.m_pitch += mouseDelta.y * mouseSensitivity;

	activeCamData.m_camOrientation.m_pitch = Clamp(activeCamData.m_camOrientation.m_pitch, -89.5f, 89.5f);
}


void Game::UpdatePlayMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	static int i = 0;
	if (i == 0)
	{
		g_jobSystem->ClaimAllCompletedJobs();
		i++;
	}

	//DebugAddWorldArrow(Vec3(5.0f, 0.0f, 0.0f), Vec3(5.0f, 0.0f, 3.0f), 0.0f, Rgba8::CYAN);
	CameraData& activeCamData = m_camData[m_activeCamIndex];
	DebugAddScreenText(Stringf("Cam Pos: %.2f, %.2f, %.2f", activeCamData.m_camPosition.x, activeCamData.m_camPosition.y, activeCamData.m_camPosition.z), Vec2(0.0f, 0.0f), 0.0f, Vec2::ZERO, 16.0f);
	DebugAddScreenText(Stringf("Cam Fov: %.2f", activeCamData.m_camFov), Vec2(0.0f, 20.0f), 0.0f, Vec2::ZERO, 16.0f);
	float camAspect = activeCamData.m_isFreeAspect ? activeCamData.m_camWidth / activeCamData.m_camHeight : g_window->GetWindowConfig().m_aspectRatio;
	DebugAddScreenText(Stringf("Cam Width: %.2f | Cam Height: %.2f | Cam Aspect: %.2f", activeCamData.m_camWidth, activeCamData.m_camHeight, camAspect), Vec2(0.0f, 40.0f), 0.0f, Vec2::ZERO, 16.0f);
	DebugAddScreenText(Stringf("Is Free Aspect: %i", activeCamData.m_isFreeAspect), Vec2(0.0f, 60.0f), 0.0f, Vec2::ZERO, 16.0f);
	DebugAddScreenText(Stringf("Camera: %i", m_activeCamIndex), Vec2(0.0f, 80.0f), 0.0f, Vec2::ZERO, 16.0f);

	std::string controlsText = "J/K: Cam Width | N/M: Cam Height | F2: Toggle Free Aspect | F8: Randomize Shapes"; 
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	DebugAddScreenText(controlsText, Vec2(0.0f, uiCanvasSizeY), 0.0f, Vec2(0.0f, 1.0f), 16.0f);
	
	//DebugAddWorldWireAABB3(m_shapeBounds, 0.0f, Rgba8::MAGENTA, DebugRenderMode::XRAY);

	float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	DebugAddScreenText(Stringf("Num Shapes: %i", m_numShapes), Vec2(uiCanvasSizeX, 0.0f), 0.0f, Vec2(1.0f, 0.0f), 16.0f);
}


void Game::DeleteGarbage()
{
	
}


void Game::RenderPlayMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	//World Camera
	{
		g_theRenderer->BeginCamera(m_worldCamera);
		g_theRenderer->SetDepthOptions(DepthTest::LESS, true);
		Texture* defaultDepthTex = g_theRenderer->GetDefaultDepthStencil();
		g_theRenderer->ClearDepth(defaultDepthTex, 1.0f, 0);

		std::vector<Vertex_PCU> shapeVerts;
		for (int i = 0; i < m_shapes3D.size(); i++)
		{
			Shape3D const& shape = m_shapes3D[i];
			if (shape.m_isCylinder)
			{
				AddVertsForCylinderZ3DToVector(shapeVerts, shape.m_zCylinderBottom.GetVec2(), FloatRange(shape.m_zCylinderBottom.z, shape.m_zCylinderTop.z), shape.m_radius, 16.0f, Rgba8::WHITE);
			}
			else if (shape.m_isSphere)
			{
				AddVertsForUVSphereZ3DToVector(shapeVerts, shape.m_sphereCenter, shape.m_radius, 16.0f, 8.0f);
			}
			else
			{
				AddVertsForAABBZ3DToVector(shapeVerts, shape.m_cube);
			}

		}
		Texture* texture = g_theRenderer->CreateOrGetTexture("Data/Images/Test_StbiFlippedAndOpenGL.png");
		g_theRenderer->BindTexture(0, texture);
		g_theRenderer->DrawVertexArray((int) shapeVerts.size(), shapeVerts.data());

		RenderGrid();

		if (m_debugMode)
		{
			RenderPlayModeDebugger();
		}

		g_theRenderer->EndCamera(m_worldCamera);
	}

	//UI Camera
	{
		g_theRenderer->BeginCamera(m_uiCamera);
		RenderPlayModeUI();
		g_theRenderer->EndCamera(m_uiCamera);
	}
}


void Game::RenderPlayModeDebugger() const
{
	
}


void Game::RenderGrid() const
{
	float lineLength = 1000.0f;
	float otherDims = 0.05f;

	std::vector<Vertex_PCU> gridVerts;

	CameraData const& activeCamData = m_camData[m_activeCamIndex];
	Vec3 camPosition = activeCamData.m_camPosition;
	int camGridX = RoundDownToInt(camPosition.x);
	int camGridY = RoundDownToInt(camPosition.y);
	IntVec2 camGridPosition2D = IntVec2(camGridX, camGridY);

	for (int lineIndex = -100; lineIndex < 100; lineIndex++)
	{
		float xAxisGridPos = (float) (camGridX + lineIndex);
		float yAxisGridPos = (float) (camGridY + lineIndex);
		float xLineLength = abs(yAxisGridPos) + lineLength;
		float yLineLength = abs(xAxisGridPos) + lineLength;
		AABB3 lineAlongXAxis = AABB3(Vec3(xAxisGridPos, 0.0f, 0.0f), otherDims, xLineLength, otherDims);
		AABB3 lineAlongYAxis = AABB3(Vec3(0.0f, yAxisGridPos, 0.0f), yLineLength, otherDims, otherDims);

		if (xAxisGridPos == 0)
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongXAxis, Rgba8::WHITE);
		}
		else
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongXAxis, Rgba8::RED);
		}

		if (yAxisGridPos == 0)
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongYAxis, Rgba8::WHITE);
		}
		else
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongYAxis, Rgba8::GREEN);
		}
	}


	g_theRenderer->SetCullMode(CullMode::NONE);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) gridVerts.size(), gridVerts.data());
}


void Game::RenderPlayModeUI() const
{

}


void Game::SetGameMode(GameMode gameMode)
{
	switch (gameMode)
	{
		case GameMode::ATTRACT:
			DeInitAttractMode();
			break;
		case GameMode::PLAY:
			DeInitPlayMode();
			break;
		case GameMode::NONE: // fallthrough
		default:
			break;
	}

	m_gameMode = gameMode;

	switch (gameMode)
	{
		case GameMode::ATTRACT:
			InitAttractMode();
			break;
		case GameMode::PLAY:
			InitPlayMode();
			break;
		case GameMode::NONE: // fallthrough
		default:
			break;
	}
}


void Game::SetWorldCamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration)
{
	m_worldCamScreenShakeMagnitude = screenShakeMagnitude;
	m_worldCamScreenShakeDuration = screenShakeDuration;
	m_shouldShakeWorldCam = true;
	m_worldCamTimeSinceScreenShakeStart = 0.0f;
}


void Game::SetUICamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration)
{
	m_uiCamScreenShakeMagnitude = screenShakeMagnitude;
	m_uiCamScreenShakeDuration = screenShakeDuration;
	m_shouldShakeUICam = true;
	m_uiCamTimeSinceScreenShakeStart = 0.0f;
}


void Game::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}
