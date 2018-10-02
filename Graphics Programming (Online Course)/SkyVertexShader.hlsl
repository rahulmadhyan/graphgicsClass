
// constant buffer 
cbuffer externalData : register(b0)
{
	//matrix world;
	matrix view;
	matrix projection;
}

struct VertexShaderInput
{
	float3 position : POSITION;
};

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float3 uvw : TEXCOORD;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	// view matrix with zero translation
	matrix viewNoMovement = view;
	viewNoMovement._41 = 0;
	viewNoMovement._42 = 0;
	viewNoMovement._43 = 0;

	matrix viewProj = mul(viewNoMovement, projection);
	output.position = mul(float4(input.position, 1.0f), viewProj);

	// make polygons to max depth
	output.position.z = output.position.w;

	// Use the cube's vertex position as a direction in space
	// from the origin (center of the cube)
	output.uvw = input.position;

	return output;
}