#include "Game/EditorPaletteWidget.hpp"
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
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Game.hpp"

bool OnClick_SetMouseDataToCoin(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_COIN;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToRedMushroom(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_RED_MUSHROOM;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToWaypoint(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_WAYPOINT;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToTurtle(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_KOOPA_TROOPA;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToBowser(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_BOWSER;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToGreenShell(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_TURTLE_SHELL;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToSwitch(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_SWITCH_BLOCK;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToRedBlock(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_RED_BLOCK;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToBlueBlock(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_BLUE_BLOCK;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
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
		mouseData->m_entityType = ENTITY_TYPE_LEVEL_END;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToBreakBlock(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_BREAKABLE_BLOCK;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}

bool OnClick_SetMouseDataToFlipBlock(NamedProperties& args)
{
	MouseData* mouseData = nullptr;
	args.GetProperty<MouseData*>("mouseData", mouseData, nullptr);
	if (mouseData)
	{
		mouseData->m_isEntity = true;
		mouseData->m_entityType = ENTITY_TYPE_FLIPPABLE_BLOCK;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
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
		mouseData->m_entityType = ENTITY_TYPE_GOOMBA;
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
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
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
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
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
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
		mouseData->m_tileDefinition = TileDefinition::GetDefinitionByName("Spikes");
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
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
		//mouseData->m_tool = MouseTool::PAINTING_TOOL;
		g_levelEditor->SetEditingMode(LevelEditor::EDITING_MODE_PAINT_SINGLE);
		return true;
	}
	return false;
}


EditorPaletteWidget::EditorPaletteWidget(Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs)
	: UIWidget(renderCam, camSpaceMins, camSpaceMaxs)
{
	AssembleWidget();
}


EditorPaletteWidget::~EditorPaletteWidget()
{
	for (int buttonIndex = 0; buttonIndex < m_buttons.size(); buttonIndex++)
	{
		delete m_buttons[buttonIndex];
		m_buttons[buttonIndex] = nullptr;
	}

	delete m_buttonNameWidget;
	m_buttonNameWidget = nullptr;

	delete m_buttonTooltipWidget;
	m_buttonTooltipWidget = nullptr;

	delete m_saveButton;
	m_saveButton = nullptr;

	if (m_levelSavedTextWidget)
	{
		delete m_levelSavedTextWidget;
		m_levelSavedTextWidget = nullptr;
	}
}


bool EditorPaletteWidget::CheckInput(MouseData* mouseData)
{
	m_buttonNameWidget->SetText("");
	m_buttonTooltipWidget->SetText("");
	int buttonIndex = 0;
	for (buttonIndex; buttonIndex < (int) m_buttons.size(); buttonIndex++)
	{
		UIButton* button = m_buttons[buttonIndex];
		if(button->IsMouseOverWidget())
		{
			button->SetHoveredOnFrame(g_currentFrameNumber);

			m_buttonNameWidget->SetText(button->GetText());
			m_buttonTooltipWidget->SetText(button->GetToolTipText());
			
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

			if (g_inputSystem->IsMouseButtonPressed(0))
			{
				m_saveButton->SetClickedOnFrame(g_currentFrameNumber);
			}

			if (g_inputSystem->WasMouseButtonJustReleased(0))
			{
				NamedProperties args;
				args.AddProperty("mouseData", mouseData);
				m_saveButton->OnClick(args);
				return true;
			}
		}
	}

	return false;
}


void EditorPaletteWidget::Render() const
{
	UIWidget::Render();

	for (int buttonIndex = 0; buttonIndex < m_buttons.size(); buttonIndex++)
	{
		m_buttons[buttonIndex]->Render();
	}

	m_buttonNameWidget->Render();
	m_buttonTooltipWidget->Render();
	m_saveButton->Render();

	if (m_levelSavedTextWidget)
	{
		m_levelSavedTextWidget->Render();
	}
}


void EditorPaletteWidget::ShowSavedTextWidget()
{
	if (m_levelSavedTextWidget)
	{
		delete m_levelSavedTextWidget;
		m_levelSavedTextWidget = nullptr;
	}

	//Vec2 widgetDims = g_gameConfigBlackboard.GetValue("savedLevelTextWidgetDims", Vec2(200.0f, 50.0f));
	float widgetWidth = m_camSpaceMaxs.x - m_camSpaceMins.x;
	float windowHeight = m_camSpaceMaxs.y - m_camSpaceMins.y;
	Vec2 widgetMins = m_camSpaceMins + Vec2(widgetWidth * 0.35f, windowHeight * 0.10f);
	Vec2 widgetMaxs = widgetMins + m_saveButtonDims;
	m_levelSavedTextWidget = new UIWidget(m_renderCam, widgetMins, widgetMaxs, Rgba8::GREY);
	m_levelSavedTextWidget->SetText("Saved");
	m_levelSavedTextWidget->SetFadeoutDuration(1.0f);
	m_levelSavedTextWidget->SetFadeoutStartTime((float) GetCurrentTimeSeconds());
}


void EditorPaletteWidget::AssembleWidget()
{
	XmlDocument paletteWidgetConfig;
	XmlError result = paletteWidgetConfig.LoadFile("Data/XMLData/PaletteWidgetConfig.xml");
	GUARANTEE_OR_DIE(result == tinyxml2::XMLError::XML_SUCCESS, "Unable to open/find Data/XMLData/PaletteWidgetConfig.xml");

	XmlElement* rootElement = paletteWidgetConfig.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "Config") == 0, "Palette Widget Config file is missing the root \"Config\" element");

	float widgetWidth = m_camSpaceMaxs.x - m_camSpaceMins.x;
	float windowHeight = m_camSpaceMaxs.y - m_camSpaceMins.y;
	Vec2 buttonDims = ParseXmlAttribute(*rootElement, "palleteButtonDims", Vec2::ZERO);
	Vec2 buttonNameWidgetDims = ParseXmlAttribute(*rootElement, "buttonNameDims", Vec2::ZERO);
	Vec2 buttonToolTipWidgetDims = ParseXmlAttribute(*rootElement, "buttonToolTipDims", Vec2::ZERO);

	float relativeMinsFractionX = 0.0f;
	float relativeMinsFractionY = 0.0f;

	//GROUND BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* groundButtonElement = rootElement->FirstChildElement("GroundTileButton");
	GUARANTEE_OR_DIE(groundButtonElement != nullptr, "Palette Widget Config file is missing the \"GroundTileButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*groundButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*groundButtonElement, "relativeMinsFractionY", 0.0f);
	Vec2 buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	Vec2 buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	Vec2 buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	Vec2 buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	TileDefinition const* tileDef = TileDefinition::GetDefinitionByName("Ground");
	SpriteDefinition const* buttonSpriteDef = tileDef->GetSpriteDefinition();
	ButtonCallbackFunction* callbackGround = new ButtonCallbackFunction();
	callbackGround->m_funcPtr = OnClick_SetMouseDataToGroundTile;
	UIButton* button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackGround, Rgba8::WHITE, buttonSpriteDef, "Ground"
											  , "Solid tile\nMario and enemies can walk on it\nCannot be assigned to waypoints");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//SPIKES BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* spikesButtonElement = rootElement->FirstChildElement("SpikesTileButton");
	GUARANTEE_OR_DIE(spikesButtonElement != nullptr, "Palette Widget Config file is missing the \"SpikesTileButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*spikesButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*spikesButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	tileDef = TileDefinition::GetDefinitionByName("Spikes");
	buttonSpriteDef = tileDef->GetSpriteDefinition();
	ButtonCallbackFunction* callbackSpikes = new ButtonCallbackFunction();
	callbackSpikes->m_funcPtr = OnClick_SetMouseDataToSpikesTile;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackSpikes, Rgba8::WHITE, buttonSpriteDef, "Spikes"
									, "Solid tile\nMario and enemies will take damage\nif they touch it\nCannot be assigned to waypoints");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//BRIDGE BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* bridgeButtonElement = rootElement->FirstChildElement("BridgeTileButton");
	GUARANTEE_OR_DIE(bridgeButtonElement != nullptr, "Palette Widget Config file is missing the \"BridgeTileButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*bridgeButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*bridgeButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	tileDef = TileDefinition::GetDefinitionByName("Bridge");
	buttonSpriteDef = tileDef->GetSpriteDefinition();
	ButtonCallbackFunction* callbackBridge = new ButtonCallbackFunction();
	callbackBridge->m_funcPtr = OnClick_SetMouseDataToBridgeTile;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackBridge, Rgba8::WHITE, buttonSpriteDef, "Bridge"
									, "Semi-solid tile\nMario can jump through them but\nenemies and mario can still walk on it\nCannot be assigned to waypoints");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//MARIO BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* marioButtonElement = rootElement->FirstChildElement("MarioButton");
	GUARANTEE_OR_DIE(marioButtonElement != nullptr, "Palette Widget Config file is missing the \"MarioButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*marioButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*marioButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_MARIO);
	ButtonCallbackFunction* callbackMario = new ButtonCallbackFunction();
	callbackMario->m_funcPtr = OnClick_SetMouseDataToMario;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackMario, Rgba8::WHITE, buttonSpriteDef, "Mario"
									, "The protagonist\nControlled by the player");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//GOOMBA BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* goombaButtonElement = rootElement->FirstChildElement("GoombaButton");
	GUARANTEE_OR_DIE(goombaButtonElement != nullptr, "Palette Widget Config file is missing the \"GoombaButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*goombaButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*goombaButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_GOOMBA);
	ButtonCallbackFunction* callbackGoomba = new ButtonCallbackFunction();
	callbackGoomba->m_funcPtr = OnClick_SetMouseDataToGoomba;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackGoomba, Rgba8::WHITE, buttonSpriteDef, "Goomba"
									, "Basic enemy\nWalks from side to side\nWill not walk off an edge");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//COIN BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* coinButtonElement = rootElement->FirstChildElement("CoinButton");
	GUARANTEE_OR_DIE(coinButtonElement != nullptr, "Palette Widget Config file is missing the \"CoinButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*coinButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*coinButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_COIN);
	ButtonCallbackFunction* callbackCoin = new ButtonCallbackFunction();
	callbackCoin->m_funcPtr = OnClick_SetMouseDataToCoin;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackCoin, Rgba8::WHITE, buttonSpriteDef, "Coin"
									, "Shiny collectible for Mario");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//LEVEL END BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* levelEndButtonElement = rootElement->FirstChildElement("LevelEndButton");
	GUARANTEE_OR_DIE(levelEndButtonElement != nullptr, "Palette Widget Config file is missing the \"LevelEndButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*levelEndButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*levelEndButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_LEVEL_END);
	ButtonCallbackFunction* callbackLevelEnd = new ButtonCallbackFunction();
	callbackLevelEnd->m_funcPtr = OnClick_SetMouseDataToLevelEnd;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackLevelEnd, Rgba8::WHITE, buttonSpriteDef, "Level End Axe"
									, "You win the level if you touch the axe");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//SWITCH BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* switchButtonElement = rootElement->FirstChildElement("SwitchButton");
	GUARANTEE_OR_DIE(switchButtonElement != nullptr, "Palette Widget Config file is missing the \"SwitchButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*switchButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*switchButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_SWITCH_BLOCK);
	ButtonCallbackFunction* callbackSwitch = new ButtonCallbackFunction();
	callbackSwitch->m_funcPtr = OnClick_SetMouseDataToSwitch;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackSwitch, Rgba8::WHITE, buttonSpriteDef, "Switch"
									, "Can be hit like other blocks\nOn hit, toggles between on and off\nAffects red and blue blocks");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//RED BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* redBlockButtonElement = rootElement->FirstChildElement("RedBlockButton");
	GUARANTEE_OR_DIE(redBlockButtonElement != nullptr, "Palette Widget Config file is missing the \"RedBlockButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*redBlockButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*redBlockButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_RED_BLOCK);
	ButtonCallbackFunction* callbackRedBlock = new ButtonCallbackFunction();
	callbackRedBlock->m_funcPtr = OnClick_SetMouseDataToRedBlock;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackRedBlock, Rgba8::WHITE, buttonSpriteDef, "Red Block"
									, "Solid block\nMario and enemies can walk on it\nIs solid when switch is on\nIs transparent when switch is off");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//BLUE BLOCK ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* blueBlockButtonElement = rootElement->FirstChildElement("BlueBlockButton");
	GUARANTEE_OR_DIE(blueBlockButtonElement != nullptr, "Palette Widget Config file is missing the \"BlueBlockButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*blueBlockButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*blueBlockButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_BLUE_BLOCK);
	ButtonCallbackFunction* callbackBlueBlock = new ButtonCallbackFunction();
	callbackBlueBlock->m_funcPtr = OnClick_SetMouseDataToBlueBlock;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackBlueBlock, Rgba8::WHITE, buttonSpriteDef, "Blue Block"
									, "Solid block\nMario and enemies can walk on it\nIs solid when switch is off\nIs transparent when switch is on");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//BREAK BLOCK BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* breakBlockButtonElement = rootElement->FirstChildElement("BreakBlockButton");
	GUARANTEE_OR_DIE(breakBlockButtonElement != nullptr, "Palette Widget Config file is missing the \"BreakBlockButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*breakBlockButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*breakBlockButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_BREAKABLE_BLOCK);
	ButtonCallbackFunction* callbackBreakBlock = new ButtonCallbackFunction();
	callbackBreakBlock->m_funcPtr = OnClick_SetMouseDataToBreakBlock;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackBreakBlock, Rgba8::WHITE, buttonSpriteDef, "Break Block"
									, "Solid block\nMario and enemies can walk on it\nBreaks when hit by mario or koopa shell");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//FLIP BLOCK BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* flipBlockButtonElement = rootElement->FirstChildElement("FlipBlockButton");
	GUARANTEE_OR_DIE(flipBlockButtonElement != nullptr, "Palette Widget Config file is missing the \"FlipBlockButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*flipBlockButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*flipBlockButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_FLIPPABLE_BLOCK);
	ButtonCallbackFunction* callbackFlipBlock = new ButtonCallbackFunction();
	callbackFlipBlock->m_funcPtr = OnClick_SetMouseDataToFlipBlock;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackFlipBlock, Rgba8::WHITE, buttonSpriteDef, "Flip Block"
									, "Solid block\nMario and enemies can walk on it\nFlips and becomes not-solid when\nhit by mario or koopa shell");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//TURTLE BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* turtleButtonElement = rootElement->FirstChildElement("TurtleButton");
	GUARANTEE_OR_DIE(turtleButtonElement != nullptr, "Palette Widget Config file is missing the \"TurtleButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*turtleButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*turtleButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_KOOPA_TROOPA);
	ButtonCallbackFunction* callbackTurtle = new ButtonCallbackFunction();
	callbackTurtle->m_funcPtr = OnClick_SetMouseDataToTurtle;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackTurtle, Rgba8::WHITE, buttonSpriteDef, "Koopa Troopa"
									, "Basic enemy\nWalks from side to side\nWill not walk off an edge\nSpawns a shell on death");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//GREEN SHELL BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* greenShellButtonElement = rootElement->FirstChildElement("GreenShellButton");
	GUARANTEE_OR_DIE(greenShellButtonElement != nullptr, "Palette Widget Config file is missing the \"GreenShellButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*greenShellButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*greenShellButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_TURTLE_SHELL);
	ButtonCallbackFunction* callbackGreenShell = new ButtonCallbackFunction();
	callbackGreenShell->m_funcPtr = OnClick_SetMouseDataToGreenShell;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackGreenShell, Rgba8::WHITE, buttonSpriteDef, "Koopa Shell"
									, "Shell of a Koopa Troopa\nCan break/flip/toggle blocks and switches\nCan hurt mario and enemies when moving\nWill start moving when hit from the side");
	button->SetShowText(false);
	m_buttons.push_back(button);
	
	//RED MUSHROOM BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* redMushroomButtonElement = rootElement->FirstChildElement("RedMushroomButton");
	GUARANTEE_OR_DIE(redMushroomButtonElement != nullptr, "Palette Widget Config file is missing the \"RedMushroomButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*redMushroomButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*redMushroomButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_RED_MUSHROOM);
	ButtonCallbackFunction* callbackRedMushroom = new ButtonCallbackFunction();
	callbackRedMushroom->m_funcPtr = OnClick_SetMouseDataToRedMushroom;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackRedMushroom, Rgba8::WHITE, buttonSpriteDef, "Red Mushroom"
									, "Gives mario health");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//WAYPOINT BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* waypointButtonElement = rootElement->FirstChildElement("WaypointButton");
	GUARANTEE_OR_DIE(waypointButtonElement != nullptr, "Palette Widget Config file is missing the \"WaypointButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*waypointButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*waypointButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_WAYPOINT);
	ButtonCallbackFunction* callbackWaypoint = new ButtonCallbackFunction();
	callbackWaypoint->m_funcPtr = OnClick_SetMouseDataToWaypoint;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackWaypoint, Rgba8::WHITE, buttonSpriteDef, "Waypoint"
									, "Used to create custom entity paths\nPress Q to enter waypoint editing mode");
	button->SetShowText(false);
	m_buttons.push_back(button);

	//BOWSER BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* bowserButtonElement = rootElement->FirstChildElement("BowserButton");
	GUARANTEE_OR_DIE(bowserButtonElement != nullptr, "Palette Widget Config file is missing the \"BowserButton\" element");
	relativeMinsFractionX = ParseXmlAttribute(*bowserButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*bowserButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	buttonSpriteDef = EntityDefinition::GetPaletteSpriteDefinitionForEntity(ENTITY_TYPE_BOWSER);
	ButtonCallbackFunction* callbackBowser = new ButtonCallbackFunction();
	callbackBowser->m_funcPtr = OnClick_SetMouseDataToBowser;
	button = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackBowser, Rgba8::WHITE, buttonSpriteDef, "Bowser"
									, "Boss enemy\nMoves side to side\nShoots fireballs\nHas 3 health");
	button->SetShowText(false);
	m_buttons.push_back(button);

	SpriteSheet* tooltipSpritesheet = g_game->CreateOrGetSpriteSheet(IntVec2(16, 16));
	//BUTTON NAME WIDGET -----------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* buttonNameElement = rootElement->FirstChildElement("ButtonNameWidget");
	GUARANTEE_OR_DIE(buttonNameElement != nullptr, "Palette Widget Config file is missing the \"ButtonNameWidget\" element");
	relativeMinsFractionX = ParseXmlAttribute(*buttonNameElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*buttonNameElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonNameWidgetDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	m_buttonNameWidget = new UIWidget(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs);
	//m_buttonNameWidget->SetColor(Rgba8(0, 0, 0, 200));
	SpriteDefinition const& buttonNameWidgetImage = tooltipSpritesheet->GetSpriteDefinition(IntVec2(0, 5));
	m_buttonNameWidget->SetImage(&buttonNameWidgetImage);
	m_buttonNameWidget->SetTextColor(Rgba8::BLACK);

	//BUTTON TOOL TIP WIDGET -----------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* buttonToolTipElement = rootElement->FirstChildElement("ButtonToolTipWidget");
	GUARANTEE_OR_DIE(buttonToolTipElement != nullptr, "Palette Widget Config file is missing the \"ButtonToolTipWidget\" element");
	relativeMinsFractionX = ParseXmlAttribute(*buttonToolTipElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*buttonToolTipElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + buttonToolTipWidgetDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	m_buttonTooltipWidget = new UIWidget(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs);
	//m_buttonTooltipWidget->SetColor(Rgba8(0, 0, 0, 200));
	SpriteDefinition const& buttonDescWidgetImage = tooltipSpritesheet->GetSpriteDefinition(IntVec2(0, 5));
	m_buttonTooltipWidget->SetImage(&buttonDescWidgetImage);
	m_buttonTooltipWidget->SetTextColor(Rgba8::BLACK);

	//SAVE BUTTON ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XmlElement* saveButtonElement = rootElement->FirstChildElement("SaveButton");
	GUARANTEE_OR_DIE(groundButtonElement != nullptr, "Palette Widget Config file is missing the \"SaveButton\" element");
	Vec2 saveButtonDims = ParseXmlAttribute(*rootElement, "saveButtonDims", Vec2::ZERO);
	m_saveButtonDims = saveButtonDims;
	relativeMinsFractionX = ParseXmlAttribute(*saveButtonElement, "relativeMinsFractionX", 0.0f);
	relativeMinsFractionY = ParseXmlAttribute(*saveButtonElement, "relativeMinsFractionY", 0.0f);
	buttonRelativeMins = Vec2(widgetWidth * relativeMinsFractionX, windowHeight * relativeMinsFractionY);
	buttonRelativeMaxs = buttonRelativeMins + saveButtonDims;
	buttonCamSpaceMins = m_camSpaceMins + buttonRelativeMins;
	buttonCamSpaceMaxs = m_camSpaceMins + buttonRelativeMaxs;
	ButtonCallbackFunction* callbackSaveGame = new ButtonCallbackFunction();
	callbackSaveGame->m_funcPtr = LevelEditor::OnClicked_SaveButton;
	m_saveButton = UIButton::CreateButton(m_renderCam, buttonCamSpaceMins, buttonCamSpaceMaxs, callbackSaveGame, Rgba8::WHITE, nullptr, "Save Level", "saves the level to disk");
	m_saveButton->SetTextColor(Rgba8::GREY);
	SpriteSheet* uiSpriteSheet = g_game->CreateOrGetSpriteSheet(IntVec2(8, 32));
	SpriteDefinition const& saveLevelButtonImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 18));
	m_saveButton->SetImage(&saveLevelButtonImage);
	SpriteDefinition const& saveLevelButtonHoverImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 19));
	m_saveButton->SetHoverImage(&saveLevelButtonHoverImage);
	m_saveButton->SetHoverTint(Rgba8::WHITE);
	SpriteDefinition const& saveLevelButtonClickedImage = uiSpriteSheet->GetSpriteDefinition(IntVec2(0, 20));
	m_saveButton->SetClickedImage(&saveLevelButtonClickedImage);
	m_saveButton->SetClickedTint(Rgba8::WHITE);
}
