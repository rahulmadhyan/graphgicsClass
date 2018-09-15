#include "GameEntity.h"

GameEntity::GameEntity()
{
}

GameEntity::GameEntity(Mesh* _mesh, Material *_material) : mesh(_mesh), material(_material)
{
	// Set up world matrix
	XMMATRIX gameEntityWorldMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(gameEntityWorldMatrix)); 
	
	SetTranslation(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);
	SetScale(1.0f, 1.0f, 1.0f);
	SetWorldMatrix();
}

GameEntity::~GameEntity()
{
	
}

void GameEntity::SetTranslation(float x, float y, float z)
{
	XMMATRIX currentPosition = XMMatrixTranslation(x, y, z);
	XMStoreFloat4x4(&position, currentPosition);
}

void GameEntity::SetRotation(float roll, float pitch, float yaw)
{
	XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(roll, pitch, yaw);
	XMStoreFloat4x4(&rotation, currentRotation);
}

void GameEntity::SetScale(float xScale, float yScale, float zScale)
{
	XMMATRIX currentScale = XMMatrixScaling(xScale, yScale, zScale);
	XMStoreFloat4x4(&scale, currentScale);
}

void GameEntity::SetWorldMatrix()
{
	XMMATRIX currentPosition = XMLoadFloat4x4(&position);
	XMMATRIX currentRotation = XMLoadFloat4x4(&rotation);
	XMMATRIX currentScale = XMLoadFloat4x4(&scale);
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(currentScale * currentRotation * currentPosition));
}

void GameEntity::PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, DirectionalLight light, DirectionalLight light1)
{
	SimpleVertexShader *materialVertexShader = material->GetVertexShader();
	SimplePixelShader *materialPixelShader = material->GetPixelShader();

	materialVertexShader->SetMatrix4x4("world", worldMatrix);
	materialVertexShader->SetMatrix4x4("view", viewMatrix);
	materialVertexShader->SetMatrix4x4("projection", projectionMatrix);

	materialVertexShader->CopyAllBufferData();
	materialVertexShader->SetShader();

	materialPixelShader->SetData(
		"light",  // The name of the (eventual) variable in the shader
		&light,   // The address of the data to copy
		sizeof(DirectionalLight)); // The size of the data to copy

	materialPixelShader->SetData(
		"light1",  // The name of the (eventual) variable in the shader
		&light1,   // The address of the data to copy
		sizeof(DirectionalLight)); // The size of the data to copy

	ID3D11ShaderResourceView *materialSRV = material->GetResourceView();
	ID3D11SamplerState *materialSampler = material->GetSampler();

	materialPixelShader->SetShaderResourceView("diffuseTexture", materialSRV);
	materialPixelShader->SetSamplerState("basicSampler", materialSampler);
	
	materialPixelShader->CopyAllBufferData();
	materialPixelShader->SetShader();
}

XMFLOAT4X4 GameEntity::GetWorldMatrix()
{
	return worldMatrix;
}

Mesh* GameEntity::GetMesh()
{
	return mesh;
}

Material* GameEntity::GetMaterial()
{
	return material;
}