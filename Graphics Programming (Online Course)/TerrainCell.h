#pragma once
#include "stdafx.h"
#include "Mesh.h"

class TerrainCell
{
public:
	TerrainCell();
	~TerrainCell();

	float GetMinWidth();
	float GetMinHeight();
	float GetMinDepth();

	float GetMaxWidth();
	float GetMaxHeight();
	float GetMaxDepth();

	Mesh* GetMesh();
	void InitializeBuffers(ID3D11Device* device, Vertex* terrainVertices, int nodeIndexX, int nodeIndexY, int cellHeight, int cellWidth,
		int terrainWidth);
	void CalculateCellDimensions();

private:
	int indexCount;
	int debugFrameIndexCount;
	float minWidth, minHeight, minDepth;
	float maxWidth, maxHeight, maxDepth;
	float positionX, positionY, positionZ;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	Vertex* vertices;
	Mesh* terrainCellMesh;
};

