#include "stdafx.h"
#include "Terrain.h"

Terrain::Terrain(char *fileName, ID3D11Device* device, ID3D11DeviceContext* context) : device(device), context(context)
{
	int imageBytesPerRow;
	BYTE* imageData;

	//convert char* to wchar*
	wchar_t* wFileName = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, fileName, -1, wFileName, 4096);

	LoadImageDataFromFile(&imageData, wFileName, imageBytesPerRow);
	delete wFileName;
	delete imageData;
}

Terrain::Terrain(bool frustumCulling, int terrainSize, float persistence, float frequency, float amplitude, float smoothing, int octaves, int randomSeed, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename, WCHAR* normalTextureFilename, ID3D11Device* device, ID3D11DeviceContext* context) : device(device), context(context),
																																																																																	frustumCulling(frustumCulling),
																																																																																	persistence(persistence),
																																																																																	frequency(frequency),
																																																																																	amplitude(amplitude),
																																																																																	smoothing(smoothing),
																																																																																	octaves(octaves),
																																																																																	randomSeed(randomSeed)
{
	terrainCellCount = 0;

	vertexBuffer = 0;
	indexBuffer = 0;

	grassTexture = 0;
	slopeTexture = 0;
	rockTexture = 0;

	vertexShader = 0;
	pixelShader = 0;
	inputLayout = 0;
	sampler = 0;
	matrixBuffer = 0;
	lightBuffer = 0;

	terrainMesh = 0;

	LoadTextures(grassTextureFilename, slopeTextureFilename, rockTextureFilename, normalTextureFilename);

	hmInfo.terrainSize = terrainSize;

	GenerateRandomHeightMap();

	InitializeShaders();
}

