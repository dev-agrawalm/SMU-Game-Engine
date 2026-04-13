#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/Level.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <deque>
#include "Game/EditorCommand.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Game/MouseData.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class EditorPaletteWidget;
class EditorInspectorWidget;
class EditorOperation;
class NamedProperties;

class LevelEditor
{
public:
	enum EditingMode
	{
		EDITING_MODE_INSPECT,
		EDITING_MODE_PAINT_SINGLE,
		EDITING_MODE_DRAG,
		EDITING_MODE_MULTI_SELECT,
		EDITING_MODE_DUPLICATE,
		EDITING_MODE_ERASE,
		EDITING_MODE_PAINT_BOX,
		EDITING_MODE_PAINT_LINE,
		EDITING_MODE_WAYPOINTS
	};

public:
	static bool OnClicked_SaveButton(NamedProperties& args);
	static bool s_saveLevel;
	static bool s_showLevelSaveWindow;
	static char s_levelSaveName[260];

public:
	LevelEditor();
	void Initialize(std::string const& filePath = "");
	void Update();
	void Render() const;
	void CheckInput();
	void DeInit();

	void Redo();
	void Undo();
	void SwapBetweenEditorAndPlayMode();
	void SaveLevel();

	void SetEditorMusicVolume(float newVolume);
	void SetLevelMusicVolume(float newVolume);

	void SetEditingMode(EditingMode newMode);
	EditingMode GetEditingMode() const;

	void SelectTile(IntVec2 const& tileCoords);
	void DeselectTile(IntVec2 const& tileCoords);
	void SelectSpawnInfo(EntitySpawnInfo const& spawnInfo);
	void DeselectSpawnInfo(EntitySpawnInfo const& spawnInfo);
	void RemoveSelectionDataFromLevel(EditorCommand* command = nullptr);

	void PaintTilesBetweenPositions(Vec2 const& start, Vec2 const& end, EditorCommand*& out_command);
	EditorOperation* PaintTile(IntVec2 const& tileCoords, TileDefinition const* tileDefinition);
	EditorOperation* PaintEntity(IntVec2 const& tileCoords, EntityType entityType);
	
	void EraseTilesBetweenPositions(Vec2 const& start, Vec2 const& end, EditorCommand*& out_command);
	EditorOperation* EraseTile(IntVec2 const& tileCoords);

	void AddCommandToCommandHistory(EditorCommand* command);
	Level* GetLevel() const;
	MouseData const& GetMouseData() const;

private:
	void InitialiseEditorWindows(float uiOrthoX, float uiOrthoY);
	void InitialiseLevel(std::string const& filePath);
	void AddControlsToConsole();
	void ClampAndUpdateEditorCamera(AABB2& editorCamOrthoBox);
	void InitGridLineVerts();
	void PrintDebugMessages();
	void HandleInputForCamera();
	void RenderMouse() const;
	void RenderGridLines() const;
	void PopulateMouseSelectionData();
	void EmptyMouseSelectionData();
	bool IsMouseOverMouseSelection() const;
	void HighlightMouseSelection();
	bool OnGameOverEvent(EventArgs& args);
	void FillMouseSelection();
	void FillMouseLine();

private:
	void HandleMouseInput();
	void HandleMouseInput_PaintLine();
	void HandleMouseInput_PaintBox();
	void HandleMouseInput_Inspect();
	void HandleMouseInput_Drag();
	void HandleMouseInput_PaintSingle();
	void HandleMouseInput_Erase();
	void HandleMouseInput_MultiSelect();
	void HandleMouseInput_Duplicate();
	void HandleMouseInput_Waypoints();
	AABB2 GetSelectionBounds(Vec2 const& startPos, Vec2 const& endPos);

public:
	bool m_inPlayMode = false;

private:
	SoundPlaybackID m_bgMusicPlaybackId;
	Vec2 m_editorCamMaxDims;
	AABB2 m_editorCamBoundary;
	Camera m_editorCam;
	Camera m_uiCam;
	std::vector<Vertex_PCU> m_gridLineVerts;

	Level* m_level = nullptr;
	EditingMode m_editingMode = EDITING_MODE_INSPECT;
	MouseData m_mouseData = {};

	EditorPaletteWidget* m_paletteWindow = nullptr;
	EditorInspectorWidget* m_inspectorWindow = nullptr;

	EditorCommand* m_currentCommand = nullptr;
	std::vector<EditorCommand*> m_commandRedoStack;
	std::deque<EditorCommand*> m_commandHistory;

	bool m_debugRaycast = false;
	LevelRaycastResult m_debugRaycastResult = {};
};
