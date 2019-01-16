#include "stdafx.h"
#include "Clouds.h"

Clouds::Clouds()
{

}

Clouds::Clouds(ID3D11Device* device, ID3D11DeviceContext* context) : device(device), context(context)
{
	rayMarchSamples = 200;
	maxRayMarchSamples = 10;
	shadowSampleCount = 0;
	shadowScale = 0;
	densityScale = 35.0f;
	densityBias = 1.0f;
	densityCutoff = 20.0f;
	shadowSampleOffset = 0.0f; 
	volumeSamplingScale = 1.0f;

	AABBMin = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	AABBMax = XMFLOAT3(1.0f, 1.0f, 1.0f);
	fogColor = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);

	matrixBuffer = 0;
	cloudBuffer = 0;

	vertexShader = 0;
	pixelShader = 0;
	inputLayout = 0;
	sampler = 0;

	InitializeBuffers();
	InitializeShaders();
	InitializeTexture();
}

Clouds::~Clouds()
{

}

void Clouds::Render(float deltaTime, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	SetShaderParameters(deltaTime, cameraPosition, worldMatrix, viewMatrix, projectionMatrix);

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

	context->DrawIndexed(cloudMesh->GetIndexCount(), 0, 0);

	DrawCloudEditor();
}

void Clouds::SetShaderParameters(float deltaTime, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	CloudBufferType* dataPtr2;

	XMMATRIX wMatrix = XMLoadFloat4x4(&worldMatrix);
	XMMATRIX vMatrix = XMLoadFloat4x4(&viewMatrix);
	XMMATRIX pMatrix = XMLoadFloat4x4(&projectionMatrix);

	result = context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	dataPtr = (MatrixBufferType*)mappedResource.pData;

	XMStoreFloat4x4(&dataPtr->world, wMatrix);
	XMStoreFloat4x4(&dataPtr->view, vMatrix);
	XMStoreFloat4x4(&dataPtr->projection, pMatrix);

	context->Unmap(matrixBuffer, 0);

	context->VSSetConstantBuffers(0, 1, &matrixBuffer);

	result = context->Map(cloudBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	dataPtr2 = (CloudBufferType*)mappedResource.pData;

	dataPtr2->fogColor = fogColor;
	dataPtr2->cameraPosition = cameraPosition;
	dataPtr2->deltaTime = deltaTime;
	dataPtr2->AABBMin = AABBMin;
	dataPtr2->volumeSamplingScale = volumeSamplingScale;
	dataPtr2->AABBMax = AABBMax;
	dataPtr2->densityScale = densityScale;
	dataPtr2->densityBias = densityBias;
	dataPtr2->densityCutoff = densityCutoff;
	dataPtr2->shadowSampleOffset = shadowSampleOffset;
	dataPtr2->rayMarchSamples = rayMarchSamples;
	dataPtr2->maxRayMarchSamples = maxRayMarchSamples;
	dataPtr2->shadowSampleCount = shadowSampleCount;
	dataPtr2->shadowScale = shadowScale;

	context->Unmap(cloudBuffer, 0);

	context->PSSetConstantBuffers(1, 1, &cloudBuffer);

	context->PSSetShaderResources(0, 1, &densitySRV);
	context->PSSetShaderResources(1, 1, &fadeSRV);
}

void Clouds::InitializeBuffers()
{
	cloudMesh = new Mesh("Resources/Models/cube.obj", device);
	vertexBuffer = cloudMesh->GetVertextBuffer();
	indexBuffer = cloudMesh->GetIndexBuffer();
}

void Clouds::InitializeShaders()
{
	HRESULT result;
	ID3DBlob* error;
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC inputLayoutDescription[3];
	
	unsigned int numberElements;
	D3D11_SAMPLER_DESC samplerDescription;
	D3D11_BUFFER_DESC matrixBufferDescription;
	error = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	const D3D_SHADER_MACRO defines[] =
	{
		"UNIFORM_SAMPLE_STEP", "0",
		NULL, NULL
	};

	result = D3DCompileFromFile(L"Resources/Shaders/CloudVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &error);

	result = D3DCompileFromFile(L"Resources/Shaders/CloudPixelShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &error);

	if (error != nullptr)
		OutputDebugStringA((char*)error->GetBufferPointer());

	if (vertexShader == NULL)
	{
		result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);
	}

	if (pixelShader == NULL)
	{
		result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);
	}

	if (inputLayout == NULL)
	{
		// Create the vertex input layout description.
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

		numberElements = sizeof(inputLayoutDescription) / sizeof(inputLayoutDescription[0]);

		result = device->CreateInputLayout(inputLayoutDescription, numberElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),	&inputLayout);
	}

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	if (sampler == NULL)
	{
		// Create a texture sampler state description.
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

		// Create the texture sampler state.
		result = device->CreateSamplerState(&samplerDescription, &sampler);
	}

	if (matrixBuffer == NULL)
	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		matrixBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDescription.ByteWidth = sizeof(MatrixBufferType);
		matrixBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDescription.MiscFlags = 0;
		matrixBufferDescription.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&matrixBufferDescription, NULL, &matrixBuffer);
	}
	
	if (cloudBuffer == NULL)
	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		matrixBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDescription.ByteWidth = sizeof(CloudBufferType);
		matrixBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDescription.MiscFlags = 0;
		matrixBufferDescription.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&matrixBufferDescription, NULL, &cloudBuffer);
	}
}