Terrain::~Terrain()
{
	delete terrainMesh;
	terrainMesh = 0;

	if (frustumCulling)
	{
		delete[] terrainCells;
		terrainCells = 0;
	}
	
	delete[] hmInfo.heightMap;
	hmInfo.heightMap = 0;

	delete[] hmInfo.normal;
	hmInfo.normal = 0;

	delete[] hmInfo.uv;
	hmInfo.uv = 0;

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
	if (sampler)
	{
		sampler->Release();
		sampler = 0;
	}

	// Release the layout.
	if (inputLayout)
	{
		inputLayout->Release();
		inputLayout = 0;
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

	if (grassTexture)
	{
		grassTexture->Release();
	}

	if (slopeTexture)
	{
		slopeTexture->Release();
	}

	if (rockTexture)
	{
		rockTexture->Release();
	}

	if (normalTexture)
	{
		normalTexture->Release();
	}
}

int Terrain::GetTerrainCellCount()
{
	return terrainCellCount;
}

ID3D11ShaderResourceView* Terrain::GetColorTexture1()
{
	return grassTexture;
}

ID3D11ShaderResourceView* Terrain::GetColorTexture2()
{
	return slopeTexture;
}

ID3D11ShaderResourceView* Terrain::GetColorTexture3()
{
	return rockTexture;
}

Mesh* Terrain::GetMesh()
{
	return terrainMesh; 
}

void Terrain::Initialize()
{
	CalulateNormals();

	CalculateTextureCoordinates();

	InitializeBuffers();
}

void Terrain::Render(ID3D11DeviceContext* context, bool terrainShader, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, DirectionalLight dLight, FrustumCulling* frustum)
{
	if(terrainShader)
		SetShaderParameters(context, cameraPosition, worldMatrix, viewMatrix, projectionMatrix, dLight.AmbientColor, dLight.DiffuseColor, dLight.Direction);

	if (frustumCulling)
	{
		for (int i = 0; i < terrainCellCount; i++)
		{
			float maxWidth = terrainCells[i].GetMaxWidth();
			float maxHeight = terrainCells[i].GetMaxHeight();
			float maxDepth = terrainCells[i].GetMaxDepth();

			float minWidth = terrainCells[i].GetMinWidth();
			float minHeight = terrainCells[i].GetMinHeight();
			float minDepth = terrainCells[i].GetMinDepth();

			bool render = true;

			render = frustum->CheckRectangle2(maxWidth, maxHeight, maxDepth, minWidth, minHeight, minDepth);

			if (render)
			{
				unsigned int stride;
				unsigned int offset;

				// Set vertex buffer stride and offset.
				stride = sizeof(Vertex);
				offset = 0;

				vertexBuffer = terrainCells[i].GetMesh()->GetVertextBuffer();
				indexBuffer = terrainCells[i].GetMesh()->GetIndexBuffer();

				// Set the vertex buffer to active in the input assembler so it can be rendered.
				context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

				// Set the index buffer to active in the input assembler so it can be rendered.
				context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

				// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				if (terrainShader)
				{
					// Set the vertex input layout.
					context->IASetInputLayout(inputLayout);

					// Set the vertex and pixel shaders that will be used to render this triangle.
					context->VSSetShader(vertexShader, NULL, 0);
					context->PSSetShader(pixelShader, NULL, 0);

					// Set the sampler state in the pixel shader.
					context->PSSetSamplers(0, 1, &sampler);

					// Render the triangle.
					context->DrawIndexed(terrainCells[i].GetMesh()->GetIndexCount(), 0, 0);
				}
			}
		}
	}
	else
	{
		unsigned int stride;
		unsigned int offset;

		// Set vertex buffer stride and offset.
		stride = sizeof(Vertex);
		offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (terrainShader)
		{
			// Set the vertex input layout.
			context->IASetInputLayout(inputLayout);

			// Set the vertex and pixel shaders that will be used to render this triangle.
			context->VSSetShader(vertexShader, NULL, 0);
			context->PSSetShader(pixelShader, NULL, 0);

			// Set the sampler state in the pixel shader.
			context->PSSetSamplers(0, 1, &sampler);

			// Render the triangle.
			context->DrawIndexed(terrainMesh->GetIndexCount(), 0, 0);
		}
	}
	
	if(terrainShader)
		DrawTerrainEditor();
}

void Terrain::DrawTerrainEditor()
{
	ImGui::Begin("Terrain Editor", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);  

	ImGui::SetWindowCollapsed(0, ImGuiCond_Once);
	ImGui::SetWindowSize(ImVec2(500.0f, 200.0f), ImGuiCond_Always);
	
	ImGui::PushItemWidth(300.0f);
	ImGui::Text("Terran Size         ");
	ImGui::SameLine();
	ImGui::SliderInt("##TerrainSize", &hmInfo.terrainSize, 0, 512);

	ImGui::Text("Terran Frequency    ");
	ImGui::SameLine();
	ImGui::SliderFloat("##TerrainFrequency", &frequency, 0.0f, 0.1f);

	ImGui::Text("Terran Persistence  ");
	ImGui::SameLine();
	ImGui::SliderFloat("##TerrainPersistence", &persistence, 0.0f, 10.0);

	ImGui::Text("Terran Amplitude    ");
	ImGui::SameLine();
	ImGui::SliderFloat("##TerrainAmplitude", &amplitude, 0.0f, 20.0);

	ImGui::Text("Terran Smoothing    ");
	ImGui::SameLine();
	ImGui::SliderFloat("##TerrainSmoothing", &smoothing, 0.0f, 10.0);

	ImGui::Text("Terran Octaves      ");
	ImGui::SameLine();
	ImGui::SliderInt("##TerrainOctaves", &octaves, 0.0f, 10.0);

	ImGui::NewLine();
	ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
	if (ImGui::Button("Generate"))
	{
		GenerateRandomHeightMap();
		Initialize();
	}

	ImGui::PopItemWidth();
	ImGui::End();
}

DXGI_FORMAT Terrain::GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

	else return DXGI_FORMAT_UNKNOWN;
}

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID Terrain::GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

	else return GUID_WICPixelFormatDontCare;
}

int Terrain::GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
	else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;
}

