cbuffer matrixBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 position : POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
};

struct VertexToPixel
{
	float4 position  : SV_POSITION;
	float3 positionW : POSITION;
	float2 uv		 : TEXCOORD;
	float3 normal	 : NORMAL;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	output.positionW = mul(input.position, (float3x3)world);
	output.position = mul(float4(input.position, 1.0f), world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);

	output.uv = input.uv;

	output.normal = mul(input.normal, (float3x3)world);

	return output;
}