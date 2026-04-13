#include "Game/EditorInspectorWidget.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MouseData.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/LevelEditor.hpp"
#include "Game/Level.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/UICheckbox.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/TriggerManagerWidget.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"

EditorInspectorWidget::EditorInspectorWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs)
	: UIWidget(renderCam, camSpaceMins, camSpaceMaxs)
{
	AssembleWidget();
}


void EditorInspectorWidget::Render() const
{
	UIWidget::Render();

// 	g_theRenderer->BindTexture(0, nullptr);
// 	g_theRenderer->DrawVertexArray((int) m_controlsTextBoxVerts.size(), m_controlsTextBoxVerts.data());

	g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray((int) m_controlsTextVerts.size(), m_controlsTextVerts.data());

	if (m_toggleEditorMusicCheckbox)
	{
		m_toggleEditorMusicCheckbox->Render();
	}

	if (m_toggleGameMusicCheckbox)
	{
		m_toggleGameMusicCheckbox->Render();
	}

// 	if (m_toggleGameSFXCheckbox)
// 	{
// 		m_toggleGameSFXCheckbox->Render();
// 	}
}


void EditorInspectorWidget::Update()
{
	if (IsHidden())
		return;
}


void EditorInspectorWidget::CheckInput()
{
	if (m_toggleEditorMusicCheckbox)
	{
		if (m_toggleEditorMusicCheckbox->IsMouseOverWidget())
		{
			m_toggleEditorMusicCheckbox->SetHoveredOnFrame(g_currentFrameNumber);

			if (g_inputSystem->WasMouseButtonJustPressed(0))
			{
				NamedProperties args;
				m_toggleEditorMusicCheckbox->OnClick(args);
				float editorMusicVolume = m_toggleEditorMusicCheckbox->IsChecked() ? 0.0f : EDITOR_MUSIC_VOLUME;
				g_levelEditor->SetEditorMusicVolume(editorMusicVolume);
			}

			if (g_inputSystem->IsMouseButtonPressed(0))
			{
				m_toggleEditorMusicCheckbox->SetClickedOnFrame(g_currentFrameNumber);
			}

			return;
		}
	}

	if (m_toggleGameMusicCheckbox)
	{
		if (m_toggleGameMusicCheckbox->IsMouseOverWidget())
		{
			m_toggleGameMusicCheckbox->SetHoveredOnFrame(g_currentFrameNumber);

			if (g_inputSystem->WasMouseButtonJustPressed(0))
			{
				NamedProperties args;
				m_toggleGameMusicCheckbox->OnClick(args);
				float gameMusicVolume = m_toggleGameMusicCheckbox->IsChecked() ? 0.0f : GAME_MUSIC_VOLUME;
				g_levelEditor->SetLevelMusicVolume(gameMusicVolume);
			}

			if (g_inputSystem->IsMouseButtonPressed(0))
			{
				m_toggleGameMusicCheckbox->SetClickedOnFrame(g_currentFrameNumber);
			}

			return;
		}
	}

// 	if (m_toggleGameSFXCheckbox)
// 	{
// 		if (m_toggleGameSFXCheckbox->IsMouseOverWidget())
// 		{
// 			m_toggleGameSFXCheckbox->SetHoveredOnFrame(g_currentFrameNumber);
// 
// 			if (g_inputSystem->WasMouseButtonJustPressed(0))
// 			{
// 				NamedProperties args;
// 				m_toggleGameSFXCheckbox->OnClick(args);
// 				g_levelEditor->SetMuteLevelMusic(isChecked);
// 			}
// 
// 			if (g_inputSystem->IsMouseButtonPressed(0))
// 			{
// 				m_toggleGameSFXCheckbox->SetClickedOnFrame(g_currentFrameNumber);
// 			}
// 
// 			return;
// 		}
// 	}
}


