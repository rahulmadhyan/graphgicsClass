
cbuffer matrixBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix reflection;
}

cbuffer cameraNormalBuffer : register(b1)
{
	float3 cameraPosition;
	float padding1;
	float2 normalMapTiling;
	float2 padding2;
}

struct VertexShaderInput
{
	float4 position : POSITION;
	float2 uv		: TEXCOORD0;
};

struct VertexToPixel
{
	float4 position				: SV_POSIITON;
	float4 reflectionPosition	: TEXCOORD0;
	float4 refractionPosition	: TEXCOORD1;
	float3 viewDirection		: TEXCOORD2;
	float2 uv1					: TEXCOORD3;
	float2 uv2					: TEXCOORD4;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	float4 worldPosition;
	matrix reflectionProjectionWorld;
	matrix viewProjectionWorld;

	input.position.w = 1.0f;

	output.position = mul(input.position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);

	reflectionProjectionWorld = mul(reflection, projection);
	reflectionProjectionWorld = mul(world, reflectionProjectionWorld);

	output.reflectionPosition = mul(input.position, reflectProjectionWorld);

	viewProjectionWorld = mul(view, projection);
	viewProjectionWorld = mul(world, viewProjectionWorld);

	output.refractionPosition = mul(input.position, viewProjectionWorld);

	// calculate the camera's view direction for fresnel and specular calculations
	worldPosition = mul(input.position, world);

	output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
	output.viewDirection = normalize(output.viewDirection);

	// calculate two different tiling texture coordinates for water normal map

	output.uv1 = input.uv / normalMapTiling.x;
	output.uv2 = input.uv / normalMapTiling.y;

	return output;
}