// load and decode image from file
int Terrain::LoadImageDataFromFile(BYTE** imageData, LPCWSTR filename, int &bytesPerRow)
{
	HRESULT hr;

	// we only need one instance of the imaging factory to create decoders and frames
	static IWICImagingFactory *wicFactory;

	// reset decoder, frame and converter since these will be different for each image we load
	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		// Initialize the COM library
		CoInitialize(NULL);

		// create the WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
		);
		if (FAILED(hr)) return 0;
	}

	// load a decoder for the image
	hr = wicFactory->CreateDecoderFromFilename(
		filename,                        // Image we want to load in
		NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
		GENERIC_READ,                    // We want to read from this file
		WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
		&wicDecoder                      // the wic decoder to be created
	);
	if (FAILED(hr)) return 0;

	// get image from decoder (this will decode the "frame")
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
	// out how to implement the support yourself

	// convert wic pixel format to dxgi pixel format
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	// if the format of the image is not a supported dxgi format, try to convert it
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
	{
		// get a dxgi compatible wic format from the current image format
		WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

		// return if no dxgi compatible format was found
		if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

		// set the dxgi format
		dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

		// create the format converter
		hr = wicFactory->CreateFormatConverter(&wicConverter);
		if (FAILED(hr)) return 0;

		// make sure we can convert to the dxgi compatible format
		BOOL canConvert = FALSE;
		hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
		if (FAILED(hr) || !canConvert) return 0;

		// do the conversion (wicConverter will contain the converted image)
		hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return 0;

		// this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
		imageConverted = true;
	}

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
	bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
	int imageSize = bytesPerRow * textureHeight; // total image size in bytes

	// allocate enough memory for the raw image data, and set imageData to point to that memory
	*imageData = (BYTE*)malloc(imageSize);

	// copy (decoded) raw image data into the newly allocated memory (imageData)
	if (imageConverted)
	{
		// if image format needed to be converted, the wic converter will contain the converted image
		hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}
	else
	{
		// no need to convert, just copy data from the wic frame
		hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}

	hmInfo.terrainSize = textureWidth;
	hmInfo.terrainSize = textureHeight;
	hmInfo.heightMap = new DirectX::XMFLOAT3[hmInfo.terrainSize * hmInfo.terrainSize];
	hmInfo.normal = new XMFLOAT3[hmInfo.terrainSize * hmInfo.terrainSize];
	hmInfo.uv = new XMFLOAT2[hmInfo.terrainSize * hmInfo.terrainSize];

	int k = 0;
	BYTE* height = (BYTE*)malloc(imageSize);

	for (int i = 0; i < hmInfo.terrainSize; i++)
	{
		for (int j = 0; j < hmInfo.terrainSize * (bitsPerPixel / 8); j++)
		{
			{
				*(height + (k)) = *(*imageData + (i * (hmInfo.terrainSize * (bitsPerPixel / 8)) + j));
				k++;
			}
		}
	}

	int index = 0;
	BYTE currentHeight = 0;
	float heightFactor = 10.0f;

	k = 0;

	for (int j = 0; j < hmInfo.terrainSize; j++) {
		for (int i = 0; i < hmInfo.terrainSize; i++) {

			index = (j * hmInfo.terrainSize) + i;

			currentHeight = *(height + k);

			hmInfo.heightMap[index].x = (float)i;
			hmInfo.heightMap[index].y = (float)currentHeight / heightFactor;
			hmInfo.heightMap[index].z = (float)j;

			hmInfo.normal[index] = XMFLOAT3(0.0f, 0.0f, 0.0f);
			hmInfo.uv[index] = XMFLOAT2(0.0f, 0.0f);

			k += 4;
		}
	}

	delete height;
	height = 0;

	return imageSize;
}

void Terrain::GenerateRandomHeightMap()
{
	int index;

	perlinNoiseGenerator = PerlinNoise(persistence, frequency, amplitude, smoothing, octaves, randomSeed);

	if (hmInfo.heightMap != NULL)
	{
		delete[] hmInfo.heightMap;
		hmInfo.heightMap = 0;
	}

	if (hmInfo.normal != NULL)
	{
		delete[] hmInfo.normal;
		hmInfo.normal = 0;
	}

	if (hmInfo.uv != NULL)
	{
		delete[] hmInfo.uv;
		hmInfo.uv = 0;
	}

	hmInfo.heightMap = new DirectX::XMFLOAT3[hmInfo.terrainSize * hmInfo.terrainSize];
	hmInfo.normal = new XMFLOAT3[hmInfo.terrainSize * hmInfo.terrainSize];
	hmInfo.uv = new XMFLOAT2[hmInfo.terrainSize * hmInfo.terrainSize];

	for (int j = 0; j < hmInfo.terrainSize; j++) {
		for (int i = 0; i < hmInfo.terrainSize; i++) {

			index = (j * hmInfo.terrainSize) + i;

			float value = perlinNoiseGenerator.GetHeight(i, j);

			hmInfo.heightMap[index].x = (float)i;
			hmInfo.heightMap[index].y = (float)value;
			hmInfo.heightMap[index].z = (float)j;

			hmInfo.normal[index] = XMFLOAT3(0.0f, 0.0f, 0.0f);
			hmInfo.uv[index] = XMFLOAT2(0.0f, 0.0f);
		}
	}
}

