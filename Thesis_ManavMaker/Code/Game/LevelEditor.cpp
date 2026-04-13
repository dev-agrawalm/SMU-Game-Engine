#include "Game/LevelEditor.hpp"
#include "Game/Level.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/EditorOperation.hpp"
#include "Game/EditorPaletteWidget.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/UIWidget.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "Engine/Core/NamedProperties.hpp"
#include "Game/EditorInspectorWidget.hpp"
#include <windows.h> 
#include "Game/Game.hpp"

bool LevelEditor::s_saveLevel = false;
bool LevelEditor::s_showLevelSaveWindow = false;
char LevelEditor::s_levelSaveName[MAX_PATH] = {};


unsigned char KEYCODE_WAYPOINT_MODE = 'Q';


void LevelEditor::SetEditorMusicVolume(float newVolume)
{
	g_editorMusicVolume = newVolume;
	g_audioSystem->SetSoundPlaybackVolume(m_bgMusicPlaybackId, g_editorMusicVolume);
}


void LevelEditor::SetLevelMusicVolume(float newVolume)
{
	g_gameMusicVolume = newVolume;
}


bool LevelEditor::OnClicked_SaveButton(NamedProperties& args)
{
	UNUSED(args);
	s_saveLevel = true;
	std::string levelSaveName(s_levelSaveName);
	if (levelSaveName.empty())
	{
		s_showLevelSaveWindow = true;
	}
	return true;
}


LevelEditor::LevelEditor()
{

}


void LevelEditor::Initialize(std::string const& filePath /*= ""*/)
{
	static float editorCamOrthoX = g_gameConfigBlackboard.GetValue("editorCamOrthoX", 0.0f);
	static float editorCamOrthoY = g_gameConfigBlackboard.GetValue("editorCamOrthoY", 0.0f);
	m_editorCamMaxDims = Vec2(editorCamOrthoX, editorCamOrthoY);
	m_editorCamBoundary = AABB2(Vec2::ZERO, m_editorCamMaxDims);
	m_editorCam.SetOrthoView(editorCamOrthoX * 0.25f, editorCamOrthoY * 0.25f, editorCamOrthoX * 0.75f, editorCamOrthoY * 0.75f);

	static float uiOrthoX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float uiOrthoY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCam.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(uiOrthoX, uiOrthoY));

	InitialiseEditorWindows(uiOrthoX, uiOrthoY);
	InitialiseLevel(filePath);
	//AddControlsToConsole();
	InitGridLineVerts();

	m_mouseData.m_tileDefinition = TileDefinition::GetDefinitionByName("Ground");

	//bool muteEditor = g_gameConfigBlackboard.GetValue("muteEditor", false);
	//float bgMusicVolume = muteEditor ? 0.0f : g_gameConfigBlackboard.GetValue("editorBgMusicVolume", 0.65f);
	SoundID editorBgMusicSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/EditorBackgroundMusic.mp3");
	m_bgMusicPlaybackId = g_audioSystem->StartSound(editorBgMusicSoundId, true, g_editorMusicVolume);
}


void LevelEditor::InitialiseEditorWindows(float uiOrthoX, float uiOrthoY)
{
	Vec2 paintbrushWindowMins = Vec2(0.0f, 0.0f);
	Vec2 paintbrushWindowMaxs = Vec2(uiOrthoX * 0.2f, uiOrthoY);
	m_paletteWindow = new EditorPaletteWidget(m_uiCam, paintbrushWindowMins, paintbrushWindowMaxs);
	Rgba8 paintbrushWindoColor = Rgba8::WHITE;
	SpriteSheet* uiSpriteSheet = g_game->CreateOrGetSpriteSheet(IntVec2(32, 8));
	SpriteDefinition const& paletteBgImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(2, 2));
	m_paletteWindow->SetImage(&paletteBgImage);
	m_paletteWindow->SetColor(paintbrushWindoColor);
	bool isPalleteHidden = g_gameConfigBlackboard.GetValue("HidePallete", false);
	m_paletteWindow->SetHidden(isPalleteHidden);

	Vec2 inspectorWindowMins = Vec2(uiOrthoX * 0.75f, 0.0f);
	Vec2 inspectorWindowMaxs = Vec2(uiOrthoX, uiOrthoY);
	m_inspectorWindow = new EditorInspectorWidget(m_uiCam, inspectorWindowMins, inspectorWindowMaxs);
	m_inspectorWindow->SetColor(Rgba8::YELLOW);
	bool isInspectorHidden = g_gameConfigBlackboard.GetValue("HideInspector", true);
	m_inspectorWindow->SetHidden(isInspectorHidden);
}


void LevelEditor::InitialiseLevel(std::string const& filePath)
{
	m_level = new Level();
	if (!filePath.empty())
	{
		m_level->LoadLevelFromPath(filePath);
		bool useWindowDialog = g_gameConfigBlackboard.GetValue("useWindowDialog", false);
		if (useWindowDialog)
		{
			for (int i = 0; i < filePath.size(); i++)
			{
				s_levelSaveName[i] = filePath[i];
			}
		}
		else
		{
			Strings filePathComponents = SplitStringOnDelimiter(filePath, '\\');
			std::string levelNameWithExtension = filePathComponents.back();
			for (int i = 0; i < levelNameWithExtension.size(); i++)
			{
				s_levelSaveName[i] = levelNameWithExtension[i];
			}
		}
	}
	else
	{
		s_levelSaveName[0] = 0;
	}
	m_level->Init_EditorMode();
	g_eventSystem->SubscribeEventCallbackObjectMethod("GameOverEvent", *this, &LevelEditor::OnGameOverEvent);
}


void LevelEditor::AddControlsToConsole()
{
	g_console->AddLine(Rgba8::GREEN, "Left mouse button on empty tiles: Paint tiles/ place entities in the level", true);
	g_console->AddLine(Rgba8::GREEN, "Left mouse button on painted tiles/entities: Grab and move tile/entities", true);
	g_console->AddLine(Rgba8::GREEN, "Shift (Hold) + left mouse button: Multi-select tiles/entities", true);
	g_console->AddLine(Rgba8::GREEN, "Alt (Hold) + left mouse button: Duplicate selected tiles/entities", true);
	g_console->AddLine(Rgba8::GREEN, "Ctrl (Hold) + left mouse button:	Erase tiles/entities", true);
	g_console->AddLine(Rgba8::GREEN, "Select tiles/entities + Delete:	Delete selection", true);
	g_console->AddLine(Rgba8::GREEN, "In Editor: W (Hold) + mouse click:	Paint tiles in a box", true);
	g_console->AddLine(Rgba8::GREEN, "In Editor: E (Hold) + mouse click:	Paint tiles in a line", true);
	g_console->AddLine(Rgba8::GREEN, "Ctrl + Z:	Undo", true);
	g_console->AddLine(Rgba8::GREEN, "Ctrl + Y:	Redo (up to last 10 actions)", true);
	g_console->AddLine(Rgba8::GREEN, "Right mouse button: Pan the level", true);
	g_console->AddLine(Rgba8::GREEN, "Mouse wheel: Zoom in and out", true);
	g_console->AddLine(Rgba8::GREEN, "Enter: Switch between editing and playing", true);
	g_console->AddLine(Rgba8::GREEN, "F1: Toggle entity debugging (will show cosmetic and physics AABB2 of the entity)", true);
	g_console->AddLine(Rgba8::GREEN, "F2: Erase all tiles and entities from the level", true);
	g_console->AddLine(Rgba8::GREEN, "F3: Erase all entities from the level", true);
	g_console->AddLine(Rgba8::GREEN, "F4: Erase all tile data from the level", true);
	g_console->AddLine(Rgba8::GREEN, "F6: Hide palette window", true);
	g_console->AddLine(Rgba8::GREEN, "F7: Hide inspector window", true);
	g_console->AddLine(Rgba8::GREEN, "Esc: Go back to main menu", true);
	g_console->AddLine(Rgba8::GREEN, "In play mode: A/D to move Mario, W/Spacebar to jump", true);
	//DebugAddScreenText("Open dev console (~) to look at controls", Vec2(1800.0f, 0.0f), -1.0f, Vec2(1.0f, 0.0f), 16.0f);
}


