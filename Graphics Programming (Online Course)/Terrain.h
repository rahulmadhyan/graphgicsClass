#pragma once

#include <wincodec.h>  
#include <vector>
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#include "DXCore.h"
#include "TerrainMesh.h"
#include "Mesh.h"
#include "PerlinNoise.h"

using namespace DirectX;

const int TEXTURE_REPEAT = 32;

struct HeightMapInfo
{
	int terrainWidth;
	int terrainHeight;
	DirectX::XMFLOAT3 *heightMap; //terrain vertex position
	DirectX::XMFLOAT3 *normal;
	DirectX::XMFLOAT2 *uv;
};

class Terrain  
{
public:
	Terrain(char* fileName, ID3D11Device* device);
	~Terrain();

	int GetIndexCount();
	ID3D11ShaderResourceView* GetGrassTexture();
	ID3D11ShaderResourceView* GetSlopeTexture();
	ID3D11ShaderResourceView* GetRockTexture();
	Mesh* GetMesh();

	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename,
		WCHAR* rockTextureFilename);
	void Render(ID3D11DeviceContext* context);
	void Shutdown();

private:
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	int LoadImageDataFromFile(BYTE** imageData, LPCWSTR filename, int &bytesPerRow);

	void GenerateRandomHeightMap(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed);
	bool HeightMapLoad(char* filename);
	void GenerateMesh(ID3D11Device* device);

	void CalculateTextureCoordinates();
	void LoadTextures(ID3D11Device* device, ID3D11DeviceContext* context, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename);
	void ReleaseTextures();
	
	void InitializeBuffers(ID3D11Device* device);
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
	void ShutdownBuffers();

	int numFaces = 0;
	int numVertices = 0;
	int vertexCount = 0;
	int indexCount = 0;

	HeightMapInfo hmInfo;
	Mesh* terrainMesh;

	PerlinNoise perlinNoiseGenerator;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* grassTexture;
	ID3D11ShaderResourceView* slopeTexture;
	ID3D11ShaderResourceView* rockTexture;
};

