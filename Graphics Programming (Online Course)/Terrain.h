#pragma once

#include <wincodec.h>  
#include <vector>
#include <DirectXMath.h>
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
	DirectX::XMFLOAT3 *heightMap; //terrain vertex position
	DirectX::XMFLOAT3 *normal;
	DirectX::XMFLOAT2 *uv;
};

class Terrain  
{
public:
	Terrain(char* fileName, ID3D11Device* device);
	Terrain(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed);
	~Terrain();

	int GetIndexCount();
	ID3D11ShaderResourceView* GetGrassTexture();
	ID3D11ShaderResourceView* GetSlopeTexture();
	ID3D11ShaderResourceView* GetRockTexture();
	Mesh* GetMesh();

	void Initialize(ID3D11Device* device, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename,
		WCHAR* rockTextureFilename);
	void Render(ID3D11DeviceContext*);
	void Shutdown();
	
private:

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	int LoadImageDataFromFile(BYTE** imageData, LPCWSTR filename, int &bytesPerRow);
	void GenerateMesh(ID3D11Device* device);

	void GenerateRandomHeightMap(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed);
	void CalulateNormals();
	
	void CalculateTextureCoordinates();
	void LoadTextures(ID3D11Device*, WCHAR*, WCHAR*, WCHAR*);
	void ReleaseTextures();

	void InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	int numFaces = 0;
	int numVertices = 0;

	HeightMapInfo hmInfo;
	Mesh* terrainMesh;

	PerlinNoise perlinNoiseGenerator;

	int m_terrainWidth, m_terrainHeight;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	ID3D11ShaderResourceView *m_GrassTexture, *m_SlopeTexture, *m_RockTexture;
};

