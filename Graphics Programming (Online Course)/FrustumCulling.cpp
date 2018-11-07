#include "stdafx.h"
#include "FrustumCulling.h"

FrustumCulling::FrustumCulling(float screenDepth) : screenDepth(screenDepth)
{
}

FrustumCulling::~FrustumCulling()
{
}

void FrustumCulling::ConstructFrustum(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	float zMinimum;
	float r;
	float length;

	XMFLOAT4X4 fMatrix;
	XMMATRIX finalMatrix;

	// z minimum distance in the frustum
	zMinimum = -projectionMatrix._43 / projectionMatrix._33;
	r = screenDepth / (screenDepth - zMinimum);

	projectionMatrix._33 = r;
	projectionMatrix._43 = -r * zMinimum;

	XMMATRIX vMatrix = XMLoadFloat4x4(&viewMatrix);
	XMMATRIX pMatrix = XMLoadFloat4x4(&projectionMatrix);

	vMatrix = XMMatrixTranspose(vMatrix);
	pMatrix = XMMatrixTranspose(pMatrix);

	finalMatrix = XMMatrixMultiply(vMatrix, pMatrix);
	XMStoreFloat4x4(&fMatrix, finalMatrix);

	// calculate near plane of the frustum
	planes[0][0] = fMatrix._14 + fMatrix._13;
	planes[0][1] = fMatrix._24 + fMatrix._23;
	planes[0][2] = fMatrix._34 + fMatrix._33;
	planes[0][3] = fMatrix._44 + fMatrix._43;

	// normalize near plan of the frustum
	length = sqrtf((planes[0][0] * planes[0][0]) +
		(planes[0][1] * planes[0][1]) +
		(planes[0][2] * planes[0][2]));

	planes[0][0] /= length;
	planes[0][1] /= length;
	planes[0][2] /= length;
	planes[0][3] /= length;

	// calculate far plane of the frustum
	planes[1][0] = fMatrix._14 - fMatrix._13;
	planes[1][1] = fMatrix._24 - fMatrix._23;
	planes[1][2] = fMatrix._34 - fMatrix._33;
	planes[1][3] = fMatrix._44 - fMatrix._43;

	// normalize far plan of the frustum
	length = sqrtf((planes[1][0] * planes[1][0]) +
		(planes[1][1] * planes[1][1]) +
		(planes[1][2] * planes[1][2]));

	planes[1][0] /= length;
	planes[1][1] /= length;
	planes[1][2] /= length;
	planes[1][3] /= length;

	// calculate left plane of the frustum
	planes[2][0] = fMatrix._14 + fMatrix._11;
	planes[2][1] = fMatrix._24 + fMatrix._21;
	planes[2][2] = fMatrix._34 + fMatrix._31;
	planes[2][3] = fMatrix._44 + fMatrix._41;

	// normalize left plan of the frustum
	length = sqrtf((planes[2][0] * planes[2][0]) +
		(planes[2][1] * planes[2][1]) +
		(planes[2][2] * planes[2][2]));

	planes[2][0] /= length;
	planes[2][1] /= length;
	planes[2][2] /= length;
	planes[2][3] /= length;

	// calculate right plane of the frustum
	planes[3][0] = fMatrix._14 - fMatrix._11;
	planes[3][1] = fMatrix._24 - fMatrix._21;
	planes[3][2] = fMatrix._34 - fMatrix._31;
	planes[3][3] = fMatrix._44 - fMatrix._41;

	// normalize right plan of the frustum
	length = sqrtf((planes[3][0] * planes[3][0]) +
		(planes[3][1] * planes[3][1]) +
		(planes[3][2] * planes[3][2]));

	planes[3][0] /= length;
	planes[3][1] /= length;
	planes[3][2] /= length;
	planes[3][3] /= length;

	// calculate top plane of the frustum
	planes[4][0] = fMatrix._14 - fMatrix._12;
	planes[4][1] = fMatrix._24 - fMatrix._22;
	planes[4][2] = fMatrix._34 - fMatrix._32;
	planes[4][3] = fMatrix._44 - fMatrix._42;

	// normalize top plan of the frustum
	length = sqrtf((planes[4][0] * planes[4][0]) +
		(planes[4][1] * planes[4][1]) +
		(planes[4][2] * planes[4][2]));

	planes[4][0] /= length;
	planes[4][1] /= length;
	planes[4][2] /= length;
	planes[4][3] /= length;

	// calculate bottom plane of the frustum
	planes[5][0] = fMatrix._14 + fMatrix._12;
	planes[5][1] = fMatrix._24 + fMatrix._22;
	planes[5][2] = fMatrix._34 + fMatrix._32;
	planes[5][3] = fMatrix._44 + fMatrix._42;

	// normalize right plan of the frustum
	length = sqrtf((planes[5][0] * planes[5][0]) +
		(planes[5][1] * planes[5][1]) +
		(planes[5][2] * planes[5][2]));

	planes[5][0] /= length;
	planes[5][1] /= length;
	planes[5][2] /= length;
	planes[5][3] /= length;
}

