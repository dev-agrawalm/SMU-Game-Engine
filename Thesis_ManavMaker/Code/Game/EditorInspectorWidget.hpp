#pragma once
#include "Game/UIWidget.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/NamedProperties.hpp"

class UICheckbox;
struct EntityDefinition;
struct MouseSelectionData;
struct Tile;
class LevelEditor;
struct EntityDefinition;
class TriggerManagerWidget;
typedef NamedProperties ButtonCallbackArgs;

class EditorInspectorWidget : public UIWidget 
{
public:
	EditorInspectorWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs);
	void Render() const override;
	void Update();

	void CheckInput();

private:
	void AssembleWidget();

private:
	std::vector<Vertex_PCU> m_controlsTextVerts;
	std::vector<Vertex_PCU> m_controlsTextBoxVerts;

	UICheckbox* m_toggleEditorMusicCheckbox = nullptr;
	UICheckbox* m_toggleGameMusicCheckbox = nullptr;
	//UICheckbox* m_toggleGameSFXCheckbox = nullptr;
};
