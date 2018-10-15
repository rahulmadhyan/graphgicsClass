
Texture2D colorTexture		: register(t0);
Texture2D normalTexture		: register(t1);
SamplerState basicSampler	: register(s0);

cbuffer lightBuffer : register(b0)
{
	float4 lightDiffuseColor;
	float3 lightDirection;
	float colorTextureBrightness;
}

struct VertexToPixel
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float3 binormal	: BINORMAL;
	float4 color	: COLOR;
	float clip		: SC_CLIPDISTANCE;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 lightDir;
	float4 textureColor;
	float4 bumpMap;
	float3 bumpNormal;
	float lightIntensity;
	float4 color;

	// invert light direction
	lightDir = -lightDirection;

	textureColor = colorTexture.Sample(basicSampler, input.uv);

	// combine color map value with texture color
	textureColor = saturate(input.color * textureColor * colorTextureBrightness);

	// calculate bump map using normal map
	bumpMap = normalTexture.Sample(basicSampler, input.uv);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	bumpNormal = input.normal + bumpMap.x * input.tangent + bumpMap.y * input.binormal;
	bumpNormal = normalize(bumpNormal);
	lightIntensity = saturate(dot(bumpNormal, lightDir));

	// calculate bump map pixel color
	color = saturate(lightDiffuseColor * lightIntensity);
	color = color * textureColor;

	return color;
}