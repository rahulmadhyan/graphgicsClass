#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include <DirectXMath.h>

using namespace DirectX;

class GameEntity
{
public:
	GameEntity();
	GameEntity(Mesh* _mesh, Material *_material);
	~GameEntity();

	void SetTranslation(float x, float y, float z);
	void SetRotation(float roll, float pitch, float yaw);
	void SetScale(float xScale, float yScale, float zScale);
	void SetWorldMatrix();
	void PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, DirectionalLight light, DirectionalLight light1);

	XMFLOAT4X4 GetWorldMatrix();
	Mesh* GetMesh();
	Material* GetMaterial();

private:
	Mesh* mesh;
	Material* material;

	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 position;
	XMFLOAT4X4 rotation;
	XMFLOAT4X4 scale;
};

