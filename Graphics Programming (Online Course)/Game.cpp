#include "Game.h"
#include "Vertex.h"
#include <fstream>

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
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;

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
	// Release any (and all!) DirectX objects
	// we've made in the Game class

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;

	delete gameTerrain;
	delete terrainEntity;

	delete skybox;

	delete frustum;

	/*std::vector<GameEntity*>::iterator it;
	for (it = entities.begin(); it < entities.end(); it++)
	{
		GameEntity* currentEntity = *it;
		delete currentEntity;
	}*/

	sampler->Release();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateCamera();
	CreateBasicGeometry();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Initialize Light
	dLight.AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dLight.DiffuseColor = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	dLight.Direction = XMFLOAT3(1.0f, 1.0f, 1.0f);

	dLight1.AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dLight1.DiffuseColor = XMFLOAT4(1.0f, 1.0f,	1.0f, 1.0f);
	dLight1.Direction = XMFLOAT3(1.0f, -1.0f, 1.0f);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	if (!vertexShader->LoadShaderFile(L"Debug/VertexShader.cso"))
		vertexShader->LoadShaderFile(L"VertexShader.cso");		

	pixelShader = new SimplePixelShader(device, context);
	if(!pixelShader->LoadShaderFile(L"Debug/PixelShader.cso"))	
		pixelShader->LoadShaderFile(L"PixelShader.cso");

	// You'll notice that the code above attempts to load each
	// compiled shader file (.cso) from two different relative paths.

	// This is because the "working directory" (where relative paths begin)
	// will be different during the following two scenarios:
	//  - Debugging in VS: The "Project Directory" (where your .cpp files are) 
	//  - Run .exe directly: The "Output Directory" (where the .exe & .cso files are)

	// Checking both paths is the easiest way to ensure both 
	// scenarios work correctly, although others exist
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;

	// Now, create the sampler from the description
	HRESULT ok2 = device->CreateSamplerState(&sampDesc, &sampler);

	//gameTerrain = new Terrain("Debug/HeightMap/demo.png", device);
	gameTerrain = new Terrain(1024, 1024, 3.0, 0.01, 3.0, 1.5, 4, 2018);
	gameTerrain->Initialize(device, L"Debug/Textures/grass1.jpg", L"Debug/Textures/dirt1.jpg", L"Debug/Textures/rock1.jpg");

	terrainEntity = new GameEntity(gameTerrain->GetMesh(), NULL);

	terrainEntity->SetTranslation(-10.0f, 0.0f, 10.0f);
	terrainEntity->SetRotation(0.0f, 0.0f, 0.0f);
	terrainEntity->SetScale(0.1, 0.1f, 0.1f);
	terrainEntity->SetWorldMatrix();

	skybox = new Skybox("Debug/OBJ\ Files/cube.obj", L"Debug/Textures/SunnyCubeMap.dds", device);
	
	frustum = new FrustumCulling(1000.0f);
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

	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.

	//std::vector<GameEntity*>::iterator it;
	//for (it = entities.begin(); it < entities.end(); it++)
	//{
	//	GameEntity *currentEntity = *it;
	//	
	//	currentEntity->PrepareMaterial(mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix(), dLight, dLight1);

	//	UINT stride = sizeof(Vertex);
	//	UINT offset = 0;

	//	Mesh* currentEntityMesh = currentEntity->GetMesh();
	//	ID3D11Buffer* mesh1VertexBuffer = currentEntityMesh->GetVertextBuffer();
	//	context->IASetVertexBuffers(0, 1, &mesh1VertexBuffer, &stride, &offset);
	//	context->IASetIndexBuffer(currentEntityMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	//	// Finally do the actual drawing
	//	//  - Do this ONCE PER OBJECT you intend to draw
	//	//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
	//	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//	//     vertices in the currently set VERTEX BUFFER
	//	context->DrawIndexed(
	//		currentEntityMesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
	//		0,     // Offset to the first index we want to use
	//		0);    // Offset to add to each index when looking up vertices
	//}

	gameTerrain->Render(context, terrainEntity->GetWorldMatrix(),
		mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix(), dLight1, frustum);
	
	skybox->Render(context, mainCamera.GetViewMatrix(), mainCamera.GetProjectionMatrix());

	swapChain->Present(0, 0);
}


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