#include "TerrainMesh.h"

TerrainMesh::TerrainMesh()
{
	vertexShader = 0;
	pixelShader = 0;
	layout = 0;
	sampleState = 0;
	matrixBuffer = 0;
	lightBuffer = 0;
}

TerrainMesh::~TerrainMesh()
{
}

bool TerrainMesh::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result = InitializeShader(device, hwnd, L"Debug/TerrainVertexShader.vs", L"Debug/TerrainPixelShader.ps");

	return result;
}

void TerrainMesh::Shutdown()
{
	ShutdownShader();
}

bool TerrainMesh::Render(ID3D11DeviceContext* context, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT4 lightDirection,
	ID3D11ShaderResourceView* grassSRV, ID3D11ShaderResourceView* slopeSRV, ID3D11ShaderResourceView* rockSRV)
{
	// Set the shader parameters that it will use for rendering
	bool result = SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix, ambientColor, diffuseColor, lightDirection, grassSRV,
		slopeSRV, rockSRV);;

	// Now render the prepared buffers with the shader.
	RenderShader(context, indexCount);

	return true;
}

bool TerrainMesh::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFileName, WCHAR* psFileName)
{
	HRESULT result;
	ID3DBlob* errorMesage;
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC vertexShaderInput[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;

	errorMesage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile vertex shader buffer
	result = D3DCompileFromFile(vsFileName, NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMesage);

	// Compile pixel shader buffer
	result = D3DCompileFromFile(psFileName, NULL, NULL, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMesage);

	// Create the vertex shader from the buffer
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);

	// Create the pixel shader from the buffer
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);

	// Create the vertex input description
	vertexShaderInput[0].SemanticName = "POSITION";
	vertexShaderInput[0].SemanticIndex = 0;
	vertexShaderInput[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexShaderInput[0].InputSlot = 0;
	vertexShaderInput[0].AlignedByteOffset = 0;
	vertexShaderInput[0].InputSlot = D3D11_INPUT_PER_VERTEX_DATA;
	vertexShaderInput[0].InstanceDataStepRate = 0;

	vertexShaderInput[1].SemanticName = "TEXCCOORD";
	vertexShaderInput[1].SemanticIndex = 0;
	vertexShaderInput[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexShaderInput[1].InputSlot = 0;
	vertexShaderInput[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexShaderInput[1].InputSlot = D3D11_INPUT_PER_VERTEX_DATA;
	vertexShaderInput[1].InstanceDataStepRate = 0;

	vertexShaderInput[2].SemanticName = "NORAML";
	vertexShaderInput[2].SemanticIndex = 0;
	vertexShaderInput[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexShaderInput[2].InputSlot = 0;
	vertexShaderInput[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexShaderInput[2].InputSlot = D3D11_INPUT_PER_VERTEX_DATA;
	vertexShaderInput[1].InstanceDataStepRate = 0;

	numElements = sizeof(vertexShaderInput) / sizeof(vertexShaderInput[0]);

	result = device->CreateInputLayout(vertexShaderInput, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &layout);

	// Release the vertex shader buffer and the pixel shader buffer since they are no longer need
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Create the texture sampler state descripption
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
}

void TerrainMesh::ShutdownShader()
{
	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the pixel shader.
	if (pixelShader)
	{
		pixelShader->Release();
		pixelShader = 0;
	}

	// Release the vertex shader.
	if (vertexShader)
	{
		vertexShader->Release();
		vertexShader = 0;
	}
}

bool TerrainMesh::SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT4 lightDirection, ID3D11ShaderResourceView* grassSRV,
	ID3D11ShaderResourceView* slopeSRV, ID3D11ShaderResourceView* rockSRV)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr1;
	LightBufferType* dataPtr2;

	// Transpose the matrices to prepare them for the shader
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to
	result = context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer
	dataPtr1 = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer
	XMStoreFloat4x4(&dataPtr1->worldMatrix, worldMatrix);
	XMStoreFloat4x4(&dataPtr1->viewMatrix, viewMatrix);
	XMStoreFloat4x4(&dataPtr1->projectionMatrix, projectionMatrix);

	// Unlock the constant buffer
	context->Unmap(matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex buffer
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values
	context->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	// Lock the light constant buffer so it can be written to.
	result = context->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	
	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->ambientColor = ambientColor;
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;

	// Unlock the constant buffer.
	context->Unmap(lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	context->PSSetConstantBuffers(bufferNumber, 1, &lightBuffer);

	// Set shader texture resources in the pixel shader.
	context->PSSetShaderResources(0, 1, &grassSRV);
	context->PSSetShaderResources(1, 1, &slopeSRV);
	context->PSSetShaderResources(2, 1, &rockSRV);
}

void TerrainMesh::RenderShader(ID3D11DeviceContext* context, int indexCount)
{
	// Set the vertex input layout
	context->IASetInputLayout(layout);

	// Set the vertex and pixel shaders that will be used to render the triangle
	context->VSSetShader(vertexShader, NULL, 0);
	context->PSSetShader(pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader
	context->PSSetSamplers(0, 1, &sampleState);

	// Render the triangle
	context->DrawIndexed(indexCount, 0, 0);
}