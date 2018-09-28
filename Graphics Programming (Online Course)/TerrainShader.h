#pragma once

#include <DirectXMath.h>
#include "DXCore.h"
#include "Mesh.h"

using namespace DirectX;

class TerrainShader
{
public:
	TerrainShader();
	~TerrainShader();

	void Initialize(ID3D11Device* device);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext, int indexCount, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix,
		XMFLOAT4X4 projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT3 lightDirection,
		ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* slopeTexture, ID3D11ShaderResourceView* rockTexture);

private:

	struct MatrixBufferType
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;
	};

	void InitializeShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	void ShutdownShader();
	
	void SetShaderParameters(ID3D11DeviceContext* deviceContext, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix,
		XMFLOAT4X4 projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT3 lightDirection,
		ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* slopeTexture,
		ID3D11ShaderResourceView* rockTexture);
	void RenderShader(ID3D11DeviceContext* context, int indexCount);

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
};

