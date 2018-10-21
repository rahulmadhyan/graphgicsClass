#include "Water.h"

Water::Water(float waterHeight, float waterRadius, float refractReflectScale, float specularShine, XMFLOAT2 normalMapTiling, XMFLOAT4 refractionTint)
{

}

Water::~Water()
{
}

Mesh* Water::GetMesh()
{
	return waterMesh;
}
void Water::Initialize(ID3D11Device* device, WCHAR* fileName)
{
	InitializeBuffers(device);
	LoadTextures(device, fileName);
}

void Water::Update()
{
	waterTranslation += 0.003f;
	if (waterTranslation > 1.0f)
	{
		waterTranslation -= 1.0f;
	}
}

void Water::RenderReflection(XMFLOAT3 cameraPosition, XMFLOAT3 cameraRotation)
{
	// set 'UP' vector
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	// set position of camera in the world
	// invert Y position for planar reflection
	XMVECTOR position = XMVectorSet(cameraPosition.x, -cameraPosition.y + (waterHeight * 2.0f), cameraPosition.z, 0.0);
	XMVECTOR direction = XMVectorSet(0.0f, 0.0f, 1.0, 0.0f);

	// set roll, pitch & yaw rotation in radian
	// invert X rotation for reflection
	float pitch = -cameraRotation.x * 0.0174532925f;
	float yaw = cameraRotation.y * 0.0174532925f;
	float roll = cameraRotation.z * 0.0174532925f;

	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// transform 'direction' and 'up' vector my rotation matrix so the view rotated correctly at origin
	direction = XMVector3TransformCoord(direction, rotation);
	up = XMVector3TransformCoord(up, rotation);

	// translate rotated camera position to location of viewer
	direction = direction + position;

	// create reflection view matrix from updated vectors
	XMMATRIX RV = XMMatrixLookToLH(
		position,  
		direction,  
		up);  

	XMStoreFloat4x4(&reflectionMatrix, XMMatrixTranspose(RV));
}

void Water::Render(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix,
	XMFLOAT4X4 reflectionMatrix, ID3D11ShaderResourceView* refractionTexture,
	ID3D11ShaderResourceView* reflectionTexture, ID3D11ShaderResourceView* normalTexture,
	XMFLOAT3 cameraPosition, XMFLOAT3 lightDirection)
{
	SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix, refractionTexture, reflectionTexture, normalTexture, cameraPosition, lightDirection);

	unsigned int stride;
	unsigned int offset;

	stride = sizeof(Vertex);
	offset = 0;

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(inputLayout);

	context->VSSetShader(vertexShader, NULL, 0);
	context->PSSetShader(pixelShader, NULL, 0);

	context->PSSetSamplers(0, 1, &sampler);

	context->DrawIndexed(waterMesh->GetIndexCount(), 0, 0);
}


void Water::InitializeBuffers(ID3D11Device* device)
{
	Vertex* vertices;
	UINT* indices;

	int vertexCount = 4;
	int indexCount = 6;

	vertices = new Vertex[vertexCount];
	indices = new UINT[indexCount];

	// Load the vertex array with data.
	vertices[0].Position = XMFLOAT3(-waterRadius, 0.0f, waterRadius);  // Top left.
	vertices[0].UV = XMFLOAT2(0.0f, 0.0f);

	vertices[1].Position = XMFLOAT3(waterRadius, 0.0f, waterRadius);  // Top right.
	vertices[1].UV = XMFLOAT2(1.0f, 0.0f);

	vertices[2].Position = XMFLOAT3(-waterRadius, 0.0f, -waterRadius);  // Bottom left.
	vertices[2].UV = XMFLOAT2(0.0f, 1.0f);

	//vertices[3].Position = XMFLOAT3(-waterRadius, 0.0f, -waterRadius);  // Bottom left.
	//vertices[3].UV = XMFLOAT2(0.0f, 1.0f);

	//vertices[4].Position = XMFLOAT3(waterRadius, 0.0f, waterRadius);  // Top right.
	//vertices[4].UV = XMFLOAT2(1.0f, 0.0f);

	vertices[3].Position = XMFLOAT3(waterRadius, 0.0f, -waterRadius);  // Bottom right.
	vertices[3].UV = XMFLOAT2(1.0f, 1.0f);

	// Load the index array with data.
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 3;

	waterMesh = new Mesh(vertices, vertexCount, indices, indexCount, device);

	vertexBuffer = waterMesh->GetVertextBuffer();
	indexBuffer = waterMesh->GetIndexBuffer();

	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
}