void LevelEditor::Update()
{
	CheckInput();
	if (!m_inPlayMode)
	{
		switch (m_editingMode)
		{
		case LevelEditor::EDITING_MODE_INSPECT:
		{
			//m_inspectorWindow->Update();
			break;
		}
		case LevelEditor::EDITING_MODE_WAYPOINTS:
			break;
		default:
			HighlightMouseSelection();
			break;
		}

		if (s_showLevelSaveWindow)
		{
			bool useWindowDialog = g_gameConfigBlackboard.GetValue("useWindowDialog", false);
			if (useWindowDialog)
			{
				static char directory[1024] = "";
				GetCurrentDirectoryA(1024, directory);
				OPENFILENAMEA openFileDialogOptions = {};

				openFileDialogOptions.lStructSize = sizeof(OPENFILENAMEA);
				openFileDialogOptions.hwndOwner = (HWND) g_window->GetWindowHandle();
				openFileDialogOptions.lpstrFilter = "XML Files\0*.XML\0\0";
				openFileDialogOptions.lpstrFile = s_levelSaveName;
				openFileDialogOptions.nMaxFile = MAX_PATH;
				openFileDialogOptions.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
				openFileDialogOptions.lpstrDefExt = "xml";
				openFileDialogOptions.lpstrTitle = "Save Level";

				if (GetSaveFileNameA(&openFileDialogOptions))
				{
					std::string levelSaveName = s_levelSaveName;
					if (!levelSaveName.empty())
					{
						SaveLevel();
					}
					SetCurrentDirectoryA(directory);
				}

				s_showLevelSaveWindow = false;
				s_saveLevel = false;
			}
			else
			{
				ImGui::Begin("Save Level Window");
				ImGui::InputText("Level Name", s_levelSaveName, MAX_PATH);
				if (ImGui::Button("Save Level"))
				{
					SaveLevel();
				}
				ImGui::End();
			}
		}
		else if (s_saveLevel)
		{
			SaveLevel();
		}
	}

	if (m_inPlayMode)
	{
		m_level->Update();
	}
	//PrintDebugMessages();
}


void LevelEditor::PrintDebugMessages()
{
	static float consoleCameraOrthoX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float consoleCameraOrthoY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	if (m_debugRaycast)
	{
		DebugAddScreenText("Debugging Raycast", Vec2::ZERO, 0.0f, Vec2::ZERO, 12.0f);
	}

	if (m_inPlayMode)
	{
		DebugAddScreenText("Play Mode", Vec2(consoleCameraOrthoX, consoleCameraOrthoY), 0.0f, Vec2(1.0f, 1.0f), 16.0f);
	}
	else
	{
		DebugAddScreenText("Editor Mode", Vec2(consoleCameraOrthoX, consoleCameraOrthoY), 0.0f, Vec2(1.0f, 1.0f), 16.0f);
	}
}


void LevelEditor::HandleInputForCamera()
{
	Vec2 windowDims = g_window->GetWindowDims();
	AABB2 editorCamOrthoBox = m_editorCam.GetOrthoCamBoundingBox();
	IntVec2 mousePos = g_inputSystem->GetMousePosition();

	static Vec2 s_worldAnchorPoint;
	if (g_inputSystem->WasMouseButtonJustPressed(1))
	{
		s_worldAnchorPoint = m_mouseData.m_currentWorldPos;
	}

	if (g_inputSystem->IsMouseButtonPressed(1))
	{
		Vec2 orthoDims = editorCamOrthoBox.GetDimensions();
		Vec2 newEditorOrthoMins;
		newEditorOrthoMins.x = s_worldAnchorPoint.x - (orthoDims.x * mousePos.GetVec2().x) / windowDims.x;
		newEditorOrthoMins.y = s_worldAnchorPoint.y - orthoDims.y + (orthoDims.y * mousePos.GetVec2().y) / windowDims.y;
		Vec2 newEditorOrthoMaxs = newEditorOrthoMins + orthoDims;
		editorCamOrthoBox = AABB2(newEditorOrthoMins, newEditorOrthoMaxs);
	}

	//camera zoom in/out
	if (m_mouseData.m_mouseWheel != 0)
	{
		static float zoomSpeed = g_gameConfigBlackboard.GetValue("editorCamZoomSpeed", 1.1f);
		static float zoomSpeedInverse = 1.0f / zoomSpeed;
		Vec2 cameraDims = editorCamOrthoBox.GetDimensions();
		if (m_mouseData.m_mouseWheel > 0)
		{
			cameraDims *= zoomSpeedInverse;
		}
		else
		{
			cameraDims *= zoomSpeed;
		}
		//cameraDims = Clamp(cameraDims, Vec2::ZERO, m_editorCamMaxDims);
		editorCamOrthoBox.SetDimensions(cameraDims);
	}

	ClampAndUpdateEditorCamera(editorCamOrthoBox);
}


void LevelEditor::RenderGridLines() const
{
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) m_gridLineVerts.size(), m_gridLineVerts.data());
}


void LevelEditor::PopulateMouseSelectionData()
{
	EmptyMouseSelectionData();

	AABB2 selectionBounds = m_mouseData.GetSelectionBounds();
	int startX = RoundToNearestInt(selectionBounds.m_mins.x);
	int startY = RoundToNearestInt(selectionBounds.m_mins.y);
	int endX = RoundToNearestInt(selectionBounds.m_maxs.x);
	int endY = RoundToNearestInt(selectionBounds.m_maxs.y);

	for (int x = startX; x <= endX; x++)
	{
		for (int y = startY; y < endY; y++)
		{
			IntVec2 tileCoords(x, y);
			EntitySpawnInfo spawnInfo = m_level->GetEntitySpawnInfo(tileCoords);
			Tile tile = m_level->GetTile(tileCoords);
			if (!spawnInfo.IsInvalid())
			{
				m_mouseData.AddEntitySpawnToSelection(spawnInfo);
			}
			else if(!tile.IsEmpty())
			{
				TileSelection tileSelection(tile.m_definition, tileCoords);
				m_mouseData.AddTileSelectionToSelection(tileSelection);
			}
		}
	}
}


void LevelEditor::EmptyMouseSelectionData()
{
	MouseSelectionData selectionData = m_mouseData.m_selectionData;
	std::vector<TileSelection>& tileSelection = selectionData.m_tileSelections;
	std::vector<EntitySpawnInfo>& entitySelection = selectionData.m_entitySpawnSelections;

	for (int i = 0; i < tileSelection.size(); i++)
	{
		m_mouseData.RemoveTileFromSelection(tileSelection[i].m_tileCoords);
	}

	for (int i = 0; i < entitySelection.size(); i++)
	{
		m_mouseData.RemoveEntitySpawnFromSelection(entitySelection[i]);
	}
}


void LevelEditor::RemoveSelectionDataFromLevel(EditorCommand* command /*= nullptr*/)
{
	UNUSED(command);
	MouseSelectionData const& selectionData = m_mouseData.m_selectionData;
	std::vector<TileSelection> const& tileSelection = selectionData.m_tileSelections;
	std::vector<EntitySpawnInfo> const& entitySelection = selectionData.m_entitySpawnSelections;

	for (int i = 0; i < (int) tileSelection.size(); i++)
	{
		m_level->SetTileEmpty(tileSelection[i].m_tileCoords);
		m_level->MarkTileAsDeselected(tileSelection[i].m_tileCoords);
	}

	for (int i = 0; i < (int) entitySelection.size(); i++)
	{
		EntitySpawnInfo const& spawnInfo = entitySelection[i];
		m_level->MarkEntitySpawnAsDeselected(spawnInfo);
		m_level->RemoveEntitySpawn(spawnInfo);
	}
}


bool LevelEditor::IsMouseOverMouseSelection() const
{
	IntVec2 mouseTileCoords = m_mouseData.m_currentTileCoords;
	Vec2 mousePos = m_mouseData.m_currentWorldPos;
	MouseSelectionData const& selectionData = m_mouseData.m_selectionData;
	std::vector<TileSelection> const& tileSelection = selectionData.m_tileSelections;
	std::vector<EntitySpawnInfo> const& entitySelection = selectionData.m_entitySpawnSelections;

	for (int i = 0; i < (int) tileSelection.size(); i++)
	{
		if (mouseTileCoords == tileSelection[i].m_tileCoords)
			return true;
	}

	for (int i = 0; i < (int) entitySelection.size(); i++)
	{
		EntitySpawnInfo const& spawnInfo = entitySelection[i];
		AABB2 entityBounds = EntityDefinition::GetPhysicsBoundsForEntity(spawnInfo.m_type, spawnInfo.m_startingPosition);
		if (IsPointInsideAABB2D(mousePos, entityBounds))
			return true;
	}

	return false;
}


void LevelEditor::HighlightMouseSelection()
{
	m_level->MarkEverythingAsDeselected();
	MouseSelectionData const& selectionData = m_mouseData.m_selectionData;
	std::vector<TileSelection> const& tileSelectionData = selectionData.m_tileSelections;
	std::vector<EntitySpawnInfo> const& entitySpawnSelectionData = selectionData.m_entitySpawnSelections;

	for (int i = 0; i < (int) tileSelectionData.size(); i++)
	{
		m_level->MarkTileAsSelected(tileSelectionData[i].m_tileCoords);
	}

	for (int i = 0; i < (int) entitySpawnSelectionData.size(); i++)
	{
		m_level->MarkEntitySpawnAsSelected(entitySpawnSelectionData[i]);
	}
}


bool LevelEditor::OnGameOverEvent(EventArgs& args)
{
	UNUSED(args);
	SwapBetweenEditorAndPlayMode();
	return false;
}


