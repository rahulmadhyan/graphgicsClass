
Texture2D grassTexture : register(t0);
Texture2D slopeTexture : register(t1);
Texture2D rockTexture  : register(t2);

SamplerState SampleType;

cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD;
	float3 normal : NORMAL;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 grassColor;
	float4 slopeColor;
	float4 rockColor;
	float slope;
	float blendAmount;
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;

	// Sample the grass color from the texture using the sampler at this texture coordinate location.
	grassColor = grassTexture.Sample(SampleType, input.tex);

	// Sample the slope color from the texture using the sampler at this texture coordinate location.
	slopeColor = slopeTexture.Sample(SampleType, input.tex);

	// Sample the rock color from the texture using the sampler at this texture coordinate location.
	rockColor = rockTexture.Sample(SampleType, input.tex);

	float3 blending = abs(input.normal);
	// force weights to sum to 1.0
	float b = blending.x + blending.y + blending.z;
	blending /= float3(b, b, b);

	float4 x = slopeTexture.Sample(SampleType,  input.tex).xyzw;
	float4 y = rockTexture.Sample(SampleType, input.tex).xyzw;
	float4 z = grassTexture.Sample(SampleType, input.tex).xyzw;

	float4 tColor = x * blending.x + y * blending.y + z * blending.z;
	textureColor = tColor;

	// Invert the light direction for calculations.
	lightDir = -lightDirection;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightDir));

	// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
	color = diffuseColor * lightIntensity;

	// Saturate the final light color.
	color = saturate(color);

	// Multiply the texture color and the final light color to get the result.
	color = color * textureColor;

	return color;
}