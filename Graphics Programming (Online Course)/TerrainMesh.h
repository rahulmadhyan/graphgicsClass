#pragma once

#include <fstream>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "DXCore.h"

using namespace DirectX;

class TerrainMesh
{

private:
	struct MatrixBufferType
	{
		XMFLOAT4X4 worldMatrix;
		XMFLOAT4X4 viewMatrix;
		XMFLOAT4X4 projectionMatrix;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;
	};

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
	
	bool InitializeShader(ID3D11Device*, WCHAR* vsFileName, WCHAR* psFileName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFileName);

	bool SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT3 lightDirection, ID3D11ShaderResourceView* grassSRV,
		ID3D11ShaderResourceView* slopeSRV, ID3D11ShaderResourceView* rockSRV);
	void RenderShader(ID3D11DeviceContext* context, int indexCount);

public:
	TerrainMesh();
	~TerrainMesh();

	bool Initialize(ID3D11Device* device);
	void Shutdown();

	bool Render(ID3D11DeviceContext* context, int indexCount, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix,
		XMFLOAT4X4 projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT3 lightDirection,
		ID3D11ShaderResourceView* grassSRV, ID3D11ShaderResourceView* slopeSRV, ID3D11ShaderResourceView* rockSRV);
};

