#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

#include <string>

class Camera; 
class Renderer; 

/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

// Only effects "world" mode stuff; 
enum class DebugRenderMode
{
   ALWAYS,          // what is rendered always shows up (newer objects usually in front of older objects)
   USE_DEPTH,       // respect the depth buffer
   XRAY,
}; 


//------------------------------------------------------------------------
struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr; 
	bool m_startHidden = false; 
	std::string m_fontName = "Data/Fonts/SquirrelFixedFont";
}; 

/************************************************************************/
/*                                                                      */
/* FUNCTION PROTOTYPES                                                  */
/*                                                                      */
/************************************************************************/
//------------------------------------------------------------------------
//  System
//------------------------------------------------------------------------
// setup
void DebugRenderSystemStartup( DebugRenderConfig const& config );    // setup the debug render system (call from Renderer::Startup)
void DebugRenderSystemShutdown();   // cleans up the system to prevent leaks (call from Renderer::Shutdown)

// control
void DebugRenderSetVisible();		// enables the debug render system 
void DebugRenderSetHidden();		// disable the debug render system
void DebugRenderToggleVisibility();
void DebugRenderClear();			// clears all current debug render instructions
void DebugRenderSetTimeScale(double scale);

// output
void DebugRenderBeginFrame();								// ...handle any pre-frame needs of the system, call from `Renderer::BeginFrame`
void DebugRenderWorldToCamera( Camera const& camera );		// Draws all world objects to this camera 
void DebugRenderScreenToCamera( Camera const& camera );		// Draws all screen objects onto this texture (screen coordinate system is up to you.  I like a 1080p default)
void DebugRenderEndFrame();									// ...handle any post frame needs of the system, call from `Renderer::EndFrame`

//------------------------------------------------------------------------
// World Rendering
//------------------------------------------------------------------------
// points
void DebugAddWorldPoint( Vec3 const& pos, float size, Rgba8 const& startColor, Rgba8 const& endColor, float duration, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldPoint( Vec3 const& pos, float size, Rgba8 const& color, float duration = 0.0f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldPoint( Vec3 const& pos, Rgba8 const& color, float duration = 0.0f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );

// lines
void DebugAddWorldLine(Vec3 const& start, Vec3 const& end,
					   Rgba8 const& startColor, Rgba8 const& endColor,
					   float thickness,
					   float duration,
					   DebugRenderMode mode);
void DebugAddWorldLine( Vec3 const& start, Vec3 const& end, Rgba8 const& color = Rgba8::WHITE, float thickness = 0.125f, float duration = 0.0f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );

// cylinder
void DebugAddWorldWireCylinder(Vec3 const& base,
							   Vec3 const& top,
							   float radius,
							   float duration,
							   Rgba8 const& startColor,
							   Rgba8 const& endColor,
							   DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireCylinder(Vec3 const& base,
							   Vec3 const& top,
							   float radius = 1.0f,
							   float duration = 0.0f,
							   Rgba8 const& color = Rgba8::WHITE,
							   DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

// AABB3
void DebugAddWorldWireAABB3(AABB3 const& aabb3,
							float duration,
							Rgba8 const& startColor,
							Rgba8 const& endColor,
							DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireAABB3(AABB3 const& aabb3,
							float duration = 0.0f,
							Rgba8 const& color = Rgba8::WHITE,
							DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

// sphere
void DebugAddWorldWireSphere(Vec3 const& center,
							 float radius,
							 float duration,
							 Rgba8 const& startColor,
							 Rgba8 const& endColor,
							 DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireSphere(Vec3 const& center,
							 float radius,
							 float duration = 0.0f,
							 Rgba8 const& color = Rgba8::WHITE,
							 DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldSolidSphere(Vec3 const& center,
							 float radius,
							 float duration,
							 Rgba8 const& startColor,
							 Rgba8 const& endColor,
							 DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldSolidSphere(Vec3 const& center,
							 float radius,
							 float duration = 0.0f,
							 Rgba8 const& color = Rgba8::WHITE,
							 DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

// arrows
void DebugAddWorldArrow( Vec3 const& start, Vec3 const& end, Rgba8 const& startColor, Rgba8 const& endColor, float duration, float thickness, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldArrow( Vec3 const& start, Vec3 const& end,  float duration = 0.0f, Rgba8 const& color = Rgba8::WHITE, float thickness = 0.125f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );

// basis
void DebugAddWorldBasis( Mat44 const& basis, Rgba8 const& startTint, Rgba8 const& endTint, float duration, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldBasis( Mat44 const& basis, float duration = 0.0f, Rgba8 const& tint = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );

// text
// non-billboarded will be oriented in the world based on the passed in basis matrix 
void DebugAddWorldText( std::string const& text, float textHeight, Vec2 const& alignment, Rgba8 const& startColor, Rgba8 const& endColor, Mat44 const& transform = Mat44(), float duration = 0.0f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldText( std::string const& text, float textHeight = 0.25f, Vec2 const& alignment = Vec2(0.5f, 0.5f), Rgba8 const& color = Rgba8::WHITE, Mat44 const& transform = Mat44(), float duration = 0.0f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );

// this text will always orient itself to the current camera
// Origin - refers to the center of the text
void DebugAddWorldBillboardText( Vec3 const& origin, std::string const& text, float textHeight, Vec2 const& alignment, Rgba8 const& startcolor, Rgba8 const& endColor, float duration = 0.0f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );
void DebugAddWorldBillboardText( Vec3 const& origin, std::string const& text, float textHeight = 0.5f, Vec2 const& alignment = Vec2(0.5f, 0.5f), Rgba8 const& color = Rgba8::WHITE, float duration = 0.0f, DebugRenderMode mode = DebugRenderMode::USE_DEPTH );


//------------------------------------------------------------------------
// Screen Rendering
//------------------------------------------------------------------------

// text - put anywhere on the screen.  Useful for debugging values.
// position - refers to the position of the bottom left corner of the text
void DebugAddScreenText( std::string const& text, Vec2 const& position, float duration, Vec2 const& alignment, float textHeight, Rgba8 const& startColor, Rgba8 const& endColor );
void DebugAddScreenText( std::string const& text, Vec2 const& position, float duration = 0.0f, Vec2 const& alignment = Vec2(0.0f, 0.0f), float textHeight = 32.0f, Rgba8 const& color = Rgba8::WHITE );

// messages - messages display after text, at the top of the screen, most recent on top.  
void DebugAddScreenMessage( std::string const& text, float duration, float textHeight, Rgba8 const& startColor, Rgba8 const& endColor );
void DebugAddScreenMessage(std::string const& text, float duration = 0.0f, float textHeight = 32.0f, Rgba8 const& color = Rgba8::WHITE);
