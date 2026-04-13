#include "Game/TriggerManagerWidget.hpp"
#include "Game/UICheckbox.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/EntitySpawnInfo.hpp"
#include "Game/TriggerSystem.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/LevelEditor.hpp"

TriggerManagerWidget::TriggerManagerWidget(LevelEditor* editor, Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs)
	: UIWidget(renderCam, camSpaceMins, camSpaceMaxs)
	, m_editor(editor)
{
	AssembleWidget();
}


void TriggerManagerWidget::Render() const
{
	if (IsHidden())
		return;

	UIWidget::Render();
	m_addOnDamageTrigger->Render();
	m_addOnDeathTrigger->Render();
	m_addOnSpawnTrigger->Render();
}


void TriggerManagerWidget::CheckInput(EntityDefinition* definition)
{
	if (IsHidden())
		return;

	if (definition == nullptr)
		return;

	if (m_addOnSpawnTrigger->IsMouseOverWidget())
	{
		m_addOnSpawnTrigger->SetHoveredOnFrame(g_currentFrameNumber);

		if (g_inputSystem->IsMouseButtonPressed(0))
		{
			m_addOnSpawnTrigger->SetClickedOnFrame(g_currentFrameNumber);
		}

		if (g_inputSystem->WasMouseButtonJustReleased(0))
		{
			m_addOnSpawnTrigger->OnClick();

			if (m_addOnSpawnTrigger->IsChecked())
			{
				EntitySpawnInfo spawnInfo = EntitySpawnInfo(ENTITY_TYPE_GOOMBA);
				//spawnInfo.m_type = ENTITY_TYPE_GOOMBA;
				spawnInfo.m_startingPosition = Vec2(20.0, 30.0f);
				TriggerResponse onSpawnTriggerResponse = {};
				onSpawnTriggerResponse.m_triggerType = TRIGGER_TYPE_ON_SPAWN;
				onSpawnTriggerResponse.m_triggerResponseCommand = new EntitySpawnTriggerCommand(m_editor->GetLevel(), spawnInfo);
				definition->m_properties.AddProperty("onSpawnTriggerResponse", onSpawnTriggerResponse);
			}
			else
			{
				TriggerResponse onSpawnTriggerResponse = {};
				definition->m_properties.AddProperty("onSpawnTriggerResponse", onSpawnTriggerResponse);
			}
		}
	}

	if (m_addOnDeathTrigger->IsMouseOverWidget())
	{
		m_addOnDeathTrigger->SetHoveredOnFrame(g_currentFrameNumber);

		if (g_inputSystem->IsMouseButtonPressed(0))
		{
			m_addOnDeathTrigger->SetClickedOnFrame(g_currentFrameNumber);
		}

		if (g_inputSystem->WasMouseButtonJustReleased(0))
		{
			m_addOnDeathTrigger->OnClick();

			if (m_addOnDeathTrigger->IsChecked())
			{
				EntitySpawnInfo spawnInfo = EntitySpawnInfo(ENTITY_TYPE_GOOMBA);
				//spawnInfo.m_type = ENTITY_TYPE_GOOMBA;
				spawnInfo.m_startingPosition = Vec2(20.0, 30.0f);
				TriggerResponse onSpawnTriggerResponse = {};
				onSpawnTriggerResponse.m_triggerType = TRIGGER_TYPE_ON_DEATH;
				onSpawnTriggerResponse.m_triggerResponseCommand = new EntitySpawnTriggerCommand(m_editor->GetLevel(), spawnInfo);
				definition->m_properties.AddProperty("onDeathTriggerResponse", onSpawnTriggerResponse);
			}
			else
			{
				TriggerResponse onSpawnTriggerResponse = {};
				definition->m_properties.AddProperty("onDeathTriggerResponse", onSpawnTriggerResponse);
			}
		}
	}

	if (m_addOnDamageTrigger->IsMouseOverWidget())
	{
		m_addOnDamageTrigger->SetHoveredOnFrame(g_currentFrameNumber);

		if (g_inputSystem->IsMouseButtonPressed(0))
		{
			m_addOnDamageTrigger->SetClickedOnFrame(g_currentFrameNumber);
		}

		if (g_inputSystem->WasMouseButtonJustReleased(0))
		{
			m_addOnDamageTrigger->OnClick();

			if (m_addOnDamageTrigger->IsChecked())
			{
				EntitySpawnInfo spawnInfo = EntitySpawnInfo(ENTITY_TYPE_GOOMBA);
				//spawnInfo.m_type = ENTITY_TYPE_GOOMBA;
				spawnInfo.m_startingPosition = Vec2(20.0, 30.0f);
				TriggerResponse onSpawnTriggerResponse = {};
				onSpawnTriggerResponse.m_triggerType = TRIGGER_TYPE_ON_DAMAGE;
				onSpawnTriggerResponse.m_triggerResponseCommand = new EntitySpawnTriggerCommand(m_editor->GetLevel(), spawnInfo);
				definition->m_properties.AddProperty("onDamageTriggerResponse", onSpawnTriggerResponse);
			}
			else
			{
				TriggerResponse onSpawnTriggerResponse = {};
				definition->m_properties.AddProperty("onDamageTriggerResponse", onSpawnTriggerResponse);
			}
		}
	}
}


