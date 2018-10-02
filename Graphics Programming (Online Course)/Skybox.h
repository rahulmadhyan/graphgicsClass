#pragma once
#include <DirectXMath.h>
#include <DDSTextureLoader.h>
#include "DXCore.h"
#include "Mesh.h"
#include "Vertex.h"

class Skybox
{
public:
	Skybox(char* filename, WCHAR* skyboxTextureFilename, ID3D11Device* device);
	~Skybox();

	Mesh* GetMesh();

	void Render(ID3D11DeviceContext* context, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);

private:

	struct MatrixBufferType
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* matrixBuffer;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11SamplerState* sampler;
	ID3D11RasterizerState* rasterizer;
	ID3D11DepthStencilState* depthStencil;
	ID3D11ShaderResourceView* skyboxTexture;
	Mesh* skyboxMesh;

	void SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
};

