#pragma once
#include <stdlib.h>
#include <time.h>
#include "Mesh.h"

class Clouds
{
public:
	Clouds();
	Clouds(ID3D11Device* device, ID3D11DeviceContext* context);
	~Clouds();

	void Render(float deltaTime, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);

private:
	struct MatrixBufferType
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct CloudBufferType
	{
		XMFLOAT4 fogColor;
		XMFLOAT3 cameraPosition;
		float deltaTime;
		XMFLOAT3 AABBMin;
		float volumeSamplingScale;
		XMFLOAT3 AABBMax;
		float densityScale;
		float densityBias;
		float densityCutoff;
		int rayMarchSamples;
		int maxRayMarchSamples;
	};

	ID3D11Device* device;
	ID3D11DeviceContext* context;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* cloudBuffer;

	ID3D11Texture3D* densityTexture;
	ID3D11Texture3D* fadeTexture;
	ID3D11ShaderResourceView* densitySRV;
	ID3D11ShaderResourceView* fadeSRV;

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11SamplerState* sampler;

	Mesh* cloudMesh;

	void InitializeBuffers();
	void InitializeShaders();
	void InitializeTexture();
	void SetShaderParameters(float deltaTime, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
};

