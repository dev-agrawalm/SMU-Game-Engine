// a struct containing what inputs our vertex shader is expecting
struct vs_input_t
{
	float3 position : POSITION;
	float4 color	: TINT; 
	float2 uv		: TEXCOORD; 
};

// struct of data that I'm passing from the vertex stage for the fragment stage.  `v2f_t` is the name used in Unity for this, 
// but you can call it whatever you want, such as `VertexToPixel` or `VertexToFragment` if you prefer a different naming scheme
struct v2p_t // vertex to fragment
{
	float4 position : SV_Position; // SV_Position is always requried for a vertex shader, and denotes the renering location of this vertex.  As an input to the pixel shader, it gives the pixel location of the fragment
	float4 color	: VertexColor;    // This semantic is NOT `SV_`, and is just whatever name we want to call it.  If a pixel stage has an input marked as `VertexColor`, it will link the two up.
	float2 uv		: VertexUV;
};


cbuffer CAMERA_CONSTANTS : register(b2)
{
	float4x4 ORTHO_PROJ_MAT;
}


//------------------------------------------------------------------------------------------------
// Main Entry Point for the vertex stage
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2f;

	float4 inputPosition = {input.position.x, input.position.y, input.position.z, 1.0f};
	v2f.position = mul(ORTHO_PROJ_MAT, inputPosition);
	v2f.color = input.color;
	v2f.uv = input.uv;
	return v2f; // pass it on to the raster stage
}



Texture2D<float4> SurfaceTexture : register(t0);
SamplerState SurfaceSampler : register(s0);

//------------------------------------------------------------------------------------------------
// Main Entry Point for the Pixel Stage
// This returns only one value (the output color)
// so instead of using a struct, we'll just label the return value
//
// This determines the color of a single pixel or fragment in the output
// the input may vary certain variables in the raster stage, and we'll get those varied 
// inputs passed to us. 
//
// Note, system variables such as `SV_Position` have special rules, and the one output may
// have no or very little relation to the one you got into the pixel shader; 
float4 PixelMain(v2p_t input) : SV_Target0
{
	float2 texCoords = input.uv;
	float4 surfaceColor = SurfaceTexture.Sample(SurfaceSampler, texCoords);
	
	float4 tint = input.color;
	float4 finalColor = tint * surfaceColor;
	return finalColor; 
}
