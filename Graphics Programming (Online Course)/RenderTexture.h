#pragma once
#include "stdafx.h"

using namespace DirectX;

class RenderTexture
{
public:
	RenderTexture();
	~RenderTexture();

	ID3D11ShaderResourceView* GetShaderResourceView();
	
	XMFLOAT4X4 GetProjectionMatrix();
	XMFLOAT4X4 GetOrthographicMatrix();

	void Initialize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear);
	void SetRenderTarget(ID3D11DeviceContext* context);
	void ClearRenderTarget(ID3D11DeviceContext* context, float red, float green, float blue, float alpha);

private:
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT4X4 orthographicMatrix;
	D3D11_VIEWPORT viewPort;
	ID3D11Texture2D* renderTargetTexture;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11ShaderResourceView* shaderResourceView;
};

