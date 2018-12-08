
Texture2D grassTexture		: register(t0);
Texture2D slopeTexture		: register(t1);
Texture2D rockTexture		: register(t2);
Texture2D normalTexture		: register(t3);
SamplerState SampleType;

cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float4 fogColor;
	float3 cameraPosition;
	float fogStart;
	float3 lightDirection;
	float fogRange;
};

struct PixelInputType
{
	float4 position		: SV_POSITION;
	float3 positionW	: POSITION;
	float2 tex			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;

	float3 blending = abs((input.normal));
	// force weights to sum to 1.0
	float b = blending.x + blending.y + blending.z;
	blending /= float3(b, b, b);

	float4 x = slopeTexture.Sample(SampleType,  input.tex).xyzw;
	float4 y = rockTexture.Sample(SampleType, input.tex).xyzw;
	float4 z = grassTexture.Sample(SampleType, input.tex).xyzw;

	textureColor = x * blending.x + y * blending.y + z * blending.z;

	float3 normalFromMap = normalTexture.Sample(SampleType, input.tex).xyz * 2 - 1;

	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));

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

	// Vector from point being lit to eye. 
	float3 toCamera = cameraPosition - input.positionW;
	float distanceToCamera = length(toCamera);

#if FOG
	float fogAmount = saturate((distanceToCamera - fogStart) / fogRange);
	color = lerp(color, fogColor, fogAmount);
#endif

	return color;
}