
// Constant Buffer
// - Allows us to define a buffer of individual variables 
//    which will (eventually) hold data from our C++ code
// - All non-pipeline variables that get their values from 
//    our C++ code must be defined inside a Constant Buffer
// - The name of the cbuffer itself is unimportant
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;       // Normal
	float2 uv			: TEXCOORD;		// UV coordinates
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view and projection martrices.
	output.position = mul(input.position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);

	// Store the texture coordinates for the pixel shader
	output.uv = input.uv;

	// Calculate the normal vector against the wold matrix only
	output.normal = mul(input.normal, (float3x3)world);

	// Normalize the normal vector
	output.normal = normalize(output.normal);

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}