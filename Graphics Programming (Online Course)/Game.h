#pragma once
#include "stdafx.h"
#include "DXCore.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"
#include "Terrain.h"
#include "WICTextureLoader.h"
#include "Skybox.h"
#include "FrustumCulling.h"
#include "RenderTexture.h"
#include "Reflection.h"
#include "Water.h"
#include "Clouds.h"
#include "GUI.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	//void OnMouseDown (WPARAM buttonState, int x, int y);
	//void OnMouseUp	 (WPARAM buttonState, int x, int y);
	//void OnMouseMove (WPARAM buttonState, int x, int y);
	//void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void CreateBasicGeometry();
	void CreateCamera();
	void LoadShaders();

	void RenderRefraction();
	void RenderReflection();

	std::vector<GameEntity*> entities;

	// Texture related DX stuff
	ID3D11ShaderResourceView* textureSRV;
	ID3D11ShaderResourceView* normalMapSRV;
	ID3D11SamplerState* sampler;

	// Buffers to hold actual geometry data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	GameEntity* terrainEntity;
	GameEntity* waterEntity;
	GameEntity* cloudEntity;

	//Terrain
	Terrain* gameTerrain;
	
	//Skybox
	Skybox* skybox;
	FrustumCulling* frustum;

	//Water
	RenderTexture* refractionTexture;
	RenderTexture* reflectionTexture;
	Reflection* reflection;
	Water* water;

	//Clouds
	Clouds* clouds;

	//Main Camera
	Camera mainCamera;

	//Lights
	DirectionalLight dLight;
	DirectionalLight dLight1;
};

