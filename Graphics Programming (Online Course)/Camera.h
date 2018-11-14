#pragma once
#include "stdafx.h"
#include <Windows.h>
#include "GUI.h"

class Camera
{
public:
	Camera();
	Camera(unsigned int width, unsigned int height);
	~Camera();

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y);
	void SetProjectionMatrix(unsigned int newWidth, unsigned int newHeight);

	void Update(float deltaTime, float totalTime);
	void DrawCameraEditor();
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

