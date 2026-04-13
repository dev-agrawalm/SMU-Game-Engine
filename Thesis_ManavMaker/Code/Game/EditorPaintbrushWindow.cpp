#include "Game/EditorPaintbrushWindow.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/VertexUtils.hpp"

EditorPaintbrushWindow::EditorPaintbrushWindow(Vec2 const& screenSpaceMins, Vec2 const& screenSpaceMaxs)
	: m_screenSpaceMins(screenSpaceMins)
	, m_screenSpaceMaxs(screenSpaceMaxs)
{
	float windowWidth = m_screenSpaceMaxs.x - m_screenSpaceMins.x;
	float windowHeight = m_screenSpaceMaxs.y - m_screenSpaceMins.y;

	Vec2 buttonDims = g_gameConfigBlackboard.GetValue("paintBrushButtonDims", Vec2(0.0f, 0.0f));
	Vec2 buttonMins = Vec2(windowWidth * 0.15f, windowHeight * 0.60f);
	Vec2 buttonMaxs = buttonMins + buttonDims;
	AddButton(buttonMins, buttonMaxs, "Ground");

	buttonMins = Vec2(windowWidth * 0.45f, windowHeight * 0.60f);
	buttonMaxs = buttonMins + buttonDims;
	AddButton(buttonMins, buttonMaxs, "Spike");

	buttonMins = Vec2(windowWidth * 0.75f, windowHeight * 0.60f);
	buttonMaxs = buttonMins + buttonDims;
	AddButton(buttonMins, buttonMaxs, "Bridge");

	buttonMins = Vec2(windowWidth * 0.15f, windowHeight * 0.450f);
	buttonMaxs = buttonMins + buttonDims;
	AddButton(buttonMins, buttonMaxs, ENTITY_TYPE_PLAYER);

	buttonMins = Vec2(windowWidth * 0.45f, windowHeight * 0.45f);
	buttonMaxs = buttonMins + buttonDims;
	AddButton(buttonMins, buttonMaxs, ENTITY_TYPE_GOOMBA);

	buttonMins = Vec2(windowWidth * 0.75f, windowHeight * 0.45f);
	buttonMaxs = buttonMins + buttonDims;
	AddButton(buttonMins, buttonMaxs, ENTITY_TYPE_COIN);
}


bool EditorPaintbrushWindow::IsMouseOverPanel()
{
	return false;
}


bool EditorPaintbrushWindow::CheckInput(MouseData& out_mouseData)
{
	UNUSED(out_mouseData);
	return false;
}


void EditorPaintbrushWindow::Render() const
{
	AABB2 windowBounds = AABB2(m_screenSpaceMins, m_screenSpaceMaxs);
	RenderMaterial material = {};
	material.m_color = Rgba8::MAGENTA;
	g_theRenderer->DrawAABB2D(windowBounds, material);

	for (int buttonIndex = 0; buttonIndex < m_buttons.size(); buttonIndex++)
	{
		RenderButton(m_buttons[buttonIndex]);
	}
}


void EditorPaintbrushWindow::AddButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, std::string const& tileDefName)
{
	PaintbrushButton button = PaintbrushButton(relativeMins, relativeMaxs, tileDefName);
	m_buttons.push_back(button);
}


void EditorPaintbrushWindow::AddButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, EntityType type)
{
	PaintbrushButton button = PaintbrushButton(relativeMins, relativeMaxs, type);
	m_buttons.push_back(button);
}


void EditorPaintbrushWindow::RenderButton(PaintbrushButton const& button) const
{
	Vec2 screenSpaceMins = m_screenSpaceMins + button.m_relativeMins;
	Vec2 screenSpaceMaxs = m_screenSpaceMins + button.m_relativeMaxs;
	AABB2 buttonBounds = AABB2(screenSpaceMins, screenSpaceMaxs);

	std::vector<Vertex_PCU> buttonVerts;
	Texture const* texture = nullptr;
	AABB2 uvs = AABB2::ZERO_TO_ONE;
	if (button.m_isEntityButton)
	{
		EntityType type = button.m_entityType;
		if (type != ENTITY_TYPE_NONE)
		{
			SpriteSheet* spriteSheet = GetSpriteSheetForEntity(type);
			texture = &spriteSheet->GetTexture();
			IntVec2 spriteCoords = GetEditorSpriteCoordsForEntity(type);
			uvs = spriteSheet->GetSpriteUVs(spriteCoords);
		}
	}
	else
	{
		TileDefinition const* definition = TileDefinition::GetDefinitionByName(button.m_tileDefName);
		if (definition)
		{
			texture = &definition->GetSpriteSheet()->GetTexture();
			uvs = definition->GetUVs();
		}
	}

	AddVertsForAABB2ToVector(buttonVerts, buttonBounds, Rgba8::WHITE, uvs.m_mins, uvs.m_maxs);
	g_theRenderer->BindTexture(0, texture);
	g_theRenderer->DrawVertexArray((int) buttonVerts.size(), buttonVerts.data());

	static float buttonWidth = g_gameConfigBlackboard.GetValue("paintBrushButtonBorderWidth", 0.0f);
	DebugDrawAABB2Outline(buttonBounds, buttonWidth, Rgba8::CYAN);
}


PaintbrushButton::PaintbrushButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, std::string const& tileDefName)
	: m_relativeMins(relativeMins)
	, m_relativeMaxs(relativeMaxs)
	, m_tileDefName(tileDefName)
	, m_isEntityButton(false)
{

}


PaintbrushButton::PaintbrushButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, EntityType entityType)
	: m_relativeMins(relativeMins)
	, m_relativeMaxs(relativeMaxs)
	, m_entityType(entityType)
	, m_isEntityButton(true)
{

}


