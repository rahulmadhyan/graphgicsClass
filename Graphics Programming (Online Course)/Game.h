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

	void RenderRefraction();
	void RenderReflection();

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