void LevelEditor::FillMouseSelection()
{
	EditorCommand* fillCommand = new EditorCommand(this);

	AABB2 mouseSelectionBounds = m_mouseData.GetSelectionBounds();
	int minX = RoundToNearestInt(mouseSelectionBounds.m_mins.x);
	int minY = RoundToNearestInt(mouseSelectionBounds.m_mins.y);
	int maxX = RoundToNearestInt(mouseSelectionBounds.m_maxs.x);
	int maxY = RoundToNearestInt(mouseSelectionBounds.m_maxs.y);

	for (int x = minX; x < maxX; x++)
	{
		for (int y = minY; y < maxY; y++)
		{
			EditorOperation* paintOp = nullptr;
			IntVec2 tileCoords = IntVec2(x, y);
			paintOp = PaintTile(tileCoords, m_mouseData.m_tileDefinition);
			fillCommand->AddOperation(paintOp);
		}
	}

	AddCommandToCommandHistory(fillCommand);
}


void LevelEditor::FillMouseLine()
{
	EditorCommand* fillCommand = new EditorCommand(this);
	PaintTilesBetweenPositions(m_mouseData.m_dragStartPos, m_mouseData.m_currentWorldPos, fillCommand);
	AddCommandToCommandHistory(fillCommand);
}


void LevelEditor::HandleMouseInput_Inspect()
{
	if (g_inputSystem->IsKeyPressed('W') && g_inputSystem->WasMouseButtonJustPressed(0))
	{
		//m_mouseData.m_tool = MouseTool::FILL_TOOL;
		SetEditingMode(EDITING_MODE_PAINT_BOX);
		return;
	}

	if (g_inputSystem->IsKeyPressed('E') && g_inputSystem->WasMouseButtonJustPressed(0))
	{
		SetEditingMode(EDITING_MODE_PAINT_LINE);
		//m_mouseData.m_tool = MouseTool::LINE_TOOL;
		return;
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_WAYPOINT_MODE))
	{
		SetEditingMode(EDITING_MODE_WAYPOINTS);
		return;
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_CTRL))
	{
		//m_mouseData.m_tool = MouseTool::ERASE_TOOL;
		SetEditingMode(EDITING_MODE_ERASE);
	}
	else if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
	{
		SetEditingMode(EDITING_MODE_MULTI_SELECT);
		//m_mouseData.m_tool = MouseTool::MULTI_SELECT_TOOL;
	}
	else if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		Tile tile = m_level->GetTile(m_mouseData.m_currentTileCoords);
		EntitySpawnInfo spawnInfo = m_level->GetEntitySpawnInfo(m_mouseData.m_currentWorldPos);

		if (IsMouseOverMouseSelection())
		{
			if (g_inputSystem->IsKeyPressed(KEYCODE_ALT))
			{
				//m_mouseData.m_tool = MouseTool::DUPLICATE_TOOl;
				SetEditingMode(EDITING_MODE_DUPLICATE);
			}
			else
			{
				SetEditingMode(EDITING_MODE_DRAG);
				//m_mouseData.m_tool = MouseTool::DRAG_TOOL;
			}
		}
		else if (!spawnInfo.IsInvalid())
		{
			EmptyMouseSelectionData();
			m_mouseData.AddEntitySpawnToSelection(spawnInfo);
			if (g_inputSystem->IsKeyPressed(KEYCODE_ALT))
			{
				SetEditingMode(EDITING_MODE_DUPLICATE);
				//m_mouseData.m_tool = MouseTool::DUPLICATE_TOOl;
			}
			else
			{
				//m_mouseData.m_tool = MouseTool::DRAG_TOOL;
				SetEditingMode(EDITING_MODE_DRAG);
			}
		}
		else if (tile.m_definition && !tile.IsEmpty())
		{
			EmptyMouseSelectionData();
			TileSelection tileSelection(tile.m_definition, m_mouseData.m_currentTileCoords);
			m_mouseData.AddTileSelectionToSelection(tileSelection);
			if (g_inputSystem->IsKeyPressed(KEYCODE_ALT))
			{
				//m_mouseData.m_tool = MouseTool::DUPLICATE_TOOl;
				SetEditingMode(EDITING_MODE_DUPLICATE);
			}
			else
			{
				SetEditingMode(EDITING_MODE_DRAG);
				//m_mouseData.m_tool = MouseTool::DRAG_TOOL;
			}
		}
		else
		{
			EmptyMouseSelectionData();
			//m_mouseData.m_tool = MouseTool::PAINTING_TOOL;
			SetEditingMode(EDITING_MODE_PAINT_SINGLE);
		}
	}
}


void LevelEditor::HandleMouseInput_Drag()
{
	static MouseSelectionData s_startingSelectionData;
	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_mouseData.m_dragStartPos = m_mouseData.m_currentWorldPos;
		m_currentCommand = new EditorCommand(this);
		s_startingSelectionData = m_mouseData.m_selectionData;
		RemoveSelectionDataFromLevel();
	}

	if (g_inputSystem->IsMouseButtonPressed(0))
	{
		Vec2 dragPosOffset = m_mouseData.m_currentWorldPos - m_mouseData.m_dragStartPos;
		IntVec2 dragCoordsOffset = dragPosOffset.GetIntVec2();

		MouseSelectionData& selectionData = m_mouseData.m_selectionData;
		std::vector<TileSelection>& tileSelection = selectionData.m_tileSelections;
		std::vector<EntitySpawnInfo>& entitySelection = selectionData.m_entitySpawnSelections;

		for (int i = 0; i < (int) tileSelection.size(); i++)
		{
			TileSelection& startTileSelection = s_startingSelectionData.m_tileSelections[i];
			IntVec2 newTileCoords = startTileSelection.m_tileCoords.GetVec2().GetIntVec2() + dragCoordsOffset;
			
			TileSelection& selection = tileSelection[i];
			selection.m_tileCoords = newTileCoords;
		}

		for (int i = 0; i < (int) entitySelection.size(); i++)
		{
			EntitySpawnInfo& startSpawnInfo = s_startingSelectionData.m_entitySpawnSelections[i];
			Vec2 spawnInfoStartPos = startSpawnInfo.m_startingPosition;
			IntVec2 spawnInfoStartTileCoord = m_level->GetTileCoords(spawnInfoStartPos);
			IntVec2 newTileCoord = spawnInfoStartTileCoord + dragCoordsOffset;
			Vec2 newPos = m_level->GetTileBottomCenter(newTileCoord);
			
			EntitySpawnInfo& spawnInfo = entitySelection[i];
			spawnInfo.m_startingPosition = newPos;
		}
	}

	if (g_inputSystem->WasMouseButtonJustReleased(0) && m_currentCommand)
	{
		MouseSelectionData const& newSelectionData = m_mouseData.m_selectionData;
		EditorOperation* operation = new SelectionDragOperation(s_startingSelectionData, newSelectionData);
		operation->Execute(this);
		m_currentCommand->AddOperation(operation);
// 		std::vector<TileSelection> const& tileSelection = newSelectionData.m_tileSelections;
// 		std::vector<EntitySpawnInfo> const& entitySelection = newSelectionData.m_entitySpawnSelections;

// 		for (int i = 0; i < (int) tileSelection.size(); i++)
// 		{
// 			EditorOperation* operation = nullptr;
// 			TileSelection const& startTileSelection = s_startingSelectionData.m_tileSelections[i];
// 			TileSelection const& selection = tileSelection[i];
// 			TileDefinition const* overwrittenTileDef = m_level->GetTile(selection.m_tileCoords).m_definition;
// 			operation = new TileDragOperation(startTileSelection.m_tileCoords, selection.m_tileCoords, selection.m_tileDef, overwrittenTileDef);
// 
// 			if (operation->IsValid(m_level))
// 			{
// 				operation->Execute(this);
// 				m_currentCommand->AddOperation(operation);
// 			}
// 		}
// 
// 		for (int i = 0; i < (int) entitySelection.size(); i++)
// 		{
// 			EditorOperation* operation = nullptr;
// 			EntitySpawnInfo const& startSpawnInfo = s_startingSelectionData.m_entitySpawnSelections[i];
// 			EntitySpawnInfo const& spawnInfo = entitySelection[i];
// 			std::vector<EntitySpawnInfo> overlappingEntitySpawns;
// 			m_level->AddOverlappingSpawnInfosToVector(overlappingEntitySpawns, spawnInfo);
// 			operation = new EntityDragOperation(startSpawnInfo, spawnInfo, overlappingEntitySpawns);
// 
// 			if (operation->IsValid(m_level))
// 			{
// 				operation->Execute(this);
// 				m_currentCommand->AddOperation(operation);
// 			}
// 		}

		AddCommandToCommandHistory(m_currentCommand);
		m_currentCommand = nullptr;
		SetEditingMode(EDITING_MODE_INSPECT);
		//m_mouseData.m_tool = MouseTool::INSPECT_TOOL;
	}
}