void Terrain::CalulateNormals()
{
	for (int i = 0; i < hmInfo.terrainSize; i++) {
		for (int j = 0; j < hmInfo.terrainSize; j++) {

			XMVECTOR oPosition = XMLoadFloat3(&hmInfo.heightMap[i * hmInfo.terrainSize + j]);
			XMVECTOR aPosition = XMLoadFloat3(&hmInfo.heightMap[((i + 1 < hmInfo.terrainSize) ? i + 1 : i) * hmInfo.terrainSize + j]);
			XMVECTOR bPosition = XMLoadFloat3(&hmInfo.heightMap[i * hmInfo.terrainSize + ((j - 1 < 0) ? j : j - 1)]);
			XMVECTOR cPosition = XMLoadFloat3(&hmInfo.heightMap[((i - 1 < 0) ? i : i - 1) * hmInfo.terrainSize + j]);
			XMVECTOR dPosition = XMLoadFloat3(&hmInfo.heightMap[i * hmInfo.terrainSize + ((j + 1 < hmInfo.terrainSize) ? j + 1 : j)]);
			XMVECTOR oa = aPosition - oPosition;
			XMVECTOR ob = bPosition - oPosition;
			XMVECTOR oc = cPosition - oPosition;
			XMVECTOR od = dPosition - oPosition;
			XMVECTOR normal = XMVector2Normalize(
				XMVector3Normalize(XMVector3Cross(ob, oa)) +
				XMVector3Normalize(XMVector3Cross(oc, ob)) +
				XMVector3Normalize(XMVector3Cross(od, oc)) +
				XMVector3Normalize(XMVector3Cross(oa, od)));
			hmInfo.normal[i * hmInfo.terrainSize + j].x = XMVectorGetX(normal);
			hmInfo.normal[i * hmInfo.terrainSize + j].y = XMVectorGetY(normal);
			hmInfo.normal[i * hmInfo.terrainSize + j].z = XMVectorGetZ(normal);
		}
	}
}

