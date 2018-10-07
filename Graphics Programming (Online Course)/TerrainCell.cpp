#include "TerrainCell.h"

TerrainCell::TerrainCell()
{
	vertexBuffer = 0;
	indexBuffer = 0;
}

TerrainCell::~TerrainCell()
{
	delete[] vertices;
	vertices = 0;

	delete terrainCellMesh;
}

float TerrainCell::GetMinWidth()
{
	return minWidth;
}

float TerrainCell::GetMinHeight() 
{
	return minHeight;
}

float TerrainCell::GetMinDepth() 
{
	return minDepth;
}

float TerrainCell::GetMaxWidth()
{
	return maxWidth;
}

float TerrainCell::GetMaxHeight()
{
	return maxHeight;
}

float TerrainCell::GetMaxDepth()
{
	return maxDepth;
}

Mesh* TerrainCell::GetMesh()
{
	return terrainCellMesh;
}

void TerrainCell::InitializeBuffers(ID3D11Device* device, Vertex* terrainVertices, int nodeIndexX, int nodeIndexY, int cellHeight, int cellWidth,
	int terrainWidth)
{
	UINT* indices;
	int index;
	int terrainIndex;

	indexCount = (cellHeight - 1) * (cellWidth - 1) * 6;

	vertices = new Vertex[indexCount];
	indices = new UINT[indexCount];

	index = 0;
	terrainIndex = ((nodeIndexX * (cellWidth - 1)) + (nodeIndexY * (cellHeight - 1) * (terrainWidth - 1))) * 6;

	for (int j = 0; j < (cellHeight - 1); j++)
	{
		for (int i = 0; i < (cellWidth - 1) * 6; i++)
		{
			vertices[index] = terrainVertices[terrainIndex];
			indices[index] = index;
			terrainIndex++;
			index++;
		}
		terrainIndex += (terrainWidth * 6) - (cellWidth * 6);
	}

	terrainCellMesh = new Mesh(vertices, indexCount, indices, indexCount, device);

	CalculateCellDimensions();

	delete[] indices;
	indices = 0;
}

void TerrainCell::CalculateCellDimensions()
{
	float width, height, depth;

	// Initialize the dimensions of the node.
	maxWidth = FLT_MIN;
	maxHeight = FLT_MIN;
	maxDepth = FLT_MIN;

	minWidth = FLT_MAX;
	minHeight = FLT_MAX;
	minDepth = FLT_MAX;

	for (int i = 0; i < indexCount; i++)
	{
		width = vertices[i].Position.x;
		height = vertices[i].Position.y;
		depth = vertices[i].Position.z;

		// Check if the width exceeds the minimum or maximum.
		if (width > maxWidth)
		{
			maxWidth = width;
		}
		if (width < minWidth)
		{
			minWidth = width;
		}

		// Check if the height exceeds the minimum or maximum.
		if (height > maxHeight)
		{
			maxHeight = height;
		}
		if (height < minHeight)
		{
			minHeight = height;
		}

		// Check if the depth exceeds the minimum or maximum.
		if (depth > maxDepth)
		{
			maxDepth = depth;
		}
		if (depth < minDepth)
		{
			minDepth = depth;
		}
	}

	// Calculate the center position of this cell.
	positionX = (maxWidth - minWidth) + minWidth;
	positionY = (maxHeight - minHeight) + minHeight;
	positionZ = (maxDepth - minDepth) + minDepth;
}