void LevelEditor::HandleMouseInput_PaintSingle()
{
	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_currentCommand = new EditorCommand(this);
	}

	if (g_inputSystem->IsMouseButtonPressed(0) && m_currentCommand)
	{
		PaintTilesBetweenPositions(m_mouseData.m_previousWorldPos, m_mouseData.m_currentWorldPos, m_currentCommand);
	}

	if (g_inputSystem->WasMouseButtonJustReleased(0) && m_currentCommand)
	{
		AddCommandToCommandHistory(m_currentCommand);
		m_currentCommand = nullptr;
		SetEditingMode(EDITING_MODE_INSPECT);
		//m_mouseData.m_tool = MouseTool::INSPECT_TOOL;
	}
}


void LevelEditor::HandleMouseInput_Erase()
{
	if (g_inputSystem->IsKeyUp(KEYCODE_CTRL))
	{
		SetEditingMode(EDITING_MODE_INSPECT);
		//m_mouseData.m_tool = MouseTool::INSPECT_TOOL;
		if (m_currentCommand)
		{
			AddCommandToCommandHistory(m_currentCommand);
			m_currentCommand = nullptr;
		}
		return;
	}

	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_currentCommand = new EditorCommand(this);
	}

	if (g_inputSystem->IsMouseButtonPressed(0) && m_currentCommand)
	{
		EraseTilesBetweenPositions(m_mouseData.m_previousWorldPos, m_mouseData.m_currentWorldPos, m_currentCommand);
	}

	if (g_inputSystem->WasMouseButtonJustReleased(0) && m_currentCommand)
	{
		AddCommandToCommandHistory(m_currentCommand);
		m_currentCommand = nullptr;
	}
}


void LevelEditor::HandleMouseInput_MultiSelect()
{
	if (g_inputSystem->IsKeyUp(KEYCODE_SHIFT))
	{
		SetEditingMode(EDITING_MODE_INSPECT);
		//m_mouseData.m_tool = MouseTool::INSPECT_TOOL;
		return;
	}

	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_mouseData.m_dragStartPos = m_mouseData.m_currentWorldPos;
	}

	if (g_inputSystem->IsMouseButtonPressed(0))
	{
		AABB2 selectionBounds = GetSelectionBounds(m_mouseData.m_dragStartPos, m_mouseData.m_currentWorldPos);
		m_mouseData.SetSelectionBounds(selectionBounds);
		PopulateMouseSelectionData();
	}

	if (g_inputSystem->IsMouseButtonUp(0))
	{
		m_mouseData.SetSelectionBounds(AABB2());
	}
}


void LevelEditor::HandleMouseInput_Duplicate()
{
	if (g_inputSystem->IsKeyUp(KEYCODE_ALT))
	{
		if (m_currentCommand)
		{
			MouseSelectionData& selectionData = m_mouseData.m_selectionData;
			std::vector<TileSelection>& tileSelection = selectionData.m_tileSelections;
			std::vector<EntitySpawnInfo>& entitySelection = selectionData.m_entitySpawnSelections;

			for (int i = 0; i < (int) tileSelection.size(); i++)
			{
				EditorOperation* operation = nullptr;
				TileSelection& selection = tileSelection[i];
				TileDefinition const* overwrittenTileDef = m_level->GetTile(selection.m_tileCoords).m_definition;
				operation = new TilePaintingOperation(selection.m_tileCoords, selection.m_tileDef, overwrittenTileDef);

				if (operation->IsValid(m_level))
				{
					operation->Execute(this);
					m_currentCommand->AddOperation(operation);
				}
			}

			for (int i = 0; i < (int) entitySelection.size(); i++)
			{
				EditorOperation* operation = nullptr;
				EntitySpawnInfo& spawnInfo = entitySelection[i];
				std::vector<EntitySpawnInfo> overlappingEntitySpawns;
				m_level->AddOverlappingSpawnInfosToVector(overlappingEntitySpawns, spawnInfo);
				operation = new EntityPaintingOperation(spawnInfo);

				if (operation->IsValid(m_level))
				{
					operation->Execute(this);
					m_currentCommand->AddOperation(operation);
				}
			}

			AddCommandToCommandHistory(m_currentCommand);
			m_currentCommand = nullptr;
			SetEditingMode(EDITING_MODE_INSPECT);
			//m_mouseData.m_tool = MouseTool::INSPECT_TOOL;
		}
		return;
	}

	static MouseSelectionData s_startingSelectionData;
	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_mouseData.m_dragStartPos = m_mouseData.m_currentWorldPos;
		m_currentCommand = new EditorCommand(this);
		s_startingSelectionData = m_mouseData.m_selectionData;
	}

	if (g_inputSystem->IsMouseButtonPressed(0))
	{
		Vec2 dragPosOffset = m_mouseData.m_currentWorldPos - m_mouseData.m_dragStartPos;
		IntVec2 dragCoordsOffset = dragPosOffset.GetIntVec2();

		MouseSelectionData& selectionData = m_mouseData.m_selectionData;
		std::vector<TileSelection>& tileSelection = selectionData.m_tileSelections;
		std::vector<EntitySpawnInfo>& entitySelection = selectionData.m_entitySpawnSelections;

		for (int i = 0; i < (int) tileSelection.size(); i++)
		{
			TileSelection& startTileSelection = s_startingSelectionData.m_tileSelections[i];
			IntVec2 newTileCoords = startTileSelection.m_tileCoords.GetVec2().GetIntVec2() + dragCoordsOffset;

			TileSelection& selection = tileSelection[i];
			selection.m_tileCoords = newTileCoords;
		}

		for (int i = 0; i < (int) entitySelection.size(); i++)
		{
			EntitySpawnInfo& startSpawnInfo = s_startingSelectionData.m_entitySpawnSelections[i];
			Vec2 spawnInfoStartPos = startSpawnInfo.m_startingPosition;
			IntVec2 spawnInfoStartTileCoord = m_level->GetTileCoords(spawnInfoStartPos);
			IntVec2 newTileCoord = spawnInfoStartTileCoord + dragCoordsOffset;
			Vec2 newPos = m_level->GetTileBottomCenter(newTileCoord);

			EntitySpawnInfo& spawnInfo = entitySelection[i];
			spawnInfo.m_startingPosition = newPos;
		}
	}

	if (g_inputSystem->WasMouseButtonJustReleased(0) && m_currentCommand)
	{
		MouseSelectionData& selectionData = m_mouseData.m_selectionData;
		std::vector<TileSelection>& tileSelection = selectionData.m_tileSelections;
		std::vector<EntitySpawnInfo>& entitySelection = selectionData.m_entitySpawnSelections;

		for (int i = 0; i < (int) tileSelection.size(); i++)
		{
			EditorOperation* operation = nullptr;
			TileSelection& selection = tileSelection[i];
			TileDefinition const* overwrittenTileDef = m_level->GetTile(selection.m_tileCoords).m_definition;
			operation = new TilePaintingOperation(selection.m_tileCoords, selection.m_tileDef, overwrittenTileDef);

			if (operation->IsValid(m_level))
			{
				operation->Execute(this);
				m_currentCommand->AddOperation(operation);
			}
		}

		for (int i = 0; i < (int) entitySelection.size(); i++)
		{
			EditorOperation* operation = nullptr;
			EntitySpawnInfo& spawnInfo = entitySelection[i];
			std::vector<EntitySpawnInfo> overlappingEntitySpawns;
			m_level->AddOverlappingSpawnInfosToVector(overlappingEntitySpawns, spawnInfo);
			operation = new EntityPaintingOperation(spawnInfo);

			if (operation->IsValid(m_level))
			{
				operation->Execute(this);
				m_currentCommand->AddOperation(operation);
			}
		}

		AddCommandToCommandHistory(m_currentCommand);
		m_currentCommand = nullptr;
		SetEditingMode(EDITING_MODE_INSPECT);
		//m_mouseData.m_tool = MouseTool::INSPECT_TOOL;
	}
}


void LevelEditor::HandleMouseInput_Waypoints()
{
	if (g_inputSystem->IsKeyUp(KEYCODE_WAYPOINT_MODE))
	{
		SetEditingMode(EDITING_MODE_INSPECT);
		m_currentCommand = nullptr;
		return;
	}

	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_mouseData.m_spawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;
		m_mouseData.m_dragStartPos = m_mouseData.m_currentWorldPos;
		m_mouseData.m_renderMouse = true;

		EntitySpawnInfo spawnInfo = m_level->GetEntitySpawnInfo(m_mouseData.m_dragStartPos);
		if (spawnInfo.IsValid())
		{
			m_mouseData.m_spawnInfo = spawnInfo;
		}
	}

	if (g_inputSystem->WasMouseButtonJustReleased(0))
	{
		if (m_mouseData.m_spawnInfo.IsValid())
		{
			EntitySpawnInfo waypointSpawnInfo = m_level->GetEntitySpawnInfo(m_mouseData.m_currentWorldPos);
			m_currentCommand = new EditorCommand(this);
			WaypointAssignmentOperation* m_waypointAssignmentOp = new WaypointAssignmentOperation(m_mouseData.m_spawnInfo, waypointSpawnInfo.m_entityId);
			if (m_waypointAssignmentOp->IsValid(m_level))
			{
				m_waypointAssignmentOp->Execute(this);
				m_currentCommand->AddOperation(m_waypointAssignmentOp);
				AddCommandToCommandHistory(m_currentCommand);
				m_currentCommand = nullptr;
			}
			else
			{
				delete m_currentCommand;
				m_currentCommand = nullptr;
			}
		}
	}

	if (g_inputSystem->IsMouseButtonUp(0))
	{
		m_mouseData.m_spawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;
		m_mouseData.m_renderMouse = false;
	}
}


