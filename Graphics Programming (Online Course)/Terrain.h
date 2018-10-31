#pragma once

#include <wincodec.h>  
#include <vector>
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "DXCore.h"
#include "Mesh.h"
#include "Lights.h"
#include "PerlinNoise.h"
#include "TerrainCell.h"
#include "FrustumCulling.h"

using namespace DirectX;

const int TEXTURE_REPEAT = 32;

struct HeightMapInfo
{
	int terrainWidth;
	int terrainHeight;
	DirectX::XMFLOAT3 *heightMap; 
	DirectX::XMFLOAT3 *normal;
	DirectX::XMFLOAT2 *uv;
};

class Terrain  
{
public:
	Terrain(char* fileName, ID3D11Device* device);
	Terrain(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed);
	~Terrain();

	int GetTerrainCellCount();

	ID3D11ShaderResourceView* GetColorTexture1();
	ID3D11ShaderResourceView* GetColorTexture2();
	ID3D11ShaderResourceView* GetColorTexture3();
	
	Mesh* GetMesh();
	
	void Initialize(ID3D11Device* device, bool _frustumCulling, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename, WCHAR* normalTextureFilename);
	void Render(ID3D11DeviceContext* context, bool terrainShader, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, DirectionalLight dLight, FrustumCulling* frustum);
	
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
		XMFLOAT3 lightDirection;
		float padding;
	};

	bool frustumCulling;

	int terrainCellCount;

	HeightMapInfo hmInfo;
	
	PerlinNoise perlinNoiseGenerator;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* grassTexture;
	ID3D11ShaderResourceView* slopeTexture;
	ID3D11ShaderResourceView* rockTexture;
	ID3D11ShaderResourceView* normalTexture;

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

	void GenerateRandomHeightMap(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed);
	
	void CalulateNormals();
	void CalculateTextureCoordinates();
	void LoadTextures(ID3D11Device* device, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename, WCHAR* normalTextureFilename);
	void InitializeBuffers(ID3D11Device* device);
	void InitializeTerraincCells(ID3D11Device* device, Vertex* terrainVertices);

	void InitializeShaders(ID3D11Device* device);
	void SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT3 lightDirection);
};