void Water::LoadTextures(ID3D11Device* device, WCHAR* fileName)
{
	CreateDDSTextureFromFile(device, fileName, 0, & waterTexture);
}

void Water::InitializeShaders(ID3D11Device* device, HWND hwnd)
{
	unsigned int numElements;
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;

	D3D11_INPUT_ELEMENT_DESC inputLayoutDescription[2];
	D3D11_SAMPLER_DESC samplerDescription;
	D3D11_BUFFER_DESC matrixBufferDescription;
	D3D11_BUFFER_DESC cameraNormalBufferDescription;
	D3D11_BUFFER_DESC waterBuffferDescription;

	D3DCompileFromFile(L"Debug/WaterVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, 0);
	D3DCompileFromFile(L"Debug/WaterPixelShader.hlsl", NULL, NULL, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, 0);

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

	// Get a count of the elements in the layout.
	numElements = sizeof(inputLayoutDescription) / sizeof(inputLayoutDescription[0]);

	device->CreateInputLayout(inputLayoutDescription, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout);

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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

	matrixBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDescription.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDescription.MiscFlags = 0;
	matrixBufferDescription.StructureByteStride = 0;

	device->CreateBuffer(&matrixBufferDescription, NULL, &matrixBuffer);
	
	cameraNormalBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	cameraNormalBufferDescription.ByteWidth = sizeof(CameraNormalBuffer);
	cameraNormalBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraNormalBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraNormalBufferDescription.MiscFlags = 0;
	cameraNormalBufferDescription.StructureByteStride = 0;

	device->CreateBuffer(&cameraNormalBufferDescription, NULL, &cameraNormalBuffer);

	waterBuffferDescription.Usage = D3D11_USAGE_DYNAMIC;
	waterBuffferDescription.ByteWidth = sizeof(WaterBuffer);
	waterBuffferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBuffferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBuffferDescription.MiscFlags = 0;
	waterBuffferDescription.StructureByteStride = 0;

	device->CreateBuffer(&waterBuffferDescription, NULL, &waterBuffer);
}

void Water::SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix,
	XMFLOAT4X4 reflectionMatrix, ID3D11ShaderResourceView* refractionTexture,
	ID3D11ShaderResourceView* reflectionTexture, ID3D11ShaderResourceView* normalTexture,
	XMFLOAT3 cameraPosition, XMFLOAT3 lightDirection)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr1;
	CameraNormalBuffer* dataPtr2;
	WaterBuffer* dataPtr3;

	context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	dataPtr1 = (MatrixBuffer*)mappedResource.pData;

	dataPtr1->world = worldMatrix;
	dataPtr1->view = viewMatrix;
	dataPtr1->projection = projectionMatrix;
	dataPtr1->reflection = reflectionMatrix;

	context->Unmap(matrixBuffer, 0);

	bufferNumber = 0;

	context->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	context->Map(cameraNormalBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	dataPtr2 = (CameraNormalBuffer*)mappedResource.pData;

	dataPtr2->cameraPosition = cameraPosition;
	dataPtr2->padding1 = 0.0f;
	dataPtr2->normalMapTiling = normalMapTiling;
	dataPtr2->padding2 = XMFLOAT2(0.0f, 0.0f);


	context->Unmap(cameraNormalBuffer, 0);

	bufferNumber = 1;

	context->VSSetConstantBuffers(bufferNumber, 1, &cameraNormalBuffer);

	context->Map(waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	dataPtr3 = (WaterBuffer*)mappedResource.pData;

	dataPtr3->waterTranslation = waterTranslation;
	dataPtr3->reflectRefractScale = reflectRefractScale;
	dataPtr3->refractionTint = refractionTint;
	dataPtr3->lightDirection = lightDirection;
	dataPtr3->specularShine = specularShine;
	dataPtr3->padding = XMFLOAT2(0.0f, 0.0f);

	context->Unmap(waterBuffer, 0);

	bufferNumber = 0;

	context->PSSetConstantBuffers(bufferNumber, 1, &waterBuffer);

	context->PSSetShaderResources(0, 1, &refractionTexture);
	context->PSSetShaderResources(1, 1, &reflectionTexture);
	context->PSSetShaderResources(2, 1, &normalTexture);
}