void Terrain::CalculateTextureCoordinates()
{
	int incrementCount, i, j, tuCount, tvCount;
	float incrementValue, tuCoordinate, tvCoordinate;

	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)hmInfo.terrainSize;

	// Calculate how many times to repeat the texture.
	incrementCount = hmInfo.terrainSize / TEXTURE_REPEAT;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
	for (j = 0; j < hmInfo.terrainSize; j++)
	{
		for (i = 0; i < hmInfo.terrainSize; i++)
		{
			// Store the texture coordinate in the height map.
			hmInfo.uv[(hmInfo.terrainSize * j) + i].x = tuCoordinate;
			hmInfo.uv[(hmInfo.terrainSize * j) + i].y = tvCoordinate;

			// Increment the tu texture coordinate by the increment value and increment the index by one.
			tuCoordinate += incrementValue;
			tuCount++;

			// Check if at the far right end of the texture and if so then start at the beginning again.
			if (tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		// Increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		// Check if at the top of the texture and if so then start at the bottom again.
		if (tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}
}

void Terrain::LoadTextures(WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename, WCHAR* normalTextureFilename)
{
	CreateWICTextureFromFile(device, context, grassTextureFilename, 0, &grassTexture);
	CreateWICTextureFromFile(device, context, slopeTextureFilename, 0, &slopeTexture);
	CreateWICTextureFromFile(device, context, rockTextureFilename, 0, &rockTexture);
	CreateDDSTextureFromFile(device, context, normalTextureFilename, 0, &normalTexture);
}

void Terrain::InitializeBuffers()
{
	Vertex* vertices;
	UINT* indices;
	int index, i, j;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index1, index2, index3, index4;
	float tu, tv;

	// Calculate the number of vertices in the terrain mesh.
	int vertexCount = (hmInfo.terrainSize - 1) * (hmInfo.terrainSize - 1) * 6;

	// Set the index count to the same as the vertex count.
	int indexCount = vertexCount;

	// Create the vertex array.
	vertices = DBG_NEW Vertex[vertexCount];

	// Create the index array.
	indices = DBG_NEW UINT[indexCount];

	// Initialize the index to the vertex buffer.
	index = 0;

	// Load the vertex and index array with the terrain data.
	for (j = 0; j < (hmInfo.terrainSize - 1); j++)
	{
		for (i = 0; i < (hmInfo.terrainSize - 1); i++)
		{
			index1 = (hmInfo.terrainSize * j) + i;          // Bottom left.
			index2 = (hmInfo.terrainSize * j) + (i + 1);      // Bottom right.
			index3 = (hmInfo.terrainSize * (j + 1)) + i;      // Upper left.
			index4 = (hmInfo.terrainSize * (j + 1)) + (i + 1);  // Upper right.

			// Upper left.
			tv = hmInfo.uv[index3].y;

			// Modify the texture coordinates to cover the top edge.
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].Position = hmInfo.heightMap[index3];
			vertices[index].UV = XMFLOAT2(hmInfo.uv[index3].x, tv);
			vertices[index].Normal = hmInfo.normal[index3];
			indices[index] = index;
			index++;

			// Upper right.
			tu = hmInfo.uv[index4].x;
			tv = hmInfo.uv[index4].y;

			// Modify the texture coordinates to cover the top and right edge.
			if (tu == 0.0f) { tu = 1.0f; }
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].Position = hmInfo.heightMap[index4];
			vertices[index].UV = XMFLOAT2(tu, tv);
			vertices[index].Normal = hmInfo.normal[index4];
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].Position = hmInfo.heightMap[index1];
			vertices[index].UV = hmInfo.uv[index1];
			vertices[index].Normal = hmInfo.normal[index1];
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].Position = hmInfo.heightMap[index1];
			vertices[index].UV = hmInfo.uv[index1];
			vertices[index].Normal = hmInfo.normal[index1];
			indices[index] = index;
			index++;

			// Upper right.
			tu = hmInfo.uv[index4].x;
			tv = hmInfo.uv[index4].y;

			// Modify the texture coordinates to cover the top and right edge.
			if (tu == 0.0f) { tu = 1.0f; }
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].Position = hmInfo.heightMap[index4];
			vertices[index].UV = XMFLOAT2(tu, tv);
			vertices[index].Normal = hmInfo.normal[index4];
			indices[index] = index;
			index++;

			// Bottom right.
			tu = hmInfo.uv[index2].x;

			// Modify the texture coordinates to cover the right edge.
			if (tu == 0.0f) { tu = 1.0f; }

			vertices[index].Position = hmInfo.heightMap[index2];
			vertices[index].UV = XMFLOAT2(tu, hmInfo.uv[index2].y);
			vertices[index].Normal = hmInfo.normal[index2];
			indices[index] = index;
			index++;
		}
	}

	if (terrainMesh != NULL)
	{
		delete terrainMesh;
		terrainMesh = 0;
	}

	terrainMesh = DBG_NEW Mesh(vertices, vertexCount, indices, indexCount, device);

	vertexBuffer = terrainMesh->GetVertextBuffer();
	indexBuffer = terrainMesh->GetIndexBuffer();

	if(frustumCulling)
		InitializeTerraincCells(vertices);

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
}

void Terrain::InitializeTerraincCells(Vertex* terrainVertices)
{
	int cellHeight, cellWidth;
	int cellRowCount;
	int index;

	cellHeight = 33;
	cellWidth = 33;

	cellRowCount = (hmInfo.terrainSize - 1) / (cellWidth - 1);
	terrainCellCount = cellRowCount * cellRowCount;

	terrainCells = new TerrainCell[terrainCellCount];

	for (int j = 0; j < cellRowCount; j++)
	{
		for (int i = 0; i < cellRowCount; i++)
		{
			index = (cellRowCount * j) + i;

			terrainCells[index].InitializeBuffers(device, terrainVertices, i, j, cellHeight, cellWidth, hmInfo.terrainSize);
		}
	}
}

