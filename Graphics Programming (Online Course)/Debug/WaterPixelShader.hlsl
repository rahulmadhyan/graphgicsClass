
Texture2D refractionTexture : register(t0);
Texture2D reflectionTexture : register(t1);
Texture2D normalTexture		: register(t2);
SamplerState basicSampler	: register(s0);

cbuffer waterBuffer
{
	float4 refractionTint;
	float3 lightDirection;
	float waterTranslation;
	float reflectRefractScale;
	float specularShine;
	float2 padding;
};

struct VertexToPixel
{
	float4 position				: SV_POSITION;
	float4 reflectionPosition	: TEXCOORD0;
	float4 refractionPosition	: TEXCOORD1;
	float3 viewDirection		: TEXCOORD2;
	float2 uv1					: TEXCOORD3;
	float2 uv2					: TEXCOORD4;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 normalMap1;
	float4 normalMap2;
	float3 normal1;
	float3 normal2;
	float3 normal;
	float2 refractionUV;
	float2 reflectionUV;
	float4 reflectionColor;
	float4 refractionColor;
	float3 heightView;
	float r;
	float fresnelFactor;
	float4 color;
	float3 reflection;
	float specular;

	// translate the two textuer coordinates by the water translation amount
	input.uv1.y += waterTranslation;
	input.uv2.y += waterTranslation;

	// sample the normal from the normal map texture using the two different tiled and translated coordinates
	normalMap1 = normalTexture.Sample(basicSampler, input.uv1);
	normalMap2 = normalTexture.Sample(basicSampler, input.uv2);

	// range from (0, 1) to (-1, +1)
	normal1 = (normalMap1.xyz * 2.0f) - 1.0f;
	normal2 = (normalMap2.xyz * 2.0f) - 1.0f;

	// combine normal map results to get an animated water ripple effect
	normal = normalize(normal1 + normal2);

	// calculate projected refraction texture coordinates
	refractionUV.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
	refractionUV.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

	// calcualte projected reflection texture coodinates
	reflectionUV.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
	reflectionUV.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;

	// repostion the texture coordinates sampling position by the scaled normal map value to simulate the rippling wave effect
	reflectionUV = reflectionUV + (normal.xy * reflectRefractScale);
	refractionUV = refractionUV + (normal.xy * reflectRefractScale);

	// sample texture pixels from texture using the updated texture coordinates
	reflectionColor = reflectionTexture.Sample(basicSampler, reflectionUV);
	refractionColor = refractionTexture.Sample(basicSampler, refractionUV);

	// add water color tint to refraction
	refractionColor = saturate(refractionColor * refractionTint);

	// height based vector for fresnel calculation
	heightView.x = input.viewDirection.y;
	heightView.y = input.viewDirection.y;
	heightView.z = input.viewDirection.y;

	// calculate fresnel term based on height
	r = (1.2f - 1.0f) / (1.2f + 1.0f);
	fresnelFactor = max(0.0f, min(1.0f, r + (1.0f - r) * pow(1.0f - dot(normal, heightView), 2)));

	//combine reflection and refraction results for the final color
	color = lerp(reflectionColor, refractionColor, fresnelFactor);

	// calculate the reflection vector using the normal and the direction of the light
	reflection = -reflect(normalize(lightDirection), normal);

	// calcualte specular light based on the reflection and the camera position
	specular = dot(normalize(reflection), normalize(input.viewDirection));

	if (specular > 0.0f)
	{
		specular = pow(specular, specularShine);

		color = saturate(color + specular);
	}

	return color;
}