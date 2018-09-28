#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXCore.h"
#include "SimpleShader.h"
#include "Vertex.h"

using namespace DirectX;

class Mesh
{
public:
	Mesh();
	Mesh(Vertex* vertices, int numberOfVertices, UINT* indices, int numberOfIndices, ID3D11Device* device);
	Mesh(char* fileName, ID3D11Device* device);
	~Mesh();

	void CreateBuffers(Vertex* vertices, int numberOfVertices, UINT* indices, int numberOfIndices, ID3D11Device* device);
	ID3D11Buffer* GetVertextBuffer() const;
	ID3D11Buffer* GetIndexBuffer() const;
	int GetIndexCount() const;

private:
	int indexCount;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
};