bool FrustumCulling::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
	int i;
	float dotProduct;

	// Check each of the six planes to see if the cube is inside the frustum.
	for (i = 0; i < 6; i++)
	{
		// Check all eight points of the cube to see if they all reside within the frustum.
		dotProduct = (planes[i][0] * (xCenter - radius)) + (planes[i][1] * (yCenter - radius)) + (planes[i][2] * (zCenter - radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + radius)) + (planes[i][1] * (yCenter - radius)) + (planes[i][2] * (zCenter - radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter - radius)) + (planes[i][1] * (yCenter + radius)) + (planes[i][2] * (zCenter - radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + radius)) + (planes[i][1] * (yCenter + radius)) + (planes[i][2] * (zCenter - radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter - radius)) + (planes[i][1] * (yCenter - radius)) + (planes[i][2] * (zCenter + radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + radius)) + (planes[i][1] * (yCenter - radius)) + (planes[i][2] * (zCenter + radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter - radius)) + (planes[i][1] * (yCenter + radius)) + (planes[i][2] * (zCenter + radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + radius)) + (planes[i][1] * (yCenter + radius)) + (planes[i][2] * (zCenter + radius)) + (planes[i][3] * 1.0f);
		if (dotProduct > 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;
}

bool FrustumCulling::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
{
	int i;
	float dotProduct;


	// Check each of the six planes to see if the rectangle is in the frustum or not.
	for (i = 0; i < 6; i++)
	{
		dotProduct = (planes[i][0] * (xCenter - xSize)) + (planes[i][1] * (yCenter - ySize)) + (planes[i][2] * (zCenter - zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + xSize)) + (planes[i][1] * (yCenter - ySize)) + (planes[i][2] * (zCenter - zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter - xSize)) + (planes[i][1] * (yCenter + ySize)) + (planes[i][2] * (zCenter - zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + xSize)) + (planes[i][1] * (yCenter + ySize)) + (planes[i][2] * (zCenter - zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter - xSize)) + (planes[i][1] * (yCenter - ySize)) + (planes[i][2] * (zCenter + zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + xSize)) + (planes[i][1] * (yCenter - ySize)) + (planes[i][2] * (zCenter + zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter - xSize)) + (planes[i][1] * (yCenter + ySize)) + (planes[i][2] * (zCenter + zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = (planes[i][0] * (xCenter + xSize)) + (planes[i][1] * (yCenter + ySize)) + (planes[i][2] * (zCenter + zSize)) + (planes[i][3] * 1.0f);
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;
}

bool FrustumCulling::CheckRectangle2(float maxWidth, float maxHeight, float maxDepth, float minWidth, float minHeight, float minDepth)
{
	int i;
	float dotProduct;

	// Check if any of the 6 planes of the rectangle are inside the view frustum.
	for (i = 0; i < 6; i++)
	{
		dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * minHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * minHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * minHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * minHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
		if (dotProduct >= 0.0f)
		{
			continue;
		}

		return false;
	}

	return true;
}