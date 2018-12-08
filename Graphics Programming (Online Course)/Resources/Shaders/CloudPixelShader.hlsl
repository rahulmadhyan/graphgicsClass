
#include "CloudShaderHelper.hlsl"

Texture3D densityTexture		: register(t0);
Texture3D fadeTexture			: register(t1);
SamplerState basicSampler		: register(s0);

cbuffer cloudBuffer : register(b1)
{
	float4 fogColor;
	float3 cameraPosition;
	float deltaTime;
	float3 AABBMin;
	float volumeSamplingScale;
	float3 AABBMax;
	float densityScale;
	float densityBias;
	float densityCutoff;
	float shadowSampleOffset;
	int rayMarchSamples;
	int maxRayMarchSamples;
	int shadowSampleCount;
	int shadowScale;
}

struct VertexToPixel
{
	float4 position  : SV_POSITION;
	float4 positionW : POSITION;
	float2 uv		 : TEXCOORD;
	float3 normal	 : NORMAL;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	//float3 totalColor = densityTexture.Sample(basicSampler, float3(input.uv, 0.0f));

	// view direction
	float3 dir = normalize(input.positionW.xyz - cameraPosition);

	float nearHit;
	float farHit;

	RayAABBIntersection(cameraPosition, dir, AABBMin, AABBMax, nearHit, farHit);

	// inside cube check 
	nearHit = max(nearHit, 0.0f);

	// ray start and end positions
	float3 rayStartWorld = cameraPosition + dir * nearHit;
	float3 rayEndWorld = cameraPosition + dir * farHit;

	// remap ray's start and end - ensure we don't hit the very edge (0 or 1)
	float3 rayStart = WorldToAABB(rayStartWorld, AABBMin, AABBMax);
	float3 rayEnd = WorldToAABB(rayEndWorld, AABBMin, AABBMax);

	// setup current position and step amount
	float3 currentPosition = rayStart;

#if UNIFORM_SAMPLE_STEP
	float stepLarge = sqrt(3) / maxRayMarchSamples;
#else
	float rayLength = length(rayStart - rayEnd);
	float stepLarge = rayLength / maxRayMarchSamples;
#endif

	float stepSmall = stepLarge * 0.25f;
	float3 stepDirLarge = stepLarge * dir;
	float3 stepDirSmall = stepSmall * dir;

	// total color from ray march
	float4 totalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 timeOffset = float3(deltaTime, 0.0f, 0.0f);

	//// Grab the depth along this ray
	//// LinerEyeDepth ?
	//// _CameraDepthTexture ?
	//float sceneDepth = LinearEyeDepth(SampleLevel(_CameraDepthTexture, float4(input.position.xy / input.position.w, 0, 0)).r);

	// march through volume
	bool hit = false;
	float3 currentStepDir = stepDirLarge;

	for (int i = 0; i < maxRayMarchSamples; i++)
	{
		float3 samplePos = currentPosition * volumeSamplingScale + timeOffset;

		// sample density
		float4 densitySample = densityTexture.Sample(basicSampler, samplePos) * densityScale + densityBias;

		float density = densitySample.r;

		// seperate fade sample without scrolling
		float4 fadeSample = fadeTexture.SampleLevel(basicSampler, currentPosition, 0);
		float fade = fadeSample.a;

		// adjust by fade
		density *= fade;

		if (density >= densityCutoff)
		{
			if (hit == false)
			{
				hit = true;
				currentPosition -= currentStepDir; //back up
				currentStepDir = stepDirSmall;
				currentPosition += currentStepDir; // forward once with smaller step
			}

			// Calculate the amount of "shadow" blocking light
			float shadowAmount = SampleShadowAmount(
				shadowSampleCount,
				samplePos,
				float3(0.0f, -1.0f, 1.0f),
				shadowSampleOffset,
				densityTexture,
				basicSampler,
				densityScale,
				densityBias,
				fade,
				densityCutoff);

			// Scale as necessary
			shadowAmount *= shadowScale;

			// normalize density based on samples
			density = density / rayMarchSamples * maxRayMarchSamples;

			// color is based on normalized density
			float4 thisColor = float4(fogColor.rgb, density);

			// Beer's law & "powder" look - from HZD talk: 
			// http://advances.realtimerendering.com/s2015/The%20Real-time%20Volumetric%20Cloudscapes%20of%20Horizon%20-%20Zero%20Dawn%20-%20ARTR.pdf
			float beer = exp(-shadowAmount);
			float powder = 1.0f - exp(-shadowAmount * 2);

			// interpolate between colors and apply alpha
			// _LightColor0.rgb
			thisColor.rgb = lerp(thisColor.rgb, float3(0.8f, 0.8f, 0.8f), saturate(beer - powder));

			thisColor.rgb *= thisColor.a;

			// blending 
			totalColor = totalColor + thisColor * (1.0f - totalColor.a);
		}
		else
		{
			if (hit == true) 
			{
				hit = false;
				currentStepDir = stepDirLarge;
			}
		}

		// continue stepping along ray
		currentPosition += currentStepDir;

		//// check for early outs
		//if (totalColor.a >= 0.99f || any(saturate(currentPosition) != currentPosition))
		//	break;

		//// check hit
		//float3 worldPos = AABBToWorld(currentPosition, AABBMin, AABBMax);
		
		//// LinearEyeDepth ? 
		//float rayDepth = LinearEyeDepth(GetRayDepth(input.positionW));

		//// check early out based on depth
		//if (rayDepth > sceneDepth)
		//	break;
	}

	return totalColor;
}