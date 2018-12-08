
float SampleShadowAmount(int numSamples, float3 pos, float3 lightDir, float stepAdjust, Texture3D volume, SamplerState basicSampler, float scale, float bias, float fade, float cutoff)
{
	float shadowAmount = 0;
	float step = 1.0f / numSamples * stepAdjust;
	float3 stepDir = lightDir * step;

	[unroll]
	for (int i = 0; i < 5; i++)
	{
		pos += stepDir;
		float shadowSample = (volume.Sample(basicSampler, float4(pos, 0)).r * scale + bias) * fade;
		shadowSample = max(shadowSample, 0.0f); // No negative
		shadowAmount += lerp(0.0f, shadowSample, shadowSample >= cutoff); // Make sure it makes the cut
	}

	return shadowAmount / numSamples; // "Normalize"
}


float GetRayDepth(float3 worldPos, matrix view, matrix projection)
{
	matrix viewProjection = mul(view, projection);
	float4 vp = mul(viewProjection, float4(worldPos, 1));
	return vp.z / vp.w;
}

float FadeNearUVEdge(float3 uv)
{
	float3 fadePos = pow(1.0f - abs(uv * 2 - 1), 1.2f); // 1 is center, 0 is edge
	return min(min(fadePos.x, fadePos.y), fadePos.z);
}

//float NoiseFromTexture2D(float3 x, sampler2D tex)
//{
//	float3 p = floor(x);
//	float3 f = frac(x);
//	f = f * f * (3.0f - 2.0f * f);
//
//	float2 uv = (p.xy + float2(37.0f, 17.0f) * p.z) + f.xy;
//	float2 rg = SampleLevel(tex, float4((uv + 0.5f) / 1024.0f, 0, 0)).yx;
//
//	return lerp(rg.x, rg.y, f.z) * 2.0f - 1.0f;
//}
//
//
//float Noise5(float3 p, float time, sampler2D tex)
//{
//	float3 q = p - float3(0.0, 0.1, 1.0) * time;
//
//	float f;
//	f = 0.500000f * NoiseFromTexture2D(q, tex); q = q * 2.02;
//	f += 0.25000f * NoiseFromTexture2D(q, tex); q = q * 2.03;
//	f += 0.12500f * NoiseFromTexture2D(q, tex); q = q * 2.01;
//	f += 0.06250f * NoiseFromTexture2D(q, tex); q = q * 2.02;
//	f += 0.03125f * NoiseFromTexture2D(q, tex);
//
//	return saturate(f);
//}
//
//float Noise4(float3 p, float time, sampler2D tex)
//{
//	float3 q = p - float3(0.0, 0.1, 1.0) * time;
//
//	float f;
//	f = 0.500000f * NoiseFromTexture2D(q, tex); q = q * 2.02;
//	f += 0.25000f * NoiseFromTexture2D(q, tex); q = q * 2.03;
//	f += 0.12500f * NoiseFromTexture2D(q, tex); q = q * 2.01;
//	f += 0.06250f * NoiseFromTexture2D(q, tex);
//
//	return saturate(f);
//}
//
//float Noise3(float3 p, float time, sampler2D tex)
//{
//	float3 q = p - float3(0.0, 0.1, 1.0) * time;
//
//	float f;
//	f = 0.500000f * NoiseFromTexture2D(q, tex); q = q * 2.02;
//	f += 0.25000f * NoiseFromTexture2D(q, tex); q = q * 2.03;
//	f += 0.12500f * NoiseFromTexture2D(q, tex);
//
//	return saturate(f);
//}
//
//float Noise2(float3 p, float time, sampler2D tex)
//{
//	float3 q = p - float3(0.0, 0.1, 1.0) * time;
//
//	float f;
//	f = 0.500000f * NoiseFromTexture2D(q, tex); q = q * 2.02;
//	f += 0.25000f * NoiseFromTexture2D(q, tex);
//
//	return saturate(f);
//}
//
//float NoiseLOD(float3 p, float time, sampler2D tex, int iteration)
//{
//	float result = 0;
//	if (iteration < 50)			result = Noise5(p, time, tex);
//	else if (iteration < 75)	result = Noise4(p, time, tex);
//	else if (iteration < 100)	result = Noise3(p, time, tex);
//	else						result = Noise2(p, time, tex);
//	return result;
//}



// Performs a Ray-Box (specifically AABB) intersection
// Based on: http://prideout.net/blog/?p=64
bool RayAABBIntersection(float3 pos, float3 dir, float3 boxMin, float3 boxMax, out float t0, out float t1)
{
	// Invert the direction and get a test min and max
	float3 invDir = 1.0f / dir;
	float3 testMin = (boxMin - pos) * invDir;
	float3 testMax = (boxMax - pos) * invDir;

	// Figure out min and max of the results
	float3 tmin = min(testMin, testMax);
	float3 tmax = max(testMin, testMax);

	// Get max of tmin's x, y & z
	float2 t = max(tmin.xx, tmin.yz);
	t0 = max(t.x, t.y);

	// Get min of tmax's x, y & z
	t = min(tmax.xx, tmax.yz);
	t1 = min(t.x, t.y);

	// Did we hit?
	return t0 <= t1;
}


float map(float v, float min1, float max1, float min2, float max2)
{
	// Figure out the current percent
	float percent = (v - min1) / (max1 - min1);

	// Figure out the percentage between the other values
	return percent * (max2 - min2) + min2;
}

float2 map(float2 v, float2 min1, float2 max1, float2 min2, float2 max2)
{
	// Figure out the current percent
	float2 percent = (v - min1) / (max1 - min1);

	// Figure out the percentage between the other values
	return percent * (max2 - min2) + min2;
}

float3 map(float3 v, float3 min1, float3 max1, float3 min2, float3 max2)
{
	// Figure out the current percent
	float3 percent = (v - min1) / (max1 - min1);

	// Figure out the percentage between the other values
	return percent * (max2 - min2) + min2;
}

#define SMALL 0.00001f
#define LARGE 0.99999f

float3 WorldToAABB(float3 pos, float3 AABBMin, float3 AABBMax)
{
	return map(pos, AABBMin, AABBMax, float3(SMALL, SMALL, SMALL), float3(LARGE, LARGE, LARGE));
}

float3 AABBToWorld(float3 pos, float3 AABBMin, float3 AABBMax)
{
	return map(pos, float3(SMALL, SMALL, SMALL), float3(LARGE, LARGE, LARGE), AABBMin, AABBMax);
}

#undef SMALL
#undef LARGE