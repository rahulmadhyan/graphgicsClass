#include "stdafx.h"
#include "RenderTexture.h"

RenderTexture::RenderTexture()
{

}

RenderTexture::~RenderTexture()
{
	if (renderTargetTexture) { renderTargetTexture->Release(); }
	if (depthStencilView) { depthStencilView->Release(); }
	if (depthStencilBuffer) { depthStencilBuffer->Release(); }
	if (renderTargetView) { renderTargetView->Release(); }
	if (shaderResourceView) { shaderResourceView->Release(); }
}

ID3D11ShaderResourceView* RenderTexture::GetShaderResourceView()
{
	return shaderResourceView;
}

XMFLOAT4X4 RenderTexture::GetProjectionMatrix()
{
	return projectionMatrix;
}

XMFLOAT4X4 RenderTexture::GetOrthographicMatrix()
{
	return orthographicMatrix;
}

void RenderTexture::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear)
{

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;


	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, NULL, &renderTargetTexture);
	
	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(renderTargetTexture, &renderTargetViewDesc, &renderTargetView);
	
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(renderTargetTexture, &shaderResourceViewDesc, &shaderResourceView);
	

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = textureWidth;
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = device->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
	
	// Initailze the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
	
	// Setup the viewport for rendering.
	viewPort.Width = (float)textureWidth;
	viewPort.Height = (float)textureHeight;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, (float)textureHeight / (float)textureWidth, screenNear, screenDepth);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));

	XMMATRIX O = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, screenNear, screenDepth);
	XMStoreFloat4x4(&orthographicMatrix, XMMatrixTranspose(O));
}

void RenderTexture::SetRenderTarget(ID3D11DeviceContext* context)
{
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->RSSetViewports(1, &viewPort);
}

void RenderTexture::ClearRenderTarget(ID3D11DeviceContext* context, float red, float green, float blue, float alpha)
{
	float color[4] = { red, green, blue, alpha };

	context->ClearRenderTargetView(renderTargetView, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}