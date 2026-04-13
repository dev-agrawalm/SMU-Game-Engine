#pragma once
#include "Game/UIWidget.hpp"

class UICheckbox;
struct EntityDefinition;
class LevelEditor;

class TriggerManagerWidget : public UIWidget
{
public:
	TriggerManagerWidget(LevelEditor* editor, Camera const& renderCam, Vec2 const& camSpaceMins, Vec2 const& camSpaceMaxs);
	void Render() const override;
	void CheckInput(EntityDefinition* definition);
	void Update(EntityDefinition* definition);

private:
	void AssembleWidget();

private:
	LevelEditor* m_editor = nullptr;
	UICheckbox* m_addOnSpawnTrigger = nullptr;
	UICheckbox* m_addOnDeathTrigger = nullptr;
	UICheckbox* m_addOnDamageTrigger = nullptr;
};