void Clouds::InitializeTexture()
{
	D3D11_TEXTURE3D_DESC cloudTextureDescription;
	cloudTextureDescription.Height = 64;
	cloudTextureDescription.Width = 64;
	cloudTextureDescription.Depth = 64;
	cloudTextureDescription.MipLevels = 1;
	cloudTextureDescription.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	cloudTextureDescription.Usage = D3D11_USAGE_DYNAMIC;
	cloudTextureDescription.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	cloudTextureDescription.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	cloudTextureDescription.MiscFlags = 0;

	// density texture
	{
		device->CreateTexture3D(&cloudTextureDescription, NULL, &densityTexture);

		D3D11_MAPPED_SUBRESOURCE mapppedTextureSubresource;
		context->Map(densityTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapppedTextureSubresource);

		XMFLOAT4* textureData = (XMFLOAT4*)mapppedTextureSubresource.pData;

		UINT index = 0;

		srand(time(NULL));

		for (size_t depth = 0; depth < cloudTextureDescription.Depth; depth++)
		{
			for (size_t height = 0; height < cloudTextureDescription.Height; height++)
			{
				for (size_t width = 0; width < cloudTextureDescription.Width; width++)
				{
					float r = ((float) rand() / (RAND_MAX) * 0.4f) + 0.6f;
					textureData[index++] = XMFLOAT4(r, 0, 0, 0);
				}
			}
		}

		context->Unmap(densityTexture, 0);

		device->CreateShaderResourceView(densityTexture, 0, &densitySRV);
	}

	// fade texture
	{
		device->CreateTexture3D(&cloudTextureDescription, NULL, &fadeTexture);

		D3D11_MAPPED_SUBRESOURCE mapppedTextureSubresource;
		context->Map(fadeTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapppedTextureSubresource);

		XMFLOAT4* textureData = (XMFLOAT4*)mapppedTextureSubresource.pData;

		UINT index = 0;
		
		for (size_t depth = 0; depth < 64; depth++)
		{
			for (size_t height = 0; height < 64; height++)
			{
				for (size_t width = 0; width < 64; width++)
				{
					float d = pow(abs(32 - (int)depth), 2);
					float h = pow(abs(32 - (int)height), 2);
					float w = pow(abs(32 - (int)width), 2);
					float distance = sqrt(d+h+w);

					{
						distance = (float)55.4256248 - distance;
						distance = (distance) / 55.4256248;
					}
					
					index = depth * 64 * 64 + height * 64 + width;
					textureData[index] = XMFLOAT4(distance, 0, 0, 0);

				}
			}
		}

		context->Unmap(fadeTexture, 0);

		device->CreateShaderResourceView(fadeTexture, 0, &fadeSRV);
	}
}

void Clouds::DrawCloudEditor()
{
	ImGui::Begin("Cloud Editor", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	ImGui::SetWindowCollapsed(1, ImGuiCond_Once);
	ImGui::SetWindowSize(ImVec2(475.0f, 430.0f));
	
	ImGui::PushItemWidth(300.0f);

	ImGui::Text("Ray March Samples    ");
	ImGui::SameLine();
	ImGui::SliderInt("##RayMarchSamples", &rayMarchSamples, 0, 1000);

	ImGui::Text("Max Ray March Samples");
	ImGui::SameLine();
	ImGui::SliderInt("##MaxRayMarchSamples", &maxRayMarchSamples, 0, 100);

	ImGui::Text("Shadow Sample Count  ");
	ImGui::SameLine();
	ImGui::SliderInt("##ShadowSampleCount", &shadowSampleCount, 0, 100);

	ImGui::Text("Shadow Scale         ");
	ImGui::SameLine();
	ImGui::SliderInt("##ShadowScale", &shadowScale, 0, 100);

	ImGui::Text("Density Scale        ");
	ImGui::SameLine();
	ImGui::SliderFloat("##DensityScale", &densityScale, 0, 50.0f);
	
	ImGui::Text("Density Bias         ");
	ImGui::SameLine();
	ImGui::SliderFloat("##DensityBias", &densityBias, 0, 50.0f);

	ImGui::Text("Density Cutoff       ");
	ImGui::SameLine();
	ImGui::SliderFloat("##DensityCutoff", &densityCutoff, 0, 50.0f);

	ImGui::Text("Shadow Sample Offset ");
	ImGui::SameLine();
	ImGui::SliderFloat("##ShadowSampleOffset", &shadowSampleOffset, 0, 50.0f);

	ImGui::Text("Volume Sampling Scale");
	ImGui::SameLine();
	ImGui::SliderFloat("##VolumeSamplingScale", &volumeSamplingScale, 0, 50.0f);

	float aabbMin[3] = { AABBMin.x, AABBMin.y, AABBMin.z };
	
	ImGui::Text("AABB Min             ");
	ImGui::SameLine();
	ImGui::SliderFloat3("##AABBMin", aabbMin, -5.0f, 5.0f);

	AABBMin = XMFLOAT3(aabbMin[0], aabbMin[1], aabbMin[2]);

	float aabbMax[3] = { AABBMax.x, AABBMax.y, AABBMax.z };

	ImGui::Text("AABB Max             ");
	ImGui::SameLine();
	ImGui::SliderFloat3("##AABBMax", aabbMax, -5.0f, 5.0f);

	AABBMax = XMFLOAT3(aabbMax[0], aabbMax[1], aabbMax[2]);

	float fog[2] = { fogColor.x, fogColor.w };

	ImGui::Text("Fog Color            ");
	ImGui::SameLine();
	ImGui::SliderFloat2("##Fog", fog, 0, 1.0f);

	fogColor = XMFLOAT4(fog[0], fog[0], fog[0], fog[1]);

	ImGui::PopItemWidth();
 	ImGui::End();
}