#pragma once

#include <wincodec.h>  
#include <vector>
#include <DirectXMath.h>
#include "DXCore.h"
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

	Mesh* GetMesh();

private:
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	int LoadImageDataFromFile(BYTE** imageData, LPCWSTR filename, int &bytesPerRow);

	void GenerateRandomHeightMap(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed);
	bool HeightMapLoad(char* filename);
	void CalulateNormals();
	void GenerateMesh(ID3D11Device* device);

	int numFaces = 0;
	int numVertices = 0;

	HeightMapInfo hmInfo;
	Mesh* terrainMesh;

	PerlinNoise perlinNoiseGenerator;
};

