struct vs_input_t
{
	float3 position : POSITION;
	float4 color : TINT;
	float2 uv : TEXCOORD;
};

struct v2p_t // vertex to fragment
{
	float4 position : SV_Position;
	float4 color : VertexColor;
	float2 uv		: VertexUV;
};


cbuffer CAMERA_CONSTANTS : register(b2)
{
	float4x4 PROJ_MAT;
	float4x4 VIEW_MAT;
}


cbuffer MODEL_CONSTANTS : register(b3)
{
	float4x4 MODEL_MAT;
	float4 TINT;
}


//------------------------------------------------------------------------------------------------
// Main Entry Point for the vertex stage
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2f;

	float4 inputPosition = {input.position.x, input.position.y, input.position.z, 1.0f};
	float4 worldPosition = mul(MODEL_MAT, inputPosition);
	float4 cameraPosition = mul(VIEW_MAT, worldPosition);
	float4 clipPosition = mul(PROJ_MAT, cameraPosition);

	v2f.color = input.color * TINT;
	v2f.uv = input.uv;
	v2f.position = clipPosition;
	return v2f; // pass it on to the raster stage
}


Texture2D<float4> SurfaceTexture : register(t0);
SamplerState SurfaceSampler : register(s0);

//------------------------------------------------------------------------------------------------
// Main Entry Point for the Pixel Stage
float4 PixelMain(v2p_t input) : SV_Target0
{
	float2 texCoords = input.uv;
	float4 surfaceColor = SurfaceTexture.Sample(SurfaceSampler, texCoords);

	float4 tint = input.color;
	float4 finalColor = tint * surfaceColor;
	return finalColor;
}