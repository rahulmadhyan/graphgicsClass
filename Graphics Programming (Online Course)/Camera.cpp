#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
}

Camera::Camera(unsigned int width, unsigned int height) : screenWidth(width), screenHeight(height)
{
	XMVECTOR pos = XMVectorSet(0, 5, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);

	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!
	
	SetProjectionMatrix(width, height);

	XMStoreFloat3(&position, pos);
	XMStoreFloat3(&direction, dir);
}

Camera::~Camera()
{
}

void Camera::CreateMatrices()
{
	
}

DirectX::XMFLOAT3 Camera::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Camera::GetRotation()
{
	return XMFLOAT3(xRotation, yRotation, 0.0f);
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

void Camera::Update(float deltaTime, float totalTime)
{
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(xRotation, yRotation, 0.0f);
	XMVECTOR defualtVector = XMVectorSet(0.0, 0.0, 1.0, 0.0);

	XMVECTOR pos = XMLoadFloat3(&position);

	XMVECTOR newDirection = XMVector3Transform(defualtVector, rotationMatrix);
	XMStoreFloat3(&direction, newDirection);

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);

	XMVECTOR lrVector = XMVector3Cross(newDirection, up);
	
	float moveRate = 2.0f;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		pos += (newDirection * moveRate * deltaTime);
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		pos += (-newDirection * moveRate * deltaTime);
	}
	
	if (GetAsyncKeyState('A') & 0x8000)
	{
		pos += (lrVector * moveRate * deltaTime);
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		pos += (-lrVector * moveRate * deltaTime);
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		pos += (up * moveRate * deltaTime);
	}

	if (GetAsyncKeyState('X') & 0x8000)
	{
		pos += (-up * moveRate * deltaTime);
	}

	XMStoreFloat3(&position, pos);

	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		newDirection,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!
}

void Camera::SetXRotation(float amount)
{
	xRotation += amount * 0.001f;;
	/*if (xRotation < -0.9f)
	{
		xRotation = -0.9f;
	}
	if (xRotation > 0.9f)
	{
		xRotation = 0.9f;
	}*/
}

void Camera::SetYRotation(float amount)
{
	yRotation += amount * 0.001f;
	/*if (yRotation < -0.9f)
	{
		yRotation = -0.9f;
	}
	if (yRotation > 0.9f)
	{
		yRotation = 0.9f;
	}*/
}

void Camera::SetProjectionMatrix(unsigned int newWidth, unsigned int newHeight)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)newWidth / newWidth,		// Aspect ratio
		0.1f,						// Near clip plane distance
		1000.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

void Camera::ResetCamera()
{
	XMVECTOR pos = XMVectorSet(-10, 10, -5, 0);
	XMStoreFloat3(&position, pos);
	xRotation = 0.0f;
	yRotation = 0.0f;
}