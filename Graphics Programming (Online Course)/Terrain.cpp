#include "Terrain.h"

Terrain::Terrain(char *fileName, ID3D11Device* device)
{
	int imageBytesPerRow;
	BYTE* imageData;

	//convert char* to wchar*
	wchar_t* wFileName = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, fileName, -1, wFileName, 4096);

	//LoadImageDataFromFile(&imageData, wFileName, imageBytesPerRow);
	GenerateRandomHeightMap(512, 512, 5.0, 0.01, 2.0, 1.0, 4, 2018);
	
	CalulateNormals();

	GenerateMesh(device);

	//delete imageData;
	imageData = 0;

	delete wFileName;
}

Terrain::~Terrain()
{
	delete terrainMesh;
}

Mesh* Terrain::GetMesh()
{
	return terrainMesh; 
}

// get the dxgi format equivilent of a wic format
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

	hmInfo.terrainWidth = textureWidth;
	hmInfo.terrainHeight = textureHeight;
	hmInfo.heightMap = new DirectX::XMFLOAT3[hmInfo.terrainHeight * hmInfo.terrainWidth];

	int k = 0;
	BYTE* height = (BYTE*)malloc(imageSize);

	for (int i = 0; i < hmInfo.terrainHeight; i++)
	{
		for (int j = 0; j < hmInfo.terrainWidth * (bitsPerPixel / 8); j++)
		{
			{
				*(height + (k)) = *(*imageData + (i * (hmInfo.terrainWidth * (bitsPerPixel / 8)) + j));
				k++;
			}
		}
	}

	int index = 0;
	BYTE currentHeight = 0;
	float heightFactor = 10.0f;

	k = 0;

	for (int i = 0; i < hmInfo.terrainHeight; i++) {
		for (int j = 0; j < hmInfo.terrainWidth; j++) {

			index = (i * hmInfo.terrainHeight) + j;

			currentHeight = *(height + k);

			hmInfo.heightMap[index].x = (float)i;
			hmInfo.heightMap[index].y = (float)currentHeight / heightFactor;
			hmInfo.heightMap[index].z = (float)j;

			k += 4;
		} 
	}

	delete height;
	height = 0;

	return imageSize;
}

void Terrain::GenerateRandomHeightMap(int imageWidth, int imageHeight, double persistence, double frequency, double amplitude, double smoothing, int octaves, int randomSeed)
{
	int index;

	perlinNoiseGenerator = PerlinNoise(persistence, frequency, amplitude, smoothing, octaves, randomSeed);

	hmInfo.terrainWidth = imageWidth;
	hmInfo.terrainHeight = imageHeight;
	hmInfo.heightMap = new DirectX::XMFLOAT3[hmInfo.terrainHeight * hmInfo.terrainWidth];

	for (int j = 0; j < hmInfo.terrainHeight; j++) {
		for (int i = 0; i < hmInfo.terrainWidth; i++) {

			index = (i * hmInfo.terrainHeight) + j;

			float value = perlinNoiseGenerator.GetHeight(i, j);

			hmInfo.heightMap[index].x = (float)i;
			hmInfo.heightMap[index].y = (float)value;
			hmInfo.heightMap[index].z = (float)j;
		}
	}

	hmInfo.normal = new XMFLOAT3[hmInfo.terrainHeight * hmInfo.terrainWidth];

	for (int i = 0; i < hmInfo.terrainHeight; i++) {
		for (int j = 0; j < hmInfo.terrainWidth; j++) {

			XMVECTOR oPosition = XMLoadFloat3(&hmInfo.heightMap[i * hmInfo.terrainWidth + j]);

			XMVECTOR aPosition = XMLoadFloat3(&hmInfo.heightMap[((i + 1 < hmInfo.terrainHeight) ? i + 1 : i) * hmInfo.terrainWidth + j]);
			XMVECTOR bPosition = XMLoadFloat3(&hmInfo.heightMap[i * hmInfo.terrainWidth + ((j - 1 < 0) ? j : j - 1)]);
			XMVECTOR cPosition = XMLoadFloat3(&hmInfo.heightMap[((i - 1 < 0) ? i : i - 1) * hmInfo.terrainWidth + j]);
			XMVECTOR dPosition = XMLoadFloat3(&hmInfo.heightMap[i * hmInfo.terrainWidth + ((j + 1 < hmInfo.terrainWidth) ? j + 1 : j)]);

			XMVECTOR oa = aPosition - oPosition;
			XMVECTOR ob = bPosition - oPosition;
			XMVECTOR oc = cPosition - oPosition;
			XMVECTOR od = dPosition - oPosition;

			XMVECTOR normal = XMVector2Normalize(
				XMVector3Normalize(XMVector3Cross(ob, oa)) +
				XMVector3Normalize(XMVector3Cross(oc, ob)) +
				XMVector3Normalize(XMVector3Cross(od, oc)) +
				XMVector3Normalize(XMVector3Cross(oa, od)));

			hmInfo.normal[i * hmInfo.terrainHeight + j].x = XMVectorGetX(normal);
			hmInfo.normal[i * hmInfo.terrainHeight + j].y = XMVectorGetY(normal);
			hmInfo.normal[i * hmInfo.terrainHeight + j].z = XMVectorGetZ(normal);
		}
	}
}

