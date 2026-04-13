//##
//## Disclaimer: The HsvToRgb and RgbToHsv functions are borrowed from stack overflow and is not my own math
//##https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both - link to the page from where I got it


struct VertexShaderInput
{
	float3 localPosition	: POSITION;
	float4 color			: TINT;
	float2 texCoords		: TEXCOORD;
};


struct VertexShaderOutput
{
	float4 clipPosition		: SV_POSITION;
	float4 color			: TINT;
	float2 texCoords		: TEXCOORD;
};


cbuffer CAMERA_CONSTANTS : register(b2)
{
	float4x4 PROJ_MAT;
	float4x4 VIEW_MAT;
};


cbuffer MODEL_CONSTANTS : register(b3)
{
	float4x4 MODEL_MAT;
	float4 MODEL_TINT;
};


cbuffer GAME_CONSTANTS : register(b4)
{
	float4x4 COLOR_TRANSFORMATION_MATRIX;
	float GAUSSIAN_BLUR_NORMALIZED_RADIUS;
	float GUASSIAN_BLUR_PER_STEP_DISTANCE;
	int GAUSSIAN_BLUR_NUM_OUTWARD_STEPS;
	float GAUSSIAN_BLUR_THETA_DEVIATION_PER_STEP;
	float GUASSIAN_BLUR_THETA_DEVIATION_WITHIN_STEP;
	float TIME;
//	int padding0;
	int padding1;
	int padding2;
}


VertexShaderOutput VertexMain(VertexShaderInput vs_input)
{
	float4 localPos = float4(vs_input.localPosition, 1.0f);
	float4 worldPos = mul(MODEL_MAT, localPos);
	float4 screenPos = mul(VIEW_MAT, worldPos);
	float4 clipPos = mul(PROJ_MAT, screenPos);

	VertexShaderOutput vs_output;
	vs_output.clipPosition = clipPos;
	vs_output.color = vs_input.color * MODEL_TINT;
	vs_output.texCoords = vs_input.texCoords;
	return vs_output;
}

Texture2D<float4> DiffuseTexture : register(t0);
Texture2D<float4> NoiseTexture : register(t1);
SamplerState DiffuseSampler : register(s0);

float3 TransformRGB(float3 rgb, float4x4 transformMatrix)
{
	float4 rgbVec = float4(rgb.xyz, 1);
	float4 transformedRgbVec = mul(rgbVec, transformMatrix);
	return transformedRgbVec.xyz;
}


float4 PixelMain(VertexShaderOutput ps_input) : SV_Target0
{
	float2 inputTexCoords = ps_input.texCoords;

	float totalWeight = 0.0f;
	float4 weightedColor = {0.0f, 0.0f, 0.0f, 0.0f};
	for (int stepIndex = 0; stepIndex < GAUSSIAN_BLUR_NUM_OUTWARD_STEPS; stepIndex++)
	{
		float startingTheta = GAUSSIAN_BLUR_THETA_DEVIATION_PER_STEP * stepIndex;
		float theta = startingTheta;
		float stepDistance = (stepIndex + 1) * GUASSIAN_BLUR_PER_STEP_DISTANCE;
		while ((theta - 360.0f) <= startingTheta)
		{
			float2 sampleOffset = stepDistance * float2(cos(theta), sin(theta));
			float2 sampleTexCoords = inputTexCoords + sampleOffset;
			float4 sampleDiffuseColor = DiffuseTexture.Sample(DiffuseSampler, sampleTexCoords);
			weightedColor += sampleDiffuseColor;
			totalWeight++;
			theta += GUASSIAN_BLUR_THETA_DEVIATION_WITHIN_STEP;
		}
	}

	float4 finalColor = weightedColor / totalWeight;
	float4 tint = ps_input.color;
	finalColor *= tint;
	finalColor.xyz = TransformRGB(finalColor.xyz, COLOR_TRANSFORMATION_MATRIX);

	float2 noiseTexCoords = inputTexCoords + 0.01f * TIME;
	float4 noise = NoiseTexture.Sample(DiffuseSampler, noiseTexCoords);
	noise *= 1.5f;
	finalColor *= noise;

	return finalColor;
}
