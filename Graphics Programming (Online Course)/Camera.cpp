#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
}

Camera::Camera(unsigned int width, unsigned int height) : screenWidth(width), screenHeight(height)
{
	XMVECTOR pos = XMVectorSet(0, 0, 0, 0);
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

	ResetCamera();
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

void Camera::SetPosition(float x, float y, float z)
{
	XMVECTOR pos = XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&position, pos);
}

void Camera::SetRotation(float x, float y)
{
	xRotation = x;
	yRotation = y;
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
	
	float moveRate = 10.0f;

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

	DrawCameraEditor();
}

void Camera::DrawCameraEditor()
{
	ImGui::Begin("Camera Editor", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

	ImGui::SetWindowCollapsed(0, 2);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(400.0f, 125.0f), ImGuiCond_Always);
	float position[3] = { this->position.x, this->position.y, this->position.z };
	float rotation[2] = { xRotation, yRotation };

	ImGui::Text("Camera keyboard input: W, A, S, D, X, Space");

	ImGui::Text("Camera Position");
	ImGui::SameLine();
	ImGui::SliderFloat3("##CameraPosition", position, -50.0f, 50.0f);

	ImGui::Text("Camera Rotation");
	ImGui::SameLine();
	ImGui::SliderFloat2("##CameraRotation", rotation, -1.0f, 1.0f);

	SetPosition(position[0], position[1], position[2]);
	SetRotation(rotation[0], rotation[1]);

	ImGui::NewLine();
	ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcItemWidth() * 0.2f);
	if (ImGui::Button("Reset Camera"))
	{
		ResetCamera();
	}

	if(ImGui::IsWindowCollapsed())
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 25.0f));
	}
	else
	{
		float windowHeight = ImGui::GetWindowHeight();
		ImGui::SetNextWindowPos(ImVec2(0.0f, windowHeight + 5.0f));
	}

	ImGui::End();
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
	XMVECTOR pos = XMVectorSet(-40, 40, -45, 0);
	XMStoreFloat3(&position, pos);
	xRotation = 0.07f;
	yRotation = 0.48f;
}