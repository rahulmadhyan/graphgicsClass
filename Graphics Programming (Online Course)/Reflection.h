#pragma once
#include "stdafx.h"
#include <d3dcompiler.h>

using namespace DirectX;

class Reflection
{
public:
	Reflection();
	~Reflection();

	void Initialize(ID3D11Device* device);
	void Render(ID3D11DeviceContext* context, int indexCount, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, ID3D11ShaderResourceView* colorTexture1, ID3D11ShaderResourceView* colorTexture2, ID3D11ShaderResourceView* colorTexture3, ID3D11ShaderResourceView* normalTexture, XMFLOAT4 lightDiffuseColor, XMFLOAT3 lightDirection, float colorTextureBrightness, XMFLOAT4 clipPlane);

private:
	struct MatrixBuffer
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct ClipPlaneBuffer
	{
		XMFLOAT4 clipPlane;
	};

	struct LightBuffer
	{
		XMFLOAT4 lightDiffuseColor;
		XMFLOAT3 lightDirection;
		float colorTextureBrightness;
	};

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11SamplerState* sampler;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* clipPlaneBuffer;
	ID3D11Buffer* lightBuffer;

	void InitializeShaders(ID3D11Device* device);
	void SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, ID3D11ShaderResourceView* colorTexture1, ID3D11ShaderResourceView* colorTexture2, ID3D11ShaderResourceView* colorTexture3, ID3D11ShaderResourceView* normalTexture, XMFLOAT4 lightDiffuseColor, XMFLOAT3 lightDirection, float colorTextureBrightness, XMFLOAT4 clipPlane);
};

