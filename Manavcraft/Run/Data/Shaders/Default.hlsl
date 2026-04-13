//Data that the vertex shader will receive per vertex
struct VertexShaderInput
{
	float3 vertexPosition : POSITION;   //position of the vertex
	float4 color		  : TINT;		//color of the vertex
	float2 uv			  : TEXCOORD;	//uv of the vertex
};

//Data that the pixel shader will receive per pixel
//Do not assume that the data that you passed to this struct in the vertex shader stage will be the same values for each pixel in the pixel shader
//The values would only be same for the pixels that coincide with the vertexes themselves but there will be more pixels in each triangle
struct VertexToPixelData 
{
	float4 clipPosition	: SV_Position; //clip position of the pixel
	float4 color		: VertexColor; //color of the pixel
	float2 uv			: VertexUV;	   //uv of the pixel
};


cbuffer CAMERA_CONSTANTS : register(b2) //camera constants that the engine requires // reserved for slot 2
{
	float4x4 PROJ_MAT;
	float4x4 VIEW_MAT;
}


//constants for the model to which the vertex belong 
//would contain the local to world conversion matrix for the vertex positions
// reserved for slot 3
cbuffer MODEL_CONSTANTS : register(b3) 
{
	float4x4 MODEL_MAT;
	float4 TINT;
}


//------------------------------------------------------------------------------------------------
// Main Entry Point for the vertex stage
VertexToPixelData VertexMain(VertexShaderInput input)
{
	float4 inputPosition = {input.vertexPosition.x, input.vertexPosition.y, input.vertexPosition.z, 1.0f};
	float4 worldPosition = mul(MODEL_MAT, inputPosition);
	float4 cameraPosition = mul(VIEW_MAT, worldPosition);
	float4 clipPosition = mul(PROJ_MAT, cameraPosition);

	VertexToPixelData vertexToPixelData;
	vertexToPixelData.color		= input.color * TINT;
	vertexToPixelData.uv		= input.uv;
	vertexToPixelData.clipPos	= clipPosition;
	return vertexToPixelData; // pass it on to the raster stage
}


Texture2D<float4> SurfaceTexture : register(t0);
SamplerState SurfaceSampler : register(s0);

//------------------------------------------------------------------------------------------------
// Main Entry Point for the Pixel Stage
float4 PixelMain(VertexToPixelData input) : SV_Target0
{
	float2 texCoords = input.uv;
	float4 surfaceColor = SurfaceTexture.Sample(SurfaceSampler, texCoords);

	float4 tint = input.color;
	float4 finalColor = tint * surfaceColor;
	return finalColor;
}