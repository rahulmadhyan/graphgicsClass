#pragma once

#include <wincodec.h>  
#include <vector>
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "DXCore.h"
#include "Mesh.h"
#include "PerlinNoise.h"

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

	ID3D11ShaderResourceView* GetGrassTexture();
	ID3D11ShaderResourceView* GetSlopeTexture();
	ID3D11ShaderResourceView* GetRockTexture();
	Mesh* GetMesh();

	void Initialize(ID3D11Device* device, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename,
		WCHAR* rockTextureFilename);
	void Render(ID3D11DeviceContext*);
	void Shutdown();
	
private:

	HeightMapInfo hmInfo;
	Mesh* terrainMesh;

	PerlinNoise perlinNoiseGenerator;

	ID3D11Buffer *vertexBuffer, *indexBuffer;
	ID3D11ShaderResourceView *grassTexture, *slopeTexture, *rockTexture;

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	int LoadImageDataFromFile(BYTE** imageData, LPCWSTR filename, int &bytesPerRow);
	void GenerateMesh(ID3D11Device* device);

	void GenerateRandomHeightMap(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed);
	void CalulateNormals();
	
	void CalculateTextureCoordinates();
	void LoadTextures(ID3D11Device* device, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename);
	void ReleaseTextures();

	void InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* context);
};