AABB2 LevelEditor::GetSelectionBounds(Vec2 const& startPos, Vec2 const& endPos)
{
	float startX = startPos.x;
	float startY = startPos.y;
	float endX = endPos.x;
	float endY = endPos.y;

	if (endX >= startX && endY < startY)
	{
		return AABB2(startX, endY, endX, startY);
	}

	if (endX < startX && endY < startY)
	{
		return AABB2(endPos, startPos);
	}

	if (endX >= startX && endY >= startY)
	{
		return AABB2(startPos, endPos);
	}

	if (endX < startX && endY >= startY)
	{
		return AABB2(endX, startY, startX, endY);
	}

	return AABB2();
}


void LevelEditor::Render() const
{
	if (!m_inPlayMode)
	{
		g_theRenderer->BeginCamera(m_editorCam);
		{
			switch (m_editingMode)
			{
			case LevelEditor::EDITING_MODE_WAYPOINTS:
			{
				//m_level->Render(!m_inPlayMode);
				m_level->RenderBackground();
				m_level->RenderTiles();

				RenderMaterial material = {};
				material.m_color = Rgba8(0, 0, 0, 100);
				g_theRenderer->DrawAABB2D(m_editorCam.GetOrthoCamBoundingBox(), material);
				
				SpawnData const& levelSpawnData = m_level->GetLevelSpawnData();
				for (int spawnIndex = 0; spawnIndex < (int) levelSpawnData.size(); spawnIndex++)
				{
					EntitySpawnInfo const& spawnInfo = levelSpawnData[spawnIndex];
					if (m_mouseData.m_spawnInfo.IsValid())
					{
						if (spawnInfo == m_mouseData.m_spawnInfo)
						{
							spawnInfo.Render(Rgba8::GREY);
						}
						else if (spawnInfo.m_type == ENTITY_TYPE_WAYPOINT)
						{
							spawnInfo.Render(Rgba8::GREEN);
						}
						else
						{
							spawnInfo.Render(Rgba8::RED);
						}
					}
					else
					{
						spawnInfo.Render(Rgba8::GREEN);
					}
				}

				m_level->RenderWaypointArrows(Rgba8::WHITE);

				if (m_mouseData.m_renderMouse)
				{
					DebugDrawLine(m_mouseData.m_dragStartPos, m_mouseData.m_currentWorldPos - m_mouseData.m_dragStartPos, 0.1f, Rgba8::YELLOW);
				}
				break;
			}
			default:
			{
				m_level->Render(!m_inPlayMode);
				RenderGridLines();
				if (!m_paletteWindow->IsMouseOverWidget() || m_paletteWindow->IsHidden())
				{
					RenderMouse();
				}
				break;
			}
			}

			if (m_debugRaycast)
			{
				DebugDrawRaycast(m_debugRaycastResult);
			}
		}
		g_theRenderer->EndCamera(m_editorCam);

		g_theRenderer->BeginCamera(m_uiCam);
		{
			if (!m_paletteWindow->IsHidden())
			{
				m_paletteWindow->Render();
			}

			if (!m_inspectorWindow->IsHidden())
			{
				m_inspectorWindow->Render();
			}
		}
		g_theRenderer->EndCamera(m_uiCam);
	}
	else
	{
		m_level->Render(!m_inPlayMode);
// 		if (g_gameConfigBlackboard.GetValue("renderGrid", false))
// 		{
// 			g_theRenderer->BeginCamera(m_editorCam);
// 			RenderGridLines();
// 			g_theRenderer->EndCamera(m_uiCam);
// 		}
	}
}


void LevelEditor::RenderMouse() const
{
	static std::vector<Vertex_PCU> mouseSpriteVerts;
	mouseSpriteVerts.clear();
	static Rgba8 mouseSpriteTint = Rgba8(255, 255, 255, 100);
	//if (m_mouseData.m_tool == MouseTool::ERASE_TOOL)
	if (m_editingMode == EDITING_MODE_ERASE)
	{
		RenderMaterial material = {};
		material.m_color = Rgba8(100, 100, 100, 100);
		g_theRenderer->DrawAABB2D(AABB2(m_mouseData.m_currentTileCoords.GetVec2() + Vec2(0.5f, 0.5f), 1.0f, 1.0f), material);
	}

	//if (m_mouseData.m_tool == MouseTool::DRAG_TOOL || m_mouseData.m_tool == MouseTool::DUPLICATE_TOOl)
	if (m_editingMode == EDITING_MODE_DRAG || m_editingMode == EDITING_MODE_DUPLICATE)
	{
		MouseSelectionData const& selectionData = m_mouseData.m_selectionData;
		std::vector<TileSelection> const& tileSelections = selectionData.m_tileSelections;
		std::vector<EntitySpawnInfo> const& entitySelections = selectionData.m_entitySpawnSelections;

		for (int i = 0; i < (int) tileSelections.size(); i++)
		{
			TileSelection const& tileSelection = tileSelections[i];
			AABB2 tileSpriteBounds;
			Vec2 uvMins;
			Vec2 uvMaxs;
			if (tileSelection.m_tileDef)
			{
				tileSpriteBounds = AABB2(m_level->GetTileBounds(tileSelection.m_tileCoords));
				AABB2 tileUVs = tileSelection.m_tileDef->GetUVs();
				uvMins = tileUVs.m_mins;
				uvMaxs = tileUVs.m_maxs;
			}
			AddVertsForAABB2ToVector(mouseSpriteVerts, tileSpriteBounds, mouseSpriteTint, uvMins, uvMaxs);
		}

		for (int i = 0; i < (int) entitySelections.size(); i++)
		{
			EntitySpawnInfo const& info = entitySelections[i];
			AABB2 entitySpriteBounds;
			Vec2 uvMins;
			Vec2 uvMaxs;
			EntityType type = info.m_type;
			entitySpriteBounds = EntityDefinition::GetCosmeticBoundsForEntity(type, info.m_startingPosition);
			//SpriteSheet* spriteSheet = GetSpriteSheetForEntity(type);
			//IntVec2 entitySpriteCoords = GetEditorSpriteCoordsForEntity(type);
			//spriteSheet->GetSpriteUVs(uvMins, uvMaxs, entitySpriteCoords);
			//AddVertsForAABB2ToVector(mouseSpriteVerts, entitySpriteBounds, Rgba8(255, 255, 255, 100), uvMins, uvMaxs);
			EntityDefinition const* entityDef = EntityDefinition::GetEntityDefinitionByType(type);
			entityDef->AddVertsForEditorSpriteToAABB2(mouseSpriteVerts, entitySpriteBounds, mouseSpriteTint);
		}
	}

	//if (m_mouseData.m_tool == MouseTool::INSPECT_TOOL || m_mouseData.m_tool == MouseTool::PAINTING_TOOL)
	if (m_editingMode == EDITING_MODE_INSPECT || m_editingMode == EDITING_MODE_PAINT_SINGLE)
	{
		AABB2 mouseSpriteBounds;
		Vec2 uvMins;
		Vec2 uvMaxs;
		if (m_mouseData.m_isEntity)
		{
			Vec2 entityPos = m_level->GetTileBottomCenter(m_mouseData.m_currentTileCoords);
			mouseSpriteBounds = EntityDefinition::GetCosmeticBoundsForEntity(m_mouseData.m_entityType, entityPos);
			EntityType type = m_mouseData.m_entityType;
			//SpriteSheet* spriteSheet = GetSpriteSheetForEntity(m_mouseData.m_entityType);
			//IntVec2 entitySpriteCoords = GetEditorSpriteCoordsForEntity(m_mouseData.m_entityType);
			//spriteSheet->GetSpriteUVs(uvMins, uvMaxs, entitySpriteCoords);
			EntityDefinition const* entityDef = EntityDefinition::GetEntityDefinitionByType(type);
			entityDef->AddVertsForEditorSpriteToAABB2(mouseSpriteVerts, mouseSpriteBounds, mouseSpriteTint);
		}
		else
		{
			if (m_mouseData.m_tileDefinition)
			{
				mouseSpriteBounds = AABB2(m_level->GetTileBounds(m_mouseData.m_currentTileCoords));
				AABB2 tileUVs = m_mouseData.m_tileDefinition->GetUVs();
				uvMins = tileUVs.m_mins;
				uvMaxs = tileUVs.m_maxs;
				AddVertsForAABB2ToVector(mouseSpriteVerts, mouseSpriteBounds, mouseSpriteTint, uvMins, uvMaxs);
			}
		}
	}

	//if (m_mouseData.m_tool == MouseTool::MULTI_SELECT_TOOL || m_mouseData.m_tool == MouseTool::FILL_TOOL)
	if (m_editingMode == EDITING_MODE_MULTI_SELECT || m_editingMode == EDITING_MODE_PAINT_BOX)
	{
		AABB2 selectBounds = m_mouseData.GetSelectionBounds();
		DebugDrawAABB2Outline(selectBounds, 0.1f, Rgba8::YELLOW);
	}

	//if (m_mouseData.m_tool == MouseTool::LINE_TOOL)
	if (m_editingMode == EDITING_MODE_PAINT_LINE)
	{
		DebugDrawLine(m_mouseData.m_dragStartPos, m_mouseData.m_currentWorldPos - m_mouseData.m_dragStartPos, 0.1f, Rgba8::YELLOW);
	}

	if (mouseSpriteVerts.size() > 0)
	{
		g_theRenderer->BindTexture(0, g_spriteSheetTexture);
		g_theRenderer->DrawVertexArray((int) mouseSpriteVerts.size(), mouseSpriteVerts.data());
	}
}


