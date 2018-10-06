#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class FrustumCulling
{
public:
	FrustumCulling(float screenDepth);
	~FrustumCulling();
	
	void ConstructFrustum(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
	bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);
	bool CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);
	bool CheckRectangle2(float maxWidth, float maxHeight, float maxDepth, float minWidth, float minHeight, float minDepth);
private:
	float screenDepth;
	float planes[6][4];
};

