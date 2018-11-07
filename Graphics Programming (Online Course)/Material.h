#pragma once
#include "stdafx.h"
#include "SimpleShader.h"

class Material
{
public:
	Material();
	Material(SimpleVertexShader* vertex, SimplePixelShader *pixel, ID3D11ShaderResourceView* srv, ID3D11SamplerState* samp);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();

	ID3D11ShaderResourceView* GetResourceView();
	ID3D11SamplerState* GetSampler();

private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	ID3D11ShaderResourceView* resourceView;
	ID3D11SamplerState* sampler;
};

