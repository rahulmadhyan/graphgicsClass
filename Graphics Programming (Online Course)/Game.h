#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"
#include "Terrain.h"
#include "TerrainMesh.h"
#include "WICTextureLoader.h"
#include <DirectXMath.h>

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
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateBasicGeometry();
	void CreateCamera();

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	//Mesh
	Mesh* mesh1;
	Mesh* mesh2;
	Mesh* mesh3;

	//Material
	Material *material1;
	Material *material2;

	//GameEntity
	GameEntity* entity1;
	GameEntity* entity2;
	GameEntity* entity3;
	GameEntity* entity4;
	GameEntity* entity5;
	GameEntity* terrainEntity;

	//Terrain
	Terrain* gameTerrain;
	TerrainMesh* gameTerrainMesh;

	//Collection of entitites
	std::vector<GameEntity*> entities;

	//Main Camera
	Camera mainCamera;

	//Lights
	DirectionalLight dLight;
	DirectionalLight dLight1;

	ID3D11ShaderResourceView* srv1;
	ID3D11ShaderResourceView* srv2;
	ID3D11SamplerState* sampler;
};

