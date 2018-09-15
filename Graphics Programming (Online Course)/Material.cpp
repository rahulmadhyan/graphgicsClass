#include "Material.h"

Material::Material()
{
}

Material::Material(SimpleVertexShader* vertex, SimplePixelShader *pixel, ID3D11ShaderResourceView* srv, ID3D11SamplerState* samp) : vertexShader(vertex), pixelShader(pixel), resourceView(srv), sampler(samp)
{

}

Material::~Material()
{

}

SimpleVertexShader* Material::GetVertexShader()
{
	return vertexShader;
}

SimplePixelShader* Material::GetPixelShader()
{
	return pixelShader;
}

ID3D11ShaderResourceView* Material::GetResourceView()
{
	return resourceView;
}

ID3D11SamplerState* Material::GetSampler()
{
	return sampler;
}