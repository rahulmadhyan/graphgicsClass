
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

cbuffer ExternalData : register(b0)
{
	DirectionalLight light;
	DirectionalLight light1;
};

Texture2D diffuseTexture	: register(t0);
SamplerState basicSampler	: register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(1,0,0,1);
	//return float4(input.normal, 1);
	float3 normalizedDirection = normalize(light.Direction);
	float lightAmount = saturate(dot(input.normal, normalizedDirection));

	float3 normalizedDirection1 = normalize(light1.Direction);
	float lightAmount1 = saturate(dot(input.normal, normalizedDirection1));

	float4 textureColor = diffuseTexture.Sample(basicSampler, input.uv);

	return(((light.DiffuseColor * lightAmount) + light.AmbientColor) + 
		((light1.DiffuseColor * lightAmount1) + light1.AmbientColor)) +
		textureColor;
}