#pragma once
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "Structures.h"

class GameObject
{
private:
	MeshData _meshData;
	Material _material;

	XMFLOAT4X4 _trans;
	XMFLOAT4X4 _rot;
	XMFLOAT4X4 _scale;

	XMFLOAT4X4 _world;

public:
	GameObject();

	void Update();

	void SetMeshData(MeshData newMeshData) { _meshData = newMeshData; }
	void SetMaterial(Material newMaterial) { _material = newMaterial; }

	void SetWorld(XMFLOAT4X4 newWorld) { _world = newWorld; }

	void SetPosition(XMFLOAT3 newPosition);
	void SetRotation(XMFLOAT3 newRotation);
	void SetScale(XMFLOAT3 newScale);

	MeshData* GetMeshData() { return &_meshData; }
	Material* GetMaterial() { return &_material; }
	XMFLOAT4X4* GetWorld() { return &_world; }
};