void LevelEditor::CheckInput()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && s_showLevelSaveWindow)
	{
		s_showLevelSaveWindow = false;
		s_saveLevel = false;
	}

	if (s_showLevelSaveWindow)
		return;

	Vec2 windowDims = g_window->GetWindowDims();
	AABB2 editorCamOrthoBox = m_editorCam.GetOrthoCamBoundingBox();
	IntVec2 mouseDelta = g_inputSystem->GetMouseDelta();
	IntVec2 mousePos = g_inputSystem->GetMousePosition();

	m_mouseData.m_mouseWheel = g_inputSystem->GetMouseWheel();
	m_mouseData.m_previousWorldPos = m_mouseData.m_currentWorldPos;
	m_mouseData.m_currentWorldPos = RangeMap(mousePos.GetVec2(), Vec2::ZERO, windowDims, Vec2(editorCamOrthoBox.m_mins.x, editorCamOrthoBox.m_maxs.y), Vec2(editorCamOrthoBox.m_maxs.x, editorCamOrthoBox.m_mins.y));
	m_mouseData.m_previousTileCoords = m_mouseData.m_currentTileCoords;
	m_mouseData.m_currentTileCoords = m_level->GetTileCoords(m_mouseData.m_currentWorldPos);

	HandleMouseInput();

	if (m_debugRaycast)
	{
		static Vec2 raycastStart = Vec2::ZERO;
		static Vec2 raycastEnd = Vec2::ZERO;

		if (g_inputSystem->WasKeyJustPressed('1'))
		{
			raycastStart = m_mouseData.m_currentWorldPos;
		}

		if (g_inputSystem->WasKeyJustPressed('2'))
		{
			raycastEnd = m_mouseData.m_currentWorldPos;
		}

		Vec2 raycast = raycastEnd - raycastStart;
		float length = raycast.GetLength();
		if (length > 0)
		{
			m_debugRaycastResult = m_level->RaycastLevel(raycastStart, raycast.GetNormalized(), length);
		}
	}

	//utilities
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		m_level->ClearAllTileData();
		m_level->ClearAllSpawnData();
		m_commandHistory.clear();
		m_mouseData.EmptySelectionData();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		m_level->ClearAllSpawnData();
		m_commandHistory.clear();
		m_mouseData.EmptySelectionData();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F4))
	{
		m_level->ClearAllTileData();
		m_commandHistory.clear();
		m_mouseData.EmptySelectionData();
	}

	//debug stuff
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F5))
	{
		m_debugRaycast = !m_debugRaycast;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F6))
	{
		bool isWindowHidden = m_paletteWindow->IsHidden();
		m_paletteWindow->SetHidden(!isWindowHidden);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F7))
	{
		bool isWindowHidden = m_inspectorWindow->IsHidden();
		m_inspectorWindow->SetHidden(!isWindowHidden);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) && !m_inPlayMode)
	{
		SwapBetweenEditorAndPlayMode();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && m_inPlayMode)
	{
		SwapBetweenEditorAndPlayMode();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_DELETE) && !m_mouseData.IsSelectionDataEmpty())
	{
		EditorCommand* deleteCommand = new EditorCommand(this);
		MouseSelectionData selectionData = m_mouseData.m_selectionData;
		TileSelectionData& tileSelectionData = selectionData.m_tileSelections;
		EntitySpawns& entitySpawns = selectionData.m_entitySpawnSelections;

		for (int i = 0; i < (int) tileSelectionData.size(); i++)
		{
			TileSelection& tileSelection = tileSelectionData[i];
			TileErasingOperation* tileEraseOp = new TileErasingOperation(tileSelection.m_tileCoords, tileSelection.m_tileDef);
			if (tileEraseOp->IsValid(m_level))
			{
				tileEraseOp->Execute(this);
				deleteCommand->AddOperation(tileEraseOp);
			}
		}

		for (int i = 0; i < (int) entitySpawns.size(); i++)
		{
			EntitySpawnInfo& entitySpawn = entitySpawns[i];
			EntityErasingOperation* entityEraseOp = new EntityErasingOperation(entitySpawn);
			if (entityEraseOp->IsValid(m_level))
			{
				entityEraseOp->Execute(this);
				deleteCommand->AddOperation(entityEraseOp);
			}
		}

		AddCommandToCommandHistory(deleteCommand);
	}

	// undo/redo
	if (g_inputSystem->IsKeyPressed(KEYCODE_CTRL) && g_inputSystem->WasKeyJustPressed('Z'))
	{
		Undo();
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_CTRL) && g_inputSystem->WasKeyJustPressed('Y'))
	{
		Redo();
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_CTRL) && g_inputSystem->WasKeyJustPressed('S'))
	{
		NamedProperties args;
		OnClicked_SaveButton(args);
	}
}


void LevelEditor::HandleMouseInput()
{
	if (m_inPlayMode)
		return;

	HandleInputForCamera();

	if (m_paletteWindow->IsMouseOverWidget() && !m_paletteWindow->IsHidden())
	{
		m_paletteWindow->CheckInput(&m_mouseData);;
		return;
	}

	if (m_inspectorWindow->IsMouseOverWidget() && !m_inspectorWindow->IsHidden())
	{
		m_inspectorWindow->CheckInput();
		return;
	}

 	// the order of the if conditions is deliberate
 	// A switch statement adds a frame of delay when editing modes change

	if (m_editingMode == EDITING_MODE_INSPECT)
	{
		HandleMouseInput_Inspect();
	}

	if (m_editingMode == EDITING_MODE_MULTI_SELECT)
	{
		HandleMouseInput_MultiSelect();
	}

	if (m_editingMode == EDITING_MODE_PAINT_SINGLE)
	{
		HandleMouseInput_PaintSingle();
	}

	if (m_editingMode == EDITING_MODE_ERASE)
	{
		HandleMouseInput_Erase();
	}

	if (m_editingMode == EDITING_MODE_DRAG)
	{
		HandleMouseInput_Drag();
	}

	if (m_editingMode == EDITING_MODE_DUPLICATE)
	{
		HandleMouseInput_Duplicate();
	}

	if (m_editingMode == EDITING_MODE_PAINT_BOX)
	{
		HandleMouseInput_PaintBox();
	}

	if (m_editingMode == EDITING_MODE_PAINT_LINE)
	{
		HandleMouseInput_PaintLine();
	}

	if (m_editingMode == EDITING_MODE_WAYPOINTS)
	{
		HandleMouseInput_Waypoints();
	}
}


void LevelEditor::HandleMouseInput_PaintLine()
{
	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_mouseData.m_dragStartPos = m_mouseData.m_currentWorldPos;
	}

	if (g_inputSystem->WasMouseButtonJustReleased(0) || g_inputSystem->IsKeyUp('E'))
	{
		FillMouseLine();
		SetEditingMode(EDITING_MODE_INSPECT);
	}
}


void LevelEditor::HandleMouseInput_PaintBox()
{
	if (g_inputSystem->IsKeyUp('W'))
	{
		FillMouseSelection();
		m_mouseData.SetSelectionBounds(AABB2());
		SetEditingMode(EDITING_MODE_INSPECT);
		return;
	}

	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		m_mouseData.m_dragStartPos = m_mouseData.m_currentWorldPos;
	}

	if (g_inputSystem->IsMouseButtonPressed(0))
	{
		AABB2 selectionBounds = GetSelectionBounds(m_mouseData.m_dragStartPos, m_mouseData.m_currentWorldPos);
		m_mouseData.SetSelectionBounds(selectionBounds);
	}

	if (g_inputSystem->WasMouseButtonJustReleased(0))
	{
		FillMouseSelection();
		m_mouseData.SetSelectionBounds(AABB2());
	}
}


void LevelEditor::Redo()
{
	if (m_commandRedoStack.size() > 0)
	{
		EditorCommand* command = m_commandRedoStack.back();
		m_commandRedoStack.pop_back();
		command->Execute();
		m_commandHistory.push_back(command);
	}
}


