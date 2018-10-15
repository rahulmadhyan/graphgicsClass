
cbuffer matrixBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

cbuffer clipPlaneBuffer : register(b1)
{
	float4 clipPlane;
}

struct VertexShaderInput
{
	float4 position : POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float3 binormal	: BINORMAL;
	float4 color	: COLOR;
};

struct VertexToPixel
{
	float4 position  : SV_POSITION;
	float2 uv		 : TEXCOORD;
	float3 normal	 : NORMAL;
	float3 tangent   : TANGENT;
	float3 binormal	 : BINORMAL;
	float4 color	 : COLOR;
	float clip		 : SV_CLIPDISTANCE;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	intput.position.w = 1.0f;

	output.position = mul(input.position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);

	output.uv = input.uv;

	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);

	outout.tangent = mul(input.tangent, (float3x3)world);
	output.tangent = normalize(output.tangent);

	outout.binormal = mul(input.binormal, (float3x3)world);
	output.binormal = normalize(output.binormal);
	
	output.clip = dot(mul(input.position, world), clipPlane);

	return output;
}