void Terrain::InitializeShaders()
{
	HRESULT result;
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[4];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;

	// Initialize the pointers this function will use to null.
	//errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	ID3DBlob* error = nullptr;
	
	// Compile the vertex shader code.
	result = D3DCompileFromFile(L"Resources/Shaders/TerrainVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &error);

	// Compile the pixel shader code.
	result = D3DCompileFromFile(L"Resources/Shaders/TerrainPixelShader.hlsl", defines, NULL, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &error);

	if (error != nullptr)
		OutputDebugStringA((char*)error->GetBufferPointer());

	if (vertexShader == NULL)
	{
		// Create the vertex shader from the buffer.
		result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);
	}
	
	if (pixelShader == NULL)
	{
		// Create the pixel shader from the buffer.
		result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);
	}
	
	if (inputLayout == NULL)
	{
		// Create the vertex input layout description.
		inputLayoutDesc[0].SemanticName = "POSITION";
		inputLayoutDesc[0].SemanticIndex = 0;
		inputLayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesc[0].InputSlot = 0;
		inputLayoutDesc[0].AlignedByteOffset = 0;
		inputLayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[0].InstanceDataStepRate = 0;

		inputLayoutDesc[1].SemanticName = "TEXCOORD";
		inputLayoutDesc[1].SemanticIndex = 0;
		inputLayoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputLayoutDesc[1].InputSlot = 0;
		inputLayoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		inputLayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[1].InstanceDataStepRate = 0;

		inputLayoutDesc[2].SemanticName = "NORMAL";
		inputLayoutDesc[2].SemanticIndex = 0;
		inputLayoutDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesc[2].InputSlot = 0;
		inputLayoutDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		inputLayoutDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[2].InstanceDataStepRate = 0;

		inputLayoutDesc[3].SemanticName = "TANGENT";
		inputLayoutDesc[3].SemanticIndex = 0;
		inputLayoutDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesc[3].InputSlot = 0;
		inputLayoutDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		inputLayoutDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[3].InstanceDataStepRate = 0;

		// Get a count of the elements in the layout.
		numElements = sizeof(inputLayoutDesc) / sizeof(inputLayoutDesc[0]);

		// Create the vertex input layout.
		result = device->CreateInputLayout(inputLayoutDesc, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
			&inputLayout);
	}
	
	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	if (sampler == NULL)
	{
		// Create a texture sampler state description.
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

		// Create the texture sampler state.
		result = device->CreateSamplerState(&samplerDesc, &sampler);
	}
	
	if (matrixBuffer == NULL)
	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
	}
	
	if (lightBuffer == NULL)
	{
		// Setup the description of the light dynamic constant buffer that is in the pixel shader.
		// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
		lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		lightBufferDesc.ByteWidth = sizeof(LightBufferType);
		lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		lightBufferDesc.MiscFlags = 0;
		lightBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
	}
}

void Terrain::SetShaderParameters(ID3D11DeviceContext* context, XMFLOAT3 cameraPosition, XMFLOAT4X4 worldMatrix, XMFLOAT4X4 viewMatrix,
	XMFLOAT4X4 projectionMatrix, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor, XMFLOAT3 lightDirection)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;

	XMMATRIX wMatrix = XMLoadFloat4x4(&worldMatrix);
	XMMATRIX vMatrix = XMLoadFloat4x4(&viewMatrix);
	XMMATRIX pMatrix = XMLoadFloat4x4(&projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	XMStoreFloat4x4(&dataPtr->world, wMatrix);
	XMStoreFloat4x4(&dataPtr->view, vMatrix);
	XMStoreFloat4x4(&dataPtr->projection, pMatrix);

	// Unlock the constant buffer.
	context->Unmap(matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	context->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	// Lock the light constant buffer so it can be written to.
	result = context->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->ambientColor = ambientColor;
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->fogColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	dataPtr2->cameraPosition = cameraPosition;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->fogStart = 25.0f;
	dataPtr2->fogRange = 50.0f;

	// Unlock the constant buffer.
	context->Unmap(lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	context->PSSetConstantBuffers(bufferNumber, 1, &lightBuffer);

	// Set shader texture resources in the pixel shader.
	context->PSSetShaderResources(0, 1, &grassTexture);
	context->PSSetShaderResources(1, 1, &slopeTexture);
	context->PSSetShaderResources(2, 1, &rockTexture);
	context->PSSetShaderResources(3, 1, &normalTexture);
}