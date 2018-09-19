
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
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float3 SampleDetailTriplanar(float3 uvw, float3 N, float index)
{
	float tighten = 0.4679f;
	float3 blending = saturate(abs(N) - tighten);
	// force weights to sum to 1.0
	float b = blending.x + blending.y + blending.z;
	blending /= float3(b, b, b);

	float3 x = grassTexture.Sample(SampleType, uvw.yz).xyz;
	float3 y = slopeTexture.Sample(SampleType, uvw.xz).xyz;
	float3 z = rockTexture.Sample(SampleType, uvw.xy).xyz;

	return x * blending.x + y * blending.y + z * blending.z;
}

float3 GetTexByHeightPlanar(float height, float3 uvw, float3 N, float index1, float index2)
{
	float bounds = 1 * 0.005f;
	float transition = 1 * 0.6f;
	float blendStart = transition - bounds;
	float blendEnd = transition + bounds;
	float3 c;

	if (height < blendStart) {
		c = SampleDetailTriplanar(uvw, N, index1);
	}
	
	else {
		c = SampleDetailTriplanar(uvw, N, index2);
	}

	return c;
}

float3 GetTexBySlope(float slope, float height, float3 N, float3 uvw, float startingIndex)
{
	float3 c;
	
	c = SampleDetailTriplanar(uvw, N, 3 + startingIndex);
	
	return c.rgb;
}

float4 dist_based_texturing(float height, float slope, float3 N, float3 uvw)
{

	return float4(GetTexBySlope(slope, height, N, uvw, 4), 1);
}

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

	// Calculate the slope of this point.
	slope = 1.0f - input.normal.y;

	//// Determine which texture to use based on height.
	//if (slope < 0.2)
	//{
	//	blendAmount = slope / 0.2f;
	//	textureColor = lerp(grassColor, slopeColor, blendAmount);
	//}

	//if ((slope < 0.7) && (slope >= 0.2f))
	//{
	//	blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
	//	textureColor = lerp(slopeColor, rockColor, blendAmount);
	//}

	//if (slope >= 0.7)
	//{
	//	textureColor = rockColor;
	//}

	float3 tColor = dist_based_texturing(input.position.z, acos(input.normal.z), input.normal, input.position / 2);
	textureColor = float4(tColor.x, tColor.y, tColor.z, 1);

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