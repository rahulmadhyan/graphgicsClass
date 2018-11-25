#pragma once
#include "stdafx.h"
#include <DDSTextureLoader.h>
#include "Mesh.h"

class Water
{
public:
	Water(float waterHeight, float waterRadius, float refractReflectScale, float specularShine, XMFLOAT2 normalMapTiling, XMFLOAT4 refractionTint);
	~Water();

	float GetHeight();
	ID3D11ShaderResourceView* GetNormalTexture();
	XMFLOAT4X4 GetReflectionMatrix();
	Mesh* GetMesh();

	void Initialize(ID3D11Device* device, WCHAR* fileName);
	void Update(float deltaTime);
	void RenderReflection(XMFLOAT3 cameraPosition, XMFLOAT3 cameraRotation);
	void Render(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix,
		XMFLOAT4X4 reflectionMatrix, ID3D11ShaderResourceView* refractionTexture,
		ID3D11ShaderResourceView* reflectionTexture,
		XMFLOAT3 cameraPosition, XMFLOAT3 lightDirection);

private:
	struct MatrixBuffer
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4X4 reflection;
	};

	struct CameraNormalBuffer
	{
		XMFLOAT3 cameraPosition;
		float padding1;
		XMFLOAT2 normalMapTiling;
		XMFLOAT2 padding2;
	};

	struct WaterBuffer
	{
		XMFLOAT4 refractionTint;
		XMFLOAT3 lightDirection;
		float waterTranslation;
		float reflectRefractScale;
		float specularShine;
		XMFLOAT2 padding;
	};

	float waterHeight;
	float waterRadius;
	float waterTranslation;
	float reflectRefractScale;
	float specularShine;
	XMFLOAT2 normalMapTiling;
	XMFLOAT4 refractionTint;
	XMFLOAT4X4 reflectionMatrix;

	ID3D11ShaderResourceView* waterTexture;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11SamplerState* sampler;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* cameraNormalBuffer;
	ID3D11Buffer* waterBuffer;

	Mesh* waterMesh;

	void InitializeBuffers(ID3D11Device* device);
	void LoadTextures(ID3D11Device* device, WCHAR* fileName);

	void InitializeShaders(ID3D11Device* device);
	void SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix,
		XMFLOAT4X4 reflectionMatrix, ID3D11ShaderResourceView* refractionTexture,
		ID3D11ShaderResourceView* reflectionTexture,
		XMFLOAT3 cameraPosition, XMFLOAT3 lightDirection);
 };

