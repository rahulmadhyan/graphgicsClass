#pragma once
#include "stdafx.h"
#include "Vertex.h"

using namespace DirectX;

class Mesh
{
public:
	Mesh();
	Mesh(Vertex* vertices, int numberOfVertices, UINT* indices, int numberOfIndices, ID3D11Device* device);
	Mesh(char* fileName, ID3D11Device* device);
	~Mesh();

	int GetIndexCount() const;

	ID3D11Buffer* GetVertextBuffer() const;
	ID3D11Buffer* GetIndexBuffer() const;

	void CreateBuffers(Vertex* vertices, int numberOfVertices, UINT* indices, int numberOfIndices, ID3D11Device* device);
private:
	int indexCount;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	void CalculateTangents(Vertex* vertices, int numberOfVertices, UINT* indices, int numberOfIndices);
};