void LevelEditor::Undo()
{
	if (m_commandHistory.size() > 0)
	{
		EditorCommand* command = m_commandHistory.back();
		m_commandHistory.pop_back();
		command->Undo();
		m_commandRedoStack.push_back(command);
	}
}


void LevelEditor::ClampAndUpdateEditorCamera(AABB2& editorCamOrthoBox)
{
	if (g_gameConfigBlackboard.GetValue("clampEditorCam", false))
	{
		Vec2 camDims = editorCamOrthoBox.GetDimensions();
		if (editorCamOrthoBox.m_maxs.x > m_editorCamBoundary.m_maxs.x)
		{
			editorCamOrthoBox.m_mins.x -= editorCamOrthoBox.m_maxs.x - m_editorCamBoundary.m_maxs.x;
			editorCamOrthoBox.m_maxs.x = m_editorCamBoundary.m_maxs.x;
		}

		if (editorCamOrthoBox.m_maxs.y > m_editorCamBoundary.m_maxs.y)
		{
			editorCamOrthoBox.m_mins.y -= editorCamOrthoBox.m_maxs.y - m_editorCamBoundary.m_maxs.y;
			editorCamOrthoBox.m_maxs.y = m_editorCamBoundary.m_maxs.y;
		}

		if (editorCamOrthoBox.m_mins.x < m_editorCamBoundary.m_mins.x)
		{
			editorCamOrthoBox.m_maxs.x += m_editorCamBoundary.m_mins.x - editorCamOrthoBox.m_mins.x;
			editorCamOrthoBox.m_mins.x = m_editorCamBoundary.m_mins.x;
		}

		if (editorCamOrthoBox.m_mins.y < m_editorCamBoundary.m_mins.y)
		{
			editorCamOrthoBox.m_maxs.y += m_editorCamBoundary.m_mins.y - editorCamOrthoBox.m_mins.y;
			editorCamOrthoBox.m_mins.y = m_editorCamBoundary.m_mins.y;
		}
	}
	m_editorCam.SetOrthoView(editorCamOrthoBox.m_mins, editorCamOrthoBox.m_maxs);
}


void LevelEditor::InitGridLineVerts()
{
	const int NUM_VERTS_LINE = 6;
	const int NUM_VERTICAL_LINES = LEVEL_SIZE_X - 1;
	const int NUM_HORIZONTAL_LINES = LEVEL_SIZE_Y - 1;
	m_gridLineVerts.clear();
	m_gridLineVerts.resize(NUM_VERTICAL_LINES * NUM_VERTS_LINE + NUM_HORIZONTAL_LINES * NUM_VERTS_LINE);

	const float lineHalfWidth = 0.05f;

	Rgba8 lineColor = Rgba8::GREY;
	lineColor.a = 100;
	for (int x = 1; x < LEVEL_SIZE_X; x++)
	{
		AABB2 verticalLine = AABB2((float) x - lineHalfWidth , 0.0f, (float) x + lineHalfWidth, (float) LEVEL_SIZE_Y);
		AddVertsForAABB2ToVector(m_gridLineVerts, verticalLine, lineColor);
	}

	for (int y = 1; y < LEVEL_SIZE_Y; y++)
	{
		AABB2 horizontalLine = AABB2(0.0f, (float) y - lineHalfWidth, (float) LEVEL_SIZE_X, (float) y + lineHalfWidth);
		AddVertsForAABB2ToVector(m_gridLineVerts, horizontalLine, lineColor);
	}
}


void LevelEditor::DeInit()
{
	if (m_inPlayMode)
	{
		m_level->Deinit_PlayMode();
	}
	else
	{
		m_level->Deinit_EditorMode();
	}

	if (m_level)
	{
		delete m_level;
		m_level = nullptr;
	}

	for (int i = 0; i < (int) m_commandHistory.size(); i++)
	{
		if (m_commandHistory[i])
		{
			delete m_commandHistory[i];
			m_commandHistory[i] = nullptr;
		}
	}

	for (int i = 0; i < (int) m_commandRedoStack.size(); i++)
	{
		if (m_commandRedoStack[i])
		{
			delete m_commandRedoStack[i];
			m_commandRedoStack[i] = nullptr;
		}
	}

	delete m_paletteWindow;
	m_paletteWindow = nullptr;

	g_audioSystem->StopSound(m_bgMusicPlaybackId);
	g_eventSystem->UnsubscribeEventCallbackObjectMethod("GameOverEvent", *this, &LevelEditor::OnGameOverEvent);
}


void LevelEditor::SwapBetweenEditorAndPlayMode()
{
	if (m_inPlayMode)
	{
		m_level->Deinit_PlayMode();
	}
	else
	{
		m_level->Deinit_EditorMode();
	}

	m_inPlayMode = !m_inPlayMode;

	if (m_inPlayMode)
	{
		g_audioSystem->SetSoundPlaybackSpeed(m_bgMusicPlaybackId, 0.0f);
		m_level->Init_PlayMode();
	}
	else
	{
		g_audioSystem->SetSoundPlaybackSpeed(m_bgMusicPlaybackId, 1.0f);
		m_level->Init_EditorMode();
	}
}


