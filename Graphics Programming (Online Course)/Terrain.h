#pragma once
#include "stdafx.h"
#include <wrl.h>
#include <wincodec.h>  
#include <vector>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "Mesh.h"
#include "Lights.h"
#include "PerlinNoise.h"
#include "TerrainCell.h"
#include "FrustumCulling.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const int TEXTURE_REPEAT = 16;

struct HeightMapInfo
{
	int terrainSize = 0;
	DirectX::XMFLOAT3 *heightMap = 0; 
	DirectX::XMFLOAT3 *normal = 0;
	DirectX::XMFLOAT2 *uv = 0;
};

class Terrain  
{
public:
	Terrain(char* fileName, ID3D11Device* device, ID3D11DeviceContext* context);
	Terrain(bool frustumCulling, int terrainSize, float persistence, float frequency, float amplitude, float smoothing, int octaves, int randomSeed, ID3D11Device* device, ID3D11DeviceContext* context);
	~Terrain();

	int GetTerrainCellCount();

	ID3D11ShaderResourceView* GetColorTexture1();
	ID3D11ShaderResourceView* GetColorTexture2();
	ID3D11ShaderResourceView* GetColorTexture3();
	
	Mesh* GetMesh();
	
	void Initialize();
	void Render(ID3D11DeviceContext* context, bool terrainShader, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, DirectionalLight dLight, FrustumCulling* frustum);
	
private:
	struct MatrixBufferType
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT4 fogColor;
		XMFLOAT3 cameraPosition;
		float fogStart;
		XMFLOAT3 lightDirection;
		float fogRange;
	};

	bool frustumCulling;

	int terrainCellCount;

	HeightMapInfo hmInfo;
	int octaves;
	int randomSeed;
	float persistence;
	float frequency;
	float amplitude;
	float smoothing;

	PerlinNoise perlinNoiseGenerator;

	ID3D11Device* device;
	ID3D11DeviceContext* context;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* grassTexture;
	ID3D11ShaderResourceView* slopeTexture;
	ID3D11ShaderResourceView* rockTexture;
	ID3D11ShaderResourceView* snowTexture;
	ID3D11ShaderResourceView* normalTexture;

	ID3D11ShaderResourceView* terrainTexture1;
	ID3D11ShaderResourceView* terrainTexture2;
	ID3D11ShaderResourceView* terrainTexture3;

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11SamplerState* sampler;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;

	Mesh* terrainMesh;

	TerrainCell* terrainCells;

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	int LoadImageDataFromFile(BYTE** imageData, LPCWSTR filename, int &bytesPerRow);

	void GenerateRandomHeightMap();
	
	void CalulateNormals();
	void CalculateTextureCoordinates();
	void LoadTextures();
	void InitializeBuffers();
	void InitializeTerraincCells(Vertex* terrainVertices);

	void InitializeShaders();
	void SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT3 lightDirection);

	void DrawTerrainEditor();
};