bool Terrain::HeightMapLoad(char* filename)
{
	FILE *filePtr;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;

	int imageSize;
	int index;
	unsigned char height;

	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return 0;

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	hmInfo.terrainWidth = bitmapInfoHeader.biWidth;
	hmInfo.terrainHeight = bitmapInfoHeader.biHeight;

	imageSize = hmInfo.terrainHeight * hmInfo.terrainWidth * 3;

	unsigned char* bitmapImage = new unsigned char[imageSize];

	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	fread(bitmapImage, 1, imageSize, filePtr);

	fclose(filePtr);

	hmInfo.heightMap = new DirectX::XMFLOAT3[hmInfo.terrainHeight * hmInfo.terrainWidth];

	int k = 0;

	float heightFactor = 10.0f;

	for (int i = 0; i < hmInfo.terrainHeight; i++) {
		for (int j = 0; j < hmInfo.terrainWidth; j++) {
			
			height = bitmapImage[k];
			
			index = (i * hmInfo.terrainHeight) + j;
		
			hmInfo.heightMap[index].x = (float)i;
			hmInfo.heightMap[index].y = (float)height / heightFactor;
			hmInfo.heightMap[index].z = (float)j;

			k += 3;
		}
	}

	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

void Terrain::CalulateNormals()
{
	int rows = hmInfo.terrainHeight;
	int columns = hmInfo.terrainWidth;

	hmInfo.normal = new XMFLOAT3[rows * columns];

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {

			XMVECTOR oPosition = XMLoadFloat3(&hmInfo.heightMap[i * columns + j]);

			XMVECTOR aPosition = XMLoadFloat3(&hmInfo.heightMap[((i + 1 < rows) ? i + 1 : i) * columns + j]);
			XMVECTOR bPosition = XMLoadFloat3(&hmInfo.heightMap[i * columns + ((j - 1 < 0) ? j : j - 1)]);
			XMVECTOR cPosition = XMLoadFloat3(&hmInfo.heightMap[((i - 1 < 0) ? i : i - 1) * columns + j]);
			XMVECTOR dPosition = XMLoadFloat3(&hmInfo.heightMap[i * columns + ((j + 1 < columns) ? j + 1 : j)]);

			XMVECTOR oa = aPosition - oPosition;
			XMVECTOR ob = bPosition - oPosition;
			XMVECTOR oc = cPosition - oPosition;
			XMVECTOR od = dPosition - oPosition;

			XMVECTOR normal = XMVector2Normalize(
				XMVector3Normalize(XMVector3Cross(ob, oa)) +
				XMVector3Normalize(XMVector3Cross(oc, ob)) +
				XMVector3Normalize(XMVector3Cross(od, oc)) +
				XMVector3Normalize(XMVector3Cross(oa, od)));

			hmInfo.normal[i * columns + j].x = XMVectorGetX(normal);
			hmInfo.normal[i * columns + j].y = XMVectorGetY(normal);
			hmInfo.normal[i * columns + j].z = XMVectorGetZ(normal);
		}
	}
}

void Terrain::GenerateMesh(ID3D11Device* device)
{
	int columns = hmInfo.terrainWidth;
	int rows = hmInfo.terrainHeight;

	numVertices = rows * columns;
	numFaces = (rows - 1) * (columns - 1) * 2;

	std::vector<Vertex> vertices(numVertices);

	for (DWORD i = 0; i < rows; ++i) {
		for (DWORD j = 0; j < columns; ++j) {
			
			vertices[i * rows + j].Position = hmInfo.heightMap[i * rows + j];
			vertices[i * rows + j].Normal = hmInfo.normal[i * rows + j];
		}
	}

	std::vector<UINT> indices(numFaces * 3);

	int k = 0;
	int texUIndex = 0;
	int texVIndex = 0;
	for (UINT i = 0; i < rows - 1; i++)
	{
		for (UINT j = 0; j < columns - 1; j++)
		{
			indices[k] = i * rows + j;        // Bottom left of quad
			vertices[i* rows + j].UV = XMFLOAT2(texUIndex + 0.0f, texVIndex + 1.0f);

			indices[k + 1] = i * rows + j + 1;        // Bottom right of quad
			vertices[i* rows + j + 1].UV = XMFLOAT2(texUIndex + 1.0f, texVIndex + 1.0f);

			indices[k + 2] = (i + 1) * rows + j;    // Top left of quad
			vertices[(i + 1) * rows + j].UV = XMFLOAT2(texUIndex + 0.0f, texVIndex + 0.0f);


			indices[k + 3] = (i + 1) * rows + j;    // Top left of quad
			vertices[(i + 1) * rows + j].UV = XMFLOAT2(texUIndex + 0.0f, texVIndex + 0.0f);

			indices[k + 4] = i * rows + j + 1;        // Bottom right of quad
			vertices[i * rows + j + 1].UV = XMFLOAT2(texUIndex + 1.0f, texVIndex + 1.0f);

			indices[k + 5] = (i + 1) * rows + j + 1;    // Top right of quad
			vertices[(i + 1) * rows + j + 1].UV = XMFLOAT2(texUIndex + 1.0f, texVIndex + 0.0f);

			k += 6; // next quad

			texUIndex++;
		}
		texUIndex = 0;
		texVIndex++;
	}

	terrainMesh = new Mesh(&vertices[0], numVertices, &indices[0], indices.capacity(), device);

	delete[] hmInfo.heightMap;
	hmInfo.heightMap = 0;
}