#include "Reflection.h"

Reflection::Reflection()
{

}

Reflection::~Reflection()
{

}

void Reflection::Initialize()
{

}

void Reflection::Render(ID3D11DeviceContext* context, int indexCount, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalTexture, XMFLOAT4 lightDiffuseColor, XMFLOAT3 lightDirection, float colorTextureBrightness, XMFLOAT4 clipPlane)
{
	SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix, colorTexture, normalTexture, lightDiffuseColor, lightDirection, colorTextureBrightness, clipPlane);

	context->IASetInputLayout(inputLayout);

	context->VSSetShader(vertexShader, NULL, 0);
	context->PSSetShader(pixelShader, NULL, 0);

	context->PSSetSamplers(0, 1, &sampler);

	context->DrawIndexed(indexCount, 0, 0);
}

void Reflection::InitializeShaders(ID3D11Device* device)
{
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC inputLayoutDescription[6];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDescription;
	D3D11_BUFFER_DESC matrixBufferDescription;
	D3D11_BUFFER_DESC clipPlaneBufferDescription;
	D3D11_BUFFER_DESC lightBufferDescription;

	D3DCompileFromFile(L"Debug/ReflectionVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, 0);
	D3DCompileFromFile(L"Debug/ReflectionPixelShader.hlsl", NULL, NULL, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, 0);

	device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);
	
	device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);
	
	inputLayoutDescription[0].SemanticName = "POSITION";
	inputLayoutDescription[0].SemanticIndex = 0;
	inputLayoutDescription[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayoutDescription[0].InputSlot = 0;
	inputLayoutDescription[0].AlignedByteOffset = 0;
	inputLayoutDescription[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDescription[0].InstanceDataStepRate = 0;

	inputLayoutDescription[1].SemanticName = "TEXCOORD";
	inputLayoutDescription[1].SemanticIndex = 0;
	inputLayoutDescription[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayoutDescription[1].InputSlot = 0;
	inputLayoutDescription[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputLayoutDescription[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDescription[1].InstanceDataStepRate = 0;

	inputLayoutDescription[2].SemanticName = "NORMAL";
	inputLayoutDescription[2].SemanticIndex = 0;
	inputLayoutDescription[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayoutDescription[2].InputSlot = 0;
	inputLayoutDescription[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputLayoutDescription[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDescription[2].InstanceDataStepRate = 0;

	inputLayoutDescription[3].SemanticName = "TANGENT";
	inputLayoutDescription[3].SemanticIndex = 0;
	inputLayoutDescription[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayoutDescription[3].InputSlot = 0;
	inputLayoutDescription[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputLayoutDescription[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDescription[3].InstanceDataStepRate = 0;

	inputLayoutDescription[4].SemanticName = "BINORMAL";
	inputLayoutDescription[4].SemanticIndex = 0;
	inputLayoutDescription[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputLayoutDescription[4].InputSlot = 0;
	inputLayoutDescription[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputLayoutDescription[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDescription[4].InstanceDataStepRate = 0;

	inputLayoutDescription[5].SemanticName = "COLOR";
	inputLayoutDescription[5].SemanticIndex = 0;
	inputLayoutDescription[5].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputLayoutDescription[5].InputSlot = 0;
	inputLayoutDescription[5].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputLayoutDescription[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayoutDescription[5].InstanceDataStepRate = 0;

	numElements = sizeof(inputLayoutDescription) / sizeof(inputLayoutDescription[0]);

	device->CreateInputLayout(inputLayoutDescription, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout);
	
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.MipLODBias = 0.0f;
	samplerDescription.MaxAnisotropy = 1;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDescription.BorderColor[0] = 0;
	samplerDescription.BorderColor[1] = 0;
	samplerDescription.BorderColor[2] = 0;
	samplerDescription.BorderColor[3] = 0;
	samplerDescription.MinLOD = 0;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	
	device->CreateSamplerState(&samplerDescription, &sampler);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDescription.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDescription.MiscFlags = 0;
	matrixBufferDescription.StructureByteStride = 0;

	device->CreateBuffer(&matrixBufferDescription, NULL, &matrixBuffer);
	
	clipPlaneBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	clipPlaneBufferDescription.ByteWidth = sizeof(ClipPlaneBuffer);
	clipPlaneBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	clipPlaneBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	clipPlaneBufferDescription.MiscFlags = 0;
	clipPlaneBufferDescription.StructureByteStride = 0;

	device->CreateBuffer(&clipPlaneBufferDescription, NULL, &clipPlaneBuffer);
	
	lightBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDescription.ByteWidth = sizeof(LightBuffer);
	lightBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDescription.MiscFlags = 0;
	lightBufferDescription.StructureByteStride = 0;

	device->CreateBuffer(&lightBufferDescription, NULL, &lightBuffer);
}

void Reflection::SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalTexture, XMFLOAT4 lightDiffuseColor, XMFLOAT3 lightDirection, float colorTextureBrightness, XMFLOAT4 clipPlane)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr;
	ClipPlaneBuffer* dataPtr1;
	LightBuffer* dataPtr2;
	
	context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	
	dataPtr = (MatrixBuffer*)mappedResource.pData;

	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	context->Unmap(matrixBuffer, 0);

	bufferNumber = 0;

	context->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	context->Map(clipPlaneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	
	dataPtr1 = (ClipPlaneBuffer*)mappedResource.pData;

	dataPtr1->clipPlane = clipPlane;

	context->Unmap(clipPlaneBuffer, 0);

	bufferNumber = 1;

	context->VSSetConstantBuffers(bufferNumber, 1, &clipPlaneBuffer);

	context->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	dataPtr2 = (LightBuffer*)mappedResource.pData;

	dataPtr2->lightDiffuseColor = lightDiffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->colorTextureBrightness = colorTextureBrightness;

	context->Unmap(lightBuffer, 0);

	bufferNumber = 0;

	context->PSSetConstantBuffers(bufferNumber, 1, &lightBuffer);

	context->PSSetShaderResources(0, 1, &colorTexture);
	context->PSSetShaderResources(1, 1, &normalTexture);
}