void TriggerManagerWidget::Update(EntityDefinition* definition)
{
	if (definition == nullptr)
		return;

	TriggerResponse onSpawnTriggerResponse = {};
	definition->m_properties.GetProperty("onSpawnTriggerResponse", onSpawnTriggerResponse, onSpawnTriggerResponse);
	if (onSpawnTriggerResponse.m_triggerResponseCommand)
	{
		m_addOnSpawnTrigger->SetCheckedState(true);
	}
	else
	{
		m_addOnSpawnTrigger->SetCheckedState(false);
	}

	TriggerResponse onDeathTriggerResponse = {};
	definition->m_properties.GetProperty("onDeathTriggerResponse", onDeathTriggerResponse, onDeathTriggerResponse);
	if (onDeathTriggerResponse.m_triggerResponseCommand)
	{
		m_addOnDeathTrigger->SetCheckedState(true);
	}
	else
	{
		m_addOnDeathTrigger->SetCheckedState(false);
	}

	TriggerResponse onDamageTriggerResponse = {};
	definition->m_properties.GetProperty("onDamageTriggerResponse", onDamageTriggerResponse, onDamageTriggerResponse);
	if (onDamageTriggerResponse.m_triggerResponseCommand)
	{
		m_addOnDamageTrigger->SetCheckedState(true);
	}
	else
	{
		m_addOnDamageTrigger->SetCheckedState(false);
	}
}


void TriggerManagerWidget::AssembleWidget()
{
	XmlDocument triggerManagerWidgetConfig;
	XmlError result = triggerManagerWidgetConfig.LoadFile("Data/XMLData/TriggerManagerWidgetConfig.xml");
	GUARANTEE_OR_DIE(result == tinyxml2::XMLError::XML_SUCCESS, "Unable to open/find Data/XMLData/TriggerManagerWidgetConfig.xml");

	XmlElement* rootElement = triggerManagerWidgetConfig.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "Config") == 0, "Trigger Manager Widget Config file is missing the root \"Config\" element");

	m_color = ParseXmlAttribute(*rootElement, "color", Rgba8::WHITE);

	Vec2 triggerManagerWidgetDims = GetDimensions();
	XmlElement* onSpawnCheckboxElement = rootElement->FirstChildElement("OnSpawnTriggerCheckbox");
	if (onSpawnCheckboxElement)
	{
		Vec2 spriteWidgetDims = ParseXmlAttribute(*onSpawnCheckboxElement, "widgetDims", Vec2::ZERO);
		float relativeMinsFractionX = ParseXmlAttribute(*onSpawnCheckboxElement, "relativeMinsFractionX", 0.0f);
		float relativeMinsFractionY = ParseXmlAttribute(*onSpawnCheckboxElement, "relativeMinsFractionY", 0.0f);
		Vec2 spriteWidgetRelativeMins = Vec2(triggerManagerWidgetDims.x * relativeMinsFractionX, triggerManagerWidgetDims.y * relativeMinsFractionY);
		Vec2 spriteWidgetMins = m_camSpaceMins + spriteWidgetRelativeMins;
		Vec2 spriteWidgetMaxs = spriteWidgetMins + spriteWidgetDims;
		m_addOnSpawnTrigger = new UICheckbox(m_renderCam, spriteWidgetMins, spriteWidgetMaxs);
		m_addOnSpawnTrigger->SetText("OnSpawn");
	}
	else
	{
		ERROR_RECOVERABLE("Unable to find element \"OnSpawnTriggerCheckbox\" as child to root element");
	}

	XmlElement* onDeathCheckboxElement = rootElement->FirstChildElement("OnDeathTriggerCheckbox");
	if (onDeathCheckboxElement)
	{
		Vec2 spriteWidgetDims = ParseXmlAttribute(*onDeathCheckboxElement, "widgetDims", Vec2::ZERO);
		float relativeMinsFractionX = ParseXmlAttribute(*onDeathCheckboxElement, "relativeMinsFractionX", 0.0f);
		float relativeMinsFractionY = ParseXmlAttribute(*onDeathCheckboxElement, "relativeMinsFractionY", 0.0f);
		Vec2 spriteWidgetRelativeMins = Vec2(triggerManagerWidgetDims.x * relativeMinsFractionX, triggerManagerWidgetDims.y * relativeMinsFractionY);
		Vec2 spriteWidgetMins = m_camSpaceMins + spriteWidgetRelativeMins;
		Vec2 spriteWidgetMaxs = spriteWidgetMins + spriteWidgetDims;
		m_addOnDeathTrigger = new UICheckbox(m_renderCam, spriteWidgetMins, spriteWidgetMaxs);
		m_addOnDeathTrigger->SetText("OnDeath");
	}
	else
	{
		ERROR_RECOVERABLE("Unable to find element \"OnDeathTriggerCheckbox\" as child to root element");
	}

	XmlElement* onDamageCheckboxElement = rootElement->FirstChildElement("OnDamageTriggerCheckbox");
	if (onDamageCheckboxElement)
	{
		Vec2 spriteWidgetDims = ParseXmlAttribute(*onDamageCheckboxElement, "widgetDims", Vec2::ZERO);
		float relativeMinsFractionX = ParseXmlAttribute(*onDamageCheckboxElement, "relativeMinsFractionX", 0.0f);
		float relativeMinsFractionY = ParseXmlAttribute(*onDamageCheckboxElement, "relativeMinsFractionY", 0.0f);
		Vec2 spriteWidgetRelativeMins = Vec2(triggerManagerWidgetDims.x * relativeMinsFractionX, triggerManagerWidgetDims.y * relativeMinsFractionY);
		Vec2 spriteWidgetMins = m_camSpaceMins + spriteWidgetRelativeMins;
		Vec2 spriteWidgetMaxs = spriteWidgetMins + spriteWidgetDims;
		m_addOnDamageTrigger = new UICheckbox(m_renderCam, spriteWidgetMins, spriteWidgetMaxs);
		m_addOnDamageTrigger->SetText("OnDamage");
	}
	else
	{
		ERROR_RECOVERABLE("Unable to find element \"OnDamageTriggerCheckbox\" as child to root element");
	}
}
