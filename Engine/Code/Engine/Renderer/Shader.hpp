#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>

// predefines so this compiles
struct ID3D11PixelShader; 
struct ID3D11InputLayout;
struct ID3D11VertexShader; 

class Renderer;

//------------------------------------------------------------------------------------------------
struct ShaderConfig
{
	std::string m_name;								// for now, file path without an extension 
	std::string m_sourceCode = "NULL";

	std::string m_vertexEntryPoint	= "VertexMain"; // which function in the source file we use as the entry point for the vertex stage
	std::string m_pixelEntryPoint	= "PixelMain";	// which function in the source file we use as the entry point for the pixel stage
};

//------------------------------------------------------------------------------------------------
class Shader
{
	friend class Renderer; // Only the Renderer can create new Texture objects!

public:
	bool IsValid() const; 
	std::string const& GetName() const; 


private:
	Shader(); // can't instantiate directly; must ask Renderer to do it for you
	Shader( Shader const& copy ) = delete; // No copying allowed! 
	~Shader();

	// Creating and Destroying a Shader
	bool Create( Renderer* renderer, ShaderConfig const& config );
	void Destroy();
	bool CompileAndCreateVertexShader(Renderer* renderer, std::string const sourceCode);
	bool CompileAndCreatePixelShader(Renderer* renderer, std::string const sourceCode);
	ID3D11InputLayout* CreateOrGetInputLayout_VertexPCU(Renderer* renderer);


protected:
	ShaderConfig m_config;								// options for this shader

	ID3D11VertexShader*	m_vertexStage	= nullptr;		// Vertex Shader Compiled Object
	ID3D11PixelShader* m_pixelStage		= nullptr;		// Pixel Shader Compiled Object

	std::vector<uint8_t> m_vertexByteCode;				// used in future assignments...	
	ID3D11InputLayout* m_inputLayout_VertexPCU = nullptr;
};


