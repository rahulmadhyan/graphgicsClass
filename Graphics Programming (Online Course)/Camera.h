#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <Windows.h>

class Camera
{
public:
	Camera();
	Camera(unsigned int width, unsigned int height);
	~Camera();

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	void Update(float deltaTime, float totalTime);
	void SetXRotation(float amount);
	void SetYRotation(float amount);
	void SetProjectionMatrix(unsigned int newWidth, unsigned int newHeight);
	void ResetCamera();

private:
	unsigned int screenWidth;
	unsigned int screenHeight;

	float xRotation;
	float yRotation;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 direction;

	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	void CreateMatrices();	
};

