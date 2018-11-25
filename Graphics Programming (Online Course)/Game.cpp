#include "stdafx.h"
#include "Game.h"
#include "Vertex.h"
#include <fstream>
#include <atlbase.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore( 
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	delete water;
	delete waterEntity;

	delete reflectionTexture;
	delete refractionTexture;

	delete reflection;

	delete gameTerrain;
	delete terrainEntity;

	delete skybox;

	delete frustum;

	delete GUI::getInstance();

	/*CComPtr<ID3D11Debug> debug; 
	HRESULT hr = device->QueryInterface(IID_PPV_ARGS(&debug));

	if (debug)
	{
		debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		debug = nullptr;
	}*/
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	CreateCamera();
	CreateBasicGeometry();

	//Initialize Light
	dLight.AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dLight.DiffuseColor = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	dLight.Direction = XMFLOAT3(1.0f, 1.0f, 1.0f);

	dLight1.AmbientColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	dLight1.DiffuseColor = XMFLOAT4(0.8f, 0.8f,	0.8f, 1.0f);
	dLight1.Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	//gameTerrain = new Terrain("Resources/HeightMaps/demo.png", device);
	gameTerrain = new Terrain(false, 256, 3.0, 0.01, 7.0, 2.5, 4, device, context);
	gameTerrain->Initialize();

	terrainEntity = new GameEntity(gameTerrain->GetMesh(), NULL);

	terrainEntity->SetTranslation(-20.0f, 25.0f, 10.0f);
	terrainEntity->SetRotation(0.0f, 0.0f, 0.0f);
	terrainEntity->SetScale(0.1, 0.1f, 0.1f);
	terrainEntity->SetWorldMatrix();

	skybox = new Skybox("Resources/Models/cube.obj", L"Resources/Textures/SunnyCubeMap.dds", device);
	
	frustum = new FrustumCulling(1000.0f);

	refractionTexture = new RenderTexture();	
	refractionTexture->Initialize(device, width, height, 1000.0f, 0.1f);
	
	reflectionTexture = new RenderTexture();
	reflectionTexture->Initialize(device, width, height, 1000.0f, 0.1f);

	reflection = new Reflection();
	reflection->Initialize(device);

	water = new Water(25.0f, 75.0f, 0.1f, 200.0f, XMFLOAT2(0.1f, 0.2f), XMFLOAT4(0.0f, 0.3f, 0.5f, 0.5f));
	water->Initialize(device, L"Resources/Textures/WaterNormal.dds");
	waterEntity = new GameEntity(water->GetMesh(), NULL);
	waterEntity->SetTranslation(0.0f, 25.0f, 30.0f);
	waterEntity->SetRotation(0.0f, 0.0f, 0.0f);
	waterEntity->SetScale(0.1, 0.1f, 0.1f);
	waterEntity->SetWorldMatrix();
}


void Game::CreateCamera()
{
	mainCamera = Camera(width, height);
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	mainCamera.SetProjectionMatrix(width, height);
}

void Game::RenderRefraction()
{
	XMFLOAT4 clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, water->GetHeight() + 1.0f);
	
	refractionTexture->SetRenderTarget(context);
	refractionTexture->ClearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);

	gameTerrain->Render(context, false, mainCamera.GetPosition(), terrainEntity->GetWorldMatrix(), mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix(), dLight1, frustum);
	
	reflection->Render(context, gameTerrain->GetMesh()->GetIndexCount(), terrainEntity->GetWorldMatrix(), mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix(), gameTerrain->GetColorTexture1(), gameTerrain->GetColorTexture2(), gameTerrain->GetColorTexture3(), water->GetNormalTexture() , dLight1.DiffuseColor, dLight1.Direction, 2.0f, clipPlane);

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetViewports(1, &viewport);
}

void Game::RenderReflection()
{
	XMFLOAT4 clipPlane = XMFLOAT4(0.0f, 1.0f, 0.f, -water->GetHeight());

	reflectionTexture->SetRenderTarget(context);
	reflectionTexture->ClearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);

	water->RenderReflection(mainCamera.GetPosition(), mainCamera.GetRotation());

	XMFLOAT4X4 reflectionViewMatrix = water->GetReflectionMatrix();

	skybox->Render(context, reflectionViewMatrix, mainCamera.GetProjectionMatrix());

	gameTerrain->Render(context, false, mainCamera.GetPosition(), terrainEntity->GetWorldMatrix(), reflectionViewMatrix, mainCamera.GetProjectionMatrix(), dLight1, frustum);

	reflection->Render(context, gameTerrain->GetMesh()->GetIndexCount(), terrainEntity->GetWorldMatrix(), reflectionViewMatrix, mainCamera.GetProjectionMatrix(), gameTerrain->GetColorTexture1(), gameTerrain->GetColorTexture2(), gameTerrain->GetColorTexture3(), water->GetNormalTexture(), dLight1.DiffuseColor, dLight1.Direction, 2.0f, clipPlane);

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetViewports(1, &viewport);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	mainCamera.Update(deltaTime, totalTime);
	frustum->ConstructFrustum(mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix());
	
	water->Update();

	RenderRefraction();

	RenderReflection();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = {0.4f, 0.6f, 0.75f, 0.0f};

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	skybox->Render(context, mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix());

	gameTerrain->Render(context, true, mainCamera.GetPosition(), terrainEntity->GetWorldMatrix(), mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix(), dLight1, frustum);
	
	water->Render(context, waterEntity->GetWorldMatrix(), mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix(), water->GetReflectionMatrix(), refractionTexture->GetShaderResourceView(), reflectionTexture->GetShaderResourceView(), mainCamera.GetPosition(), dLight1.Direction);

	ID3D11ShaderResourceView *const pSRV[16] = { NULL };
	context->PSSetShaderResources(0, 16, pSRV);

	GUI::getInstance()->Draw();

	swapChain->Present(0, 0);
}

/*
#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (buttonState & 0x0002)
	{
		mainCamera.ResetCamera();
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (buttonState & 0x0001)
	{
		float xRot = x - prevMousePos.x;
		float yRot = y - prevMousePos.y;

		mainCamera.SetXRotation(yRot);
		mainCamera.SetYRotation(xRot);
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion
*/