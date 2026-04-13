#pragma once
#include <string>
#include "Game/Entity.hpp"
#include <vector>
#include "Game/MouseData.hpp"

struct PaintbrushButton
{
public:
	PaintbrushButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, std::string const& tileDefName);
	PaintbrushButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, EntityType entityType);

public:
	bool m_isEntityButton = false;
	Vec2 m_relativeMins;
	Vec2 m_relativeMaxs;
	std::string m_tileDefName;
	EntityType m_entityType = ENTITY_TYPE_NONE;
};


class EditorPaintbrushWindow
{
public:
	EditorPaintbrushWindow(Vec2 const& screenSpaceMins, Vec2 const& screenSpaceMaxs);
	bool IsMouseOverPanel();
	bool CheckInput(MouseData& out_mouseData);
	void Render() const;

	void AddButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, std::string const& tileDefName);
	void AddButton(Vec2 const& relativeMins, Vec2 const& relativeMaxs, EntityType type);

private:
	void RenderButton(PaintbrushButton const& button) const;

private:
	std::vector<PaintbrushButton> m_buttons;
	Vec2 m_screenSpaceMins;
	Vec2 m_screenSpaceMaxs;
};
