#pragma once
#include "Game/Entity.hpp"







//
//
//
//
//			UNUSED CLASS
//
//
//
//









class LevelEndFlag : public Entity
{

public:
// 	LevelEndFlag(Level* level, Vec2 const& position);
// 	void Update(float deltaSeconds) override;
// 	void Render() const override;
	void StartLevelEndAnimation();

private:
	float m_levelEndAnimDuration = 0.0f;
	float m_levelEndAnimStartTime = -1.0f;
	float m_levelEndAnimTimer = 0.0f;
};