void LevelEditor::SaveLevel()
{
	std::string levelFileName = "";
	bool useWindowDialog = g_gameConfigBlackboard.GetValue("useWindowDialog", false);
	if (useWindowDialog)
	{
		levelFileName = s_levelSaveName;
	}
	else
	{
		levelFileName = Stringf("Data/Levels/%s.xml", s_levelSaveName);
	}
	g_console->AddLine(DevConsole::MINOR_INFO, levelFileName);

	//create and xml document object to store the level data
	XmlDocument levelXml;
	XmlElement* rootElement = nullptr;
	rootElement = levelXml.NewElement("Level");
	levelXml.InsertFirstChild(rootElement);

	XmlElement* gridElement = rootElement->InsertNewChildElement("Grid");
	//iterate through every tile in the tile grid
	for (int tileIndex = 0; tileIndex < LEVEL_TILE_COUNT; tileIndex++)
	{
		Tile tile = m_level->GetTile(tileIndex);
		if (!tile.IsEmpty())
		{
			//store the tile's index and definition if the tile is not empty
			XmlElement* tileElement = gridElement->InsertNewChildElement("Tile");
			tileElement->SetAttribute("index", tileIndex);
			tileElement->SetAttribute("definition", tile.m_definition->m_name.c_str());
		}
	}

	XmlElement* spawnDataElement = rootElement->InsertNewChildElement("SpawnData");
	std::vector<EntitySpawnInfo>& spawnData = m_level->m_spawnData;
	//iterate through every entity spawn information present in the level and store its details in xml
	for (int spawnIndex = 0; spawnIndex < (int) spawnData.size(); spawnIndex++)
	{
		EntitySpawnInfo& spawnInfo = spawnData[spawnIndex];
		if (!spawnInfo.IsInvalid())
		{
			XmlElement* spawnElement = spawnDataElement->InsertNewChildElement("EntitySpawn");
			spawnElement->SetAttribute("type", (int) spawnInfo.m_type);
			spawnElement->SetAttribute("pos", Stringf("%f, %f", spawnInfo.m_startingPosition.x, spawnInfo.m_startingPosition.y).c_str());
			spawnElement->SetAttribute("id", Stringf("%i", spawnInfo.m_entityId.m_id).c_str());
			spawnElement->SetAttribute("targetWaypoint", Stringf("%i", spawnInfo.m_targetWaypointId).c_str());
		}
	}

	XmlError result = levelXml.SaveFile(levelFileName.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to save level to xml. TinyXml Error Code %i", result));
	s_saveLevel = false;
	s_showLevelSaveWindow = false;
	m_paletteWindow->ShowSavedTextWidget();
}


void LevelEditor::SetEditingMode(EditingMode newMode)
{
	m_editingMode = newMode;
}


void LevelEditor::SelectTile(IntVec2 const& tileCoords)
{
	Tile tile = m_level->GetTile(tileCoords);
	TileDefinition const* tileDef = tile.m_definition;
	if (tileDef == nullptr)
		return;

	TileSelection tileSelection(tileDef, tileCoords);
	m_mouseData.AddTileSelectionToSelection(tileSelection);
	m_level->MarkTileAsSelected(tileCoords);
}


void LevelEditor::DeselectTile(IntVec2 const& tileCoords)
{
	m_mouseData.RemoveTileFromSelection(tileCoords);
	m_level->MarkTileAsDeselected(tileCoords);
}


void LevelEditor::SelectSpawnInfo(EntitySpawnInfo const& spawnInfo)
{
	if (spawnInfo.IsInvalid() || !m_level->DoesSpawnInfoExist(spawnInfo))
		return;

	m_mouseData.AddEntitySpawnToSelection(spawnInfo);
	m_level->MarkEntitySpawnAsSelected(spawnInfo);
}


void LevelEditor::DeselectSpawnInfo(EntitySpawnInfo const& spawnInfo)
{
	m_mouseData.RemoveEntitySpawnFromSelection(spawnInfo);
	m_level->MarkEntitySpawnAsDeselected(spawnInfo);
}


void LevelEditor::PaintTilesBetweenPositions(Vec2 const& start, Vec2 const& end, EditorCommand*& out_command)
{
	IntVec2 startingTileCoords = m_level->GetTileCoords(start);

	if (m_level->IsTileCoordValid(startingTileCoords.x, startingTileCoords.y))
	{
		EditorOperation* operation = nullptr; 
		if (m_mouseData.m_isEntity)
		{
			operation = PaintEntity(startingTileCoords, m_mouseData.m_entityType);
		}
		else
		{
			operation = PaintTile(startingTileCoords, m_mouseData.m_tileDefinition);
		}

		out_command->AddOperation(operation);
	}

	Vec2 raycast = end - start;
	if (raycast.GetLengthSquared() != 0)
	{
		int startingX = startingTileCoords.x;
		int startingY = startingTileCoords.y;

		float raycastX = raycast.x;
		float tPerUnitX = 1.0f / abs(raycastX);
		int tileStepX = raycastX >= 0.0f ? 1 : -1;
		int nearestTileEdgeX = tileStepX > 0 ? startingX + 1 : startingX;
		float distanceToNearestTileEdgeX = abs(start.x - (float) nearestTileEdgeX);
		float tOfNextIntersectionX = distanceToNearestTileEdgeX * tPerUnitX;

		float raycastY = raycast.y;
		float tPerUnitY = 1.0f / abs(raycastY);
		int tileStepY = raycastY >= 0.0f ? 1 : -1;
		int nearestTileEdgeY = tileStepY > 0 ? startingY + 1 : startingY;
		float distanceToNearestTileEdgeY = abs(start.y - (float) nearestTileEdgeY);
		float tOfNextIntersectionY = distanceToNearestTileEdgeY * tPerUnitY;

		IntVec2 intersectingTileCoords = startingTileCoords;
		float tOfNextIntersection = GetMin(tOfNextIntersectionX, tOfNextIntersectionY);
		while (tOfNextIntersection < 1.0f)
		{
			if (tOfNextIntersection == tOfNextIntersectionX)
			{
				intersectingTileCoords.x += tileStepX;
				if (m_level->IsTileCoordValid(intersectingTileCoords.x, intersectingTileCoords.y))
				{
					EditorOperation* operation = nullptr;
					if (m_mouseData.m_isEntity)
					{
						operation = PaintEntity(intersectingTileCoords, m_mouseData.m_entityType);
					}
					else
					{
						operation = PaintTile(intersectingTileCoords, m_mouseData.m_tileDefinition);
					}

					out_command->AddOperation(operation);
				}

				tOfNextIntersectionX += tPerUnitX;
			}

			if (tOfNextIntersection == tOfNextIntersectionY)
			{
				intersectingTileCoords.y += tileStepY;
				if (m_level->IsTileCoordValid(intersectingTileCoords.x, intersectingTileCoords.y))
				{
					EditorOperation* operation = nullptr;
					if (m_mouseData.m_isEntity)
					{
						operation = PaintEntity(intersectingTileCoords, m_mouseData.m_entityType);
					}
					else
					{
						operation = PaintTile(intersectingTileCoords, m_mouseData.m_tileDefinition);
					}

					out_command->AddOperation(operation);
				}

				tOfNextIntersectionY += tPerUnitY;
			}
			tOfNextIntersection = GetMin(tOfNextIntersectionX, tOfNextIntersectionY);
		}
	}
}


EditorOperation* LevelEditor::PaintTile(IntVec2 const& tileCoords, TileDefinition const* tileDefinition)
{
	Tile tile = m_level->GetTile(tileCoords);
	EditorOperation* operation = nullptr;

	TileDefinition const* originalTileDef = tile.m_definition;
	operation = new TilePaintingOperation(tileCoords, tileDefinition, originalTileDef);

	if (operation->IsValid(m_level))
	{
		m_level->SetTileDefinitionAtCoords(tileDefinition, tileCoords.x, tileCoords.y);
		return operation;
	}

	return nullptr;
}


EditorOperation* LevelEditor::PaintEntity(IntVec2 const& tileCoords, EntityType entityType)
{
	EntitySpawnInfo info = EntitySpawnInfo(entityType);
	//info.m_type = entityType;
	info.m_startingPosition = m_level->GetTileBottomCenter(tileCoords);

	EntitySpawns overlappingSpawns;
	m_level->AddOverlappingSpawnInfosToVector(overlappingSpawns, info);
	EditorOperation* operation = new EntityPaintingOperation(info, overlappingSpawns);
	if (operation->IsValid(m_level))
	{
		operation->Execute(this);
		return operation;
	}

	return nullptr;
}


void LevelEditor::EraseTilesBetweenPositions(Vec2 const& start, Vec2 const& end, EditorCommand*& out_command)
{
	IntVec2 startingTileCoords = m_level->GetTileCoords(start);

	if (m_level->IsTileCoordValid(startingTileCoords.x, startingTileCoords.y))
	{
		EditorOperation* operation = EraseTile(startingTileCoords);
		out_command->AddOperation(operation);
	}

	Vec2 raycast = end - start;
	if (raycast.GetLengthSquared() != 0)
	{
		int startingX = startingTileCoords.x;
		int startingY = startingTileCoords.y;

		float raycastX = raycast.x;
		float tPerUnitX = 1.0f / abs(raycastX);
		int tileStepX = raycastX >= 0.0f ? 1 : -1;
		int nearestTileEdgeX = tileStepX > 0 ? startingX + 1 : startingX;
		float distanceToNearestTileEdgeX = abs(start.x - (float) nearestTileEdgeX);
		float tOfNextIntersectionX = distanceToNearestTileEdgeX * tPerUnitX;

		float raycastY = raycast.y;
		float tPerUnitY = 1.0f / abs(raycastY);
		int tileStepY = raycastY >= 0.0f ? 1 : -1;
		int nearestTileEdgeY = tileStepY > 0 ? startingY + 1 : startingY;
		float distanceToNearestTileEdgeY = abs(start.y - (float) nearestTileEdgeY);
		float tOfNextIntersectionY = distanceToNearestTileEdgeY * tPerUnitY;

		IntVec2 intersectingTileCoords = startingTileCoords;
		float tOfNextIntersection = GetMin(tOfNextIntersectionX, tOfNextIntersectionY);
		while (tOfNextIntersection < 1.0f)
		{
			if (tOfNextIntersection == tOfNextIntersectionX)
			{
				intersectingTileCoords.x += tileStepX;
				if (m_level->IsTileCoordValid(intersectingTileCoords.x, intersectingTileCoords.y))
				{
					EditorOperation* operation = EraseTile(intersectingTileCoords);
					out_command->AddOperation(operation);
				}

				tOfNextIntersectionX += tPerUnitX;
			}

			if (tOfNextIntersection == tOfNextIntersectionY)
			{
				intersectingTileCoords.y += tileStepY;
				if (m_level->IsTileCoordValid(intersectingTileCoords.x, intersectingTileCoords.y))
				{
					EditorOperation* operation = EraseTile(intersectingTileCoords);
					out_command->AddOperation(operation);
				}

				tOfNextIntersectionY += tPerUnitY;
			}
			tOfNextIntersection = GetMin(tOfNextIntersectionX, tOfNextIntersectionY);
		}
	}
}


EditorOperation* LevelEditor::EraseTile(IntVec2 const& tileCoords)
{
	EditorOperation* operation = nullptr;
	EntitySpawnInfo info = m_level->GetEntitySpawnInfo(tileCoords);

	if (info.m_type == ENTITY_TYPE_NONE)
	{
		TileDefinition const* prevTileDef = m_level->GetTile(tileCoords).m_definition;
		operation = new TileErasingOperation(tileCoords, prevTileDef);
		if (operation->IsValid(m_level))
		{
			m_level->SetTileEmpty(tileCoords);
			DeselectTile(tileCoords);
		}
	}
	else
	{
		operation = new EntityErasingOperation(info);
		if (operation->IsValid(m_level))
		{
			m_level->RemoveEntitySpawn(info);
			DeselectSpawnInfo(info);
		}
	}
	
	return operation;
}


void LevelEditor::AddCommandToCommandHistory(EditorCommand* command)
{
	if (command == nullptr || command->GetOperationCount() <= 0)
		return;

	if (m_commandHistory.size() > 10)
	{
		m_commandHistory.pop_front();
	}

	for (int i = 0; i < m_commandRedoStack.size(); i++)
	{
		delete m_commandRedoStack[i];
		m_commandRedoStack[i] = nullptr;
	}

	m_commandRedoStack.clear();
	m_commandHistory.push_back(command);
}


Level* LevelEditor::GetLevel() const
{
	return m_level;
}


MouseData const& LevelEditor::GetMouseData() const
{
	return m_mouseData;
}


LevelEditor::EditingMode LevelEditor::GetEditingMode() const
{
	return m_editingMode;
}