void EditorInspectorWidget::AssembleWidget()
{
// 	std::string controlsText = 
// 		"Controls\n\nEditor\nF6: Show/Hide Palette\nF7: Show/Hide Controls\nEnter: Switch bw editing and playing\nLMB: Paint\nShift(hold) + LMB: Select tiles/entities\nAlt(hold) + LMB: Duplicate Selection\nDel: Delete selection\nCtrl(hold) + LMB: Erase\nW(hold) + LMB: Box Paint\nE(hold) + LMB: Line Paint\nQ(hold) + LMB: Waypoint Editor\nRMB: Pan Level\nMouseWheel: Zoom in/out\nF2: Clear Level(cannot be undone)\n\nGameplay\nA/D: Move Mario\nW: Jump";

	XmlDocument inspectWidgetConfig;
	XmlError result = inspectWidgetConfig.LoadFile("Data/XMLData/InspectorWidgetConfig.xml");
	GUARANTEE_OR_DIE(result == tinyxml2::XMLError::XML_SUCCESS, "Unable to open/find Data/XMLData/InspectWidgetConfig.xml");

	XmlElement* rootElement = inspectWidgetConfig.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "Config") == 0, "Inspect Widget Config file is missing the root \"Config\" element");
	
	Vec2 inspectorWidgetDims = GetDimensions();
	Vec2 boxDims;
	float relativeMinsFractionX = 0.0f;
	float relativeMinsFractionY = 0.0f;
	Vec2 boxRelativeMins;
	Vec2 boxMins;
	Vec2 boxMaxs;
	
	XmlElement* headerBoxElement = rootElement->FirstChildElement("HeaderBox");
	boxDims = ParseXmlAttribute(*headerBoxElement, "dimensions", Vec2::ZERO);
	relativeMinsFractionX = ParseXmlAttribute(*headerBoxElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*headerBoxElement, "relativeMinsFractionY", 0.0f);
	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
	boxMins = m_camSpaceMins + boxRelativeMins;
	boxMaxs = boxMins + boxDims;
	AABB2 headerBox = AABB2(boxMins, boxMaxs);
	g_bitmapFont->AddVertsForTextInAABB2(m_controlsTextVerts, headerBox, 45.0f, "Controls", Rgba8::BLACK, 1.0f, BitmapFont::ALIGNED_BOTTOM_CENTER);

	XmlElement* editorControlsHeaderBoxElement = rootElement->FirstChildElement("EditorControlsHeader");
	boxDims = ParseXmlAttribute(*editorControlsHeaderBoxElement, "dimensions", Vec2::ZERO);
	relativeMinsFractionX = ParseXmlAttribute(*editorControlsHeaderBoxElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*editorControlsHeaderBoxElement, "relativeMinsFractionY", 0.0f);
	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
	boxMins = m_camSpaceMins + boxRelativeMins;
	boxMaxs = boxMins + boxDims;
	AABB2 editorControlsHeaderBox = AABB2(boxMins, boxMaxs);
	g_bitmapFont->AddVertsForTextInAABB2(m_controlsTextVerts, editorControlsHeaderBox, 35.0f, "Editor", Rgba8::BLACK);

	XmlElement* editorControlsBoxElement = rootElement->FirstChildElement("EditorControlsBox");
	boxDims = ParseXmlAttribute(*editorControlsBoxElement, "dimensions", Vec2::ZERO);
	relativeMinsFractionX = ParseXmlAttribute(*editorControlsBoxElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*editorControlsBoxElement, "relativeMinsFractionY", 0.0f);
	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
	boxMins = m_camSpaceMins + boxRelativeMins;
	boxMaxs = boxMins + boxDims;
	AABB2 editorControlsBox = AABB2(boxMins, boxMaxs);
	g_bitmapFont->AddVertsForTextInAABB2(m_controlsTextVerts, editorControlsBox, 30.0f, "F6: Show/Hide Palette\nF7: Show/Hide Controls\nEnter: Switch bw editing and playing\nLMB: Paint\nShift(hold) + LMB: Select tiles/entities\nAlt(hold) + LMB: Duplicate Selection\nDel: Delete selection\nCtrl(hold) + LMB: Erase\nW(hold) + LMB: Box Paint\nE(hold) + LMB: Line Paint\nQ(hold) + LMB: Waypoint Editor\nRMB: Pan Level\nMouseWheel: Zoom in/out\nF2: Clear Level(cannot be undone)"
	, Rgba8::BLACK, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);

	XmlElement* gameplayControlsHeaderBoxElement = rootElement->FirstChildElement("GameplayControlsHeader");
	boxDims = ParseXmlAttribute(*gameplayControlsHeaderBoxElement, "dimensions", Vec2::ZERO);
	relativeMinsFractionX = ParseXmlAttribute(*gameplayControlsHeaderBoxElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*gameplayControlsHeaderBoxElement, "relativeMinsFractionY", 0.0f);
	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
	boxMins = m_camSpaceMins + boxRelativeMins;
	boxMaxs = boxMins + boxDims;
	AABB2 gameplayControlsHeaderBox = AABB2(boxMins, boxMaxs);
	g_bitmapFont->AddVertsForTextInAABB2(m_controlsTextVerts, gameplayControlsHeaderBox, 35.0f, "Gameplay", Rgba8::BLACK);

	XmlElement* gameplayControlsBoxElement = rootElement->FirstChildElement("GameplayControlsBox");
	boxDims = ParseXmlAttribute(*gameplayControlsBoxElement, "dimensions", Vec2::ZERO);
	relativeMinsFractionX = ParseXmlAttribute(*gameplayControlsBoxElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*gameplayControlsBoxElement, "relativeMinsFractionY", 0.0f);
	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
	boxMins = m_camSpaceMins + boxRelativeMins;
	boxMaxs = boxMins + boxDims;
	AABB2 gameplayControlsBox = AABB2(boxMins, boxMaxs);
	g_bitmapFont->AddVertsForTextInAABB2(m_controlsTextVerts, gameplayControlsBox, 30.0f, "A/D: Move Mario\nW: Jump", Rgba8::BLACK, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);

	XmlElement* toggleEditorMusicCheckboxElement = rootElement->FirstChildElement("ToggleEditorMusicCheckbox");
	boxDims = ParseXmlAttribute(*toggleEditorMusicCheckboxElement, "dimensions", Vec2::ZERO);
	relativeMinsFractionX = ParseXmlAttribute(*toggleEditorMusicCheckboxElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*toggleEditorMusicCheckboxElement, "relativeMinsFractionY", 0.0f);
	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
	boxMins = m_camSpaceMins + boxRelativeMins;
	boxMaxs = boxMins + boxDims;
	m_toggleEditorMusicCheckbox = new UICheckbox(m_renderCam, boxMins, boxMaxs, false);
	m_toggleEditorMusicCheckbox->SetText("Mute Editor Music");
	m_toggleEditorMusicCheckbox->SetTextColor(Rgba8::BLACK);

	XmlElement* toggleGameMusicCheckboxElement = rootElement->FirstChildElement("ToggleGameMusicCheckbox");
	boxDims = ParseXmlAttribute(*toggleGameMusicCheckboxElement, "dimensions", Vec2::ZERO);
	relativeMinsFractionX = ParseXmlAttribute(*toggleGameMusicCheckboxElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*toggleGameMusicCheckboxElement, "relativeMinsFractionY", 0.0f);
	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
	boxMins = m_camSpaceMins + boxRelativeMins;
	boxMaxs = boxMins + boxDims;
	m_toggleGameMusicCheckbox = new UICheckbox(m_renderCam, boxMins, boxMaxs, false);
	m_toggleGameMusicCheckbox->SetText("Mute Game Music");
	m_toggleGameMusicCheckbox->SetTextColor(Rgba8::BLACK);

// 	XmlElement* toggleGameSFXCheckboxElement = rootElement->FirstChildElement("ToggleGameSFXCheckbox");
// 	boxDims = ParseXmlAttribute(*toggleGameSFXCheckboxElement, "dimensions", Vec2::ZERO);
// 	relativeMinsFractionX = ParseXmlAttribute(*toggleGameSFXCheckboxElement, "relativeMinsFractionX", 0.0f);
// 	relativeMinsFractionY = ParseXmlAttribute(*toggleGameSFXCheckboxElement, "relativeMinsFractionY", 0.0f);
// 	boxRelativeMins = Vec2(inspectorWidgetDims.x * relativeMinsFractionX, inspectorWidgetDims.y * relativeMinsFractionY);
// 	boxMins = m_camSpaceMins + boxRelativeMins;
// 	boxMaxs = boxMins + boxDims;
// 	m_toggleGameSFXCheckbox = new UICheckbox(m_renderCam, boxMins, boxMaxs, false);
// 	m_toggleGameSFXCheckbox->SetText("Mute Game SFX");
}