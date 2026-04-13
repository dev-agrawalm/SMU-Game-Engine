#include "Game/EditorPaintbrushWidget.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/UIButton.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/LevelEditor.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/NamedProperties.hpp"

bool OnClick_SetMouseDataToCoin(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_RED_BLOCK;
		mouseData->m_tool = MouseTool::PAINTING_TOOL;
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToLevelEnd(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_SWITCH;
		mouseData->m_tool = MouseTool::PAINTING_TOOL;
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToGoomba(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_BLUE_BLOCK;
		mouseData->m_tool = MouseTool::PAINTING_TOOL;
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToMario(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_MARIO;
		mouseData->m_tool = MouseTool::PAINTING_TOOL;
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToGroundTile(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = false;
		mouseData->m_tileDefinition = TileDefinition::GetDefinitionByName("Ground");
		mouseData->m_tool = MouseTool::PAINTING_TOOL;
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToSpikesTile(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = false;
		mouseData->m_tileDefinition = TileDefinition::GetDefinitionByName("Spike");
		mouseData->m_tool = MouseTool::PAINTING_TOOL;
		return true;
	}

	return false;
}

bool OnClick_SetMouseDataToBridgeTile(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = false;
		mouseData->m_tileDefinition = TileDefinition::GetDefinitionByName("Bridge");
		mouseData->m_tool = MouseTool::PAINTING_TOOL;
		return true;
	}
	return false;
}


EditorPaintbrushWidget::EditorPaintbrushWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs)
	: UIWidget(renderCam, camSpaceMins, camSpaceMaxs)
{
	AssembleWidget();
}


EditorPaintbrushWidget::~EditorPaintbrushWidget()
{
	for (int buttonIndex = 0; buttonIndex < m_buttons.size(); buttonIndex++)
	{
		delete m_buttons[buttonIndex];
		m_buttons[buttonIndex] = nullptr;
	}

	delete m_saveButton;
	m_saveButton = nullptr;

	if (m_levelSavedTextWidget)
	{
		delete m_levelSavedTextWidget;
		m_levelSavedTextWidget = nullptr;
	}
}


bool EditorPaintbrushWidget::CheckInput(MouseData* mouseData)
{
	int buttonIndex = 0;
	for (buttonIndex; buttonIndex < (int) m_buttons.size(); buttonIndex++)
	{
		UIButton* button = m_buttons[buttonIndex];
		if(button->IsMouseOverWidget())
		{
			button->SetHoveredOnFrame(g_currentFrameNumber);

			Vec2 widgetDims = m_camSpaceMaxs - m_camSpaceMins;
			Vec2 textPos = m_camSpaceMins + Vec2(widgetDims.x * 0.5f, widgetDims.y * 0.75f);
			float textSize = 16.0f;
			DebugAddScreenText(button->GetText(), textPos, 0.0f, Vec2(0.5f, 0.5f), textSize);
			
			Vec2 tooltipPos = m_camSpaceMins + Vec2(widgetDims.x * 0.5f, widgetDims.y * 0.70f);
			float tooltipSize = 12.0f;
			DebugAddScreenText(button->GetToolTipText(), tooltipPos, 0.0f, Vec2(0.5f, 0.5f), tooltipSize);

			if (g_inputSystem->WasMouseButtonJustPressed(0))
			{
				NamedProperties args;
				args.AddProperty("mouseData", mouseData);
				button->OnClick(args);
				return true;
			}

			if (g_inputSystem->IsMouseButtonPressed(0))
			{
				button->SetClickedOnFrame(g_currentFrameNumber);
			}

			break;
		}
	}

	if (buttonIndex == (int) m_buttons.size())
	{
		if (m_saveButton->IsMouseOverWidget())
		{
			m_saveButton->SetHoveredOnFrame(g_currentFrameNumber);

			if (g_inputSystem->WasMouseButtonJustPressed(0))
			{
				NamedProperties args;
				args.AddProperty("mouseData", mouseData);
				m_saveButton->OnClick(args);
				return true;
			}

			if (g_inputSystem->IsMouseButtonPressed(0))
			{
				m_saveButton->SetClickedOnFrame(g_currentFrameNumber);
			}
		}
	}

	return false;
}


void EditorPaintbrushWidget::Render() const
{
	UIWidget::Render();

	for (int buttonIndex = 0; buttonIndex < m_buttons.size(); buttonIndex++)
	{
		m_buttons[buttonIndex]->Render();
	}

	m_saveButton->Render();

	if (m_levelSavedTextWidget)
	{
		m_levelSavedTextWidget->Render();
	}
}


void EditorPaintbrushWidget::ShowSavedWidget()
{
	if (m_levelSavedTextWidget)
	{
		delete m_levelSavedTextWidget;
		m_levelSavedTextWidget = nullptr;
	}

	Vec2 widgetDims = g_gameConfigBlackboard.GetValue("savedLevelTextWidgetDims", Vec2(200.0f, 50.0f));
	float widgetWidth = m_camSpaceMaxs.x - m_camSpaceMins.x;
	float windowHeight = m_camSpaceMaxs.y - m_camSpaceMins.y;
	Vec2 widgetMins = m_camSpaceMins + Vec2(widgetWidth * 0.35f, windowHeight * 0.10f);
	Vec2 widgetMaxs = widgetMins + widgetDims;
	m_levelSavedTextWidget = new UIWidget(m_renderCam, widgetMins, widgetMaxs, Rgba8::GREY);
	m_levelSavedTextWidget->SetText("Saved");
	m_levelSavedTextWidget->SetFadeoutDuration(1.0f);
	m_levelSavedTextWidget->SetFadeoutStartTime((float) GetCurrentTimeSeconds());
}


void EditorPaintbrushWidget::AssembleWidget()
{
	float widgetWidth = m_camSpaceMaxs.x - m_camSpaceMins.x;
	float windowHeight = m_camSpaceMaxs.y - m_camSpaceMins.y;
	Vec2 buttonDims = g_gameConfigBlackboard.GetValue("paintBrushButtonDims", Vec2(0.0f, 0.0f));

	Vec2 buttonRelativeMins = Vec2(widgetWidth * 0.15f, windowHeight * 0.60f);
	Vec2 buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	Vec2 buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	Vec2 buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	TileDefinition const* tileDef = TileDefinition::GetDefinitionByName("Ground");
	SpriteDefinition const* buttonSpriteDef = tileDef->GetSpriteDefinition();
	ButtonCallbackFunction* callbackGround = new ButtonCallbackFunction();
	callbackGround->m_funcPtr = OnClick_SetMouseDataToGroundTile;
	UIButton* button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackGround, Rgba8::WHITE, buttonSpriteDef, "Ground", "completely solid");
	button->SetShowText(false);
	m_buttons.push_back(button);

	buttonRelativeMins = Vec2(widgetWidth * 0.45f, windowHeight * 0.60f);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	tileDef = TileDefinition::GetDefinitionByName("Spike");
	buttonSpriteDef = tileDef->GetSpriteDefinition();
	ButtonCallbackFunction* callbackSpikes = new ButtonCallbackFunction();
	callbackSpikes->m_funcPtr = OnClick_SetMouseDataToSpikesTile;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackSpikes, Rgba8::WHITE, buttonSpriteDef, "Spikes", "damaging tile");
	button->SetShowText(false);
	m_buttons.push_back(button);

	buttonRelativeMins = Vec2(widgetWidth * 0.75f, windowHeight * 0.60f);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	tileDef = TileDefinition::GetDefinitionByName("Bridge");
	buttonSpriteDef = tileDef->GetSpriteDefinition();
	ButtonCallbackFunction* callbackBridge = new ButtonCallbackFunction();
	callbackBridge->m_funcPtr = OnClick_SetMouseDataToBridgeTile;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackBridge, Rgba8::WHITE, buttonSpriteDef, "Bridge", "one way tile");
	button->SetShowText(false);
	m_buttons.push_back(button);

	buttonRelativeMins = Vec2(widgetWidth * 0.15f, windowHeight * 0.45f);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetEditorSpriteDefinitionForEntity(ENTITY_TYPE_MARIO);
	ButtonCallbackFunction* callbackMario = new ButtonCallbackFunction();
	callbackMario->m_funcPtr = OnClick_SetMouseDataToMario;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackMario, Rgba8::WHITE, buttonSpriteDef, "Mario", "player controlled character");
	button->SetShowText(false);
	m_buttons.push_back(button);

	buttonRelativeMins = Vec2(widgetWidth * 0.45f, windowHeight * 0.45f);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetEditorSpriteDefinitionForEntity(ENTITY_TYPE_BLUE_BLOCK);
	ButtonCallbackFunction* callbackGoomba = new ButtonCallbackFunction();
	callbackGoomba->m_funcPtr = OnClick_SetMouseDataToGoomba;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackGoomba, Rgba8::WHITE, buttonSpriteDef, "Goomba", "walking enemy");
	button->SetShowText(false);
	m_buttons.push_back(button);

	buttonRelativeMins = Vec2(widgetWidth * 0.75f, windowHeight * 0.45f);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetEditorSpriteDefinitionForEntity(ENTITY_TYPE_RED_BLOCK);
	ButtonCallbackFunction* callbackCoin = new ButtonCallbackFunction();
	callbackCoin->m_funcPtr = OnClick_SetMouseDataToCoin;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackCoin, Rgba8::WHITE, buttonSpriteDef, "Coin", "collectible for Mario");
	button->SetShowText(false);
	m_buttons.push_back(button);

	buttonRelativeMins = Vec2(widgetWidth * 0.15f, windowHeight * 0.30f);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetEditorSpriteDefinitionForEntity(ENTITY_TYPE_SWITCH);
	ButtonCallbackFunction* callbackLevelEnd = new ButtonCallbackFunction();
	callbackLevelEnd->m_funcPtr = OnClick_SetMouseDataToLevelEnd;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackLevelEnd, Rgba8::WHITE, buttonSpriteDef, "Level End", "triggers end of level");
	button->SetShowText(false);
	m_buttons.push_back(button);

	Vec2 saveButtonDims = g_gameConfigBlackboard.GetValue("saveButtonDims", Vec2(100.0f, 50.0f));
	buttonRelativeMins = Vec2(widgetWidth * 0.35f, windowHeight * 0.2f);
	buttonRelativeMaxs = buttonRelativeMins + saveButtonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	ButtonCallbackFunction* callbackSaveGame = new ButtonCallbackFunction();
	callbackSaveGame->m_funcPtr = LevelEditor::OnClicked_SaveButton;
	m_saveButton = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackSaveGame, Rgba8::GREY, nullptr, "Save Level", "saves the level to disk");
}
