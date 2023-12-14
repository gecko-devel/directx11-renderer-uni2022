#pragma once
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "Structures.h"

class GameObject
{
private:
	ID3D11ShaderResourceView* _texture = nullptr;
	MeshData _meshData;
	XMFLOAT4 _color;

	XMFLOAT4X4 _world;
	XMFLOAT4X4 _trans;
	XMFLOAT4X4 _rot;
	XMFLOAT4X4 _scale;

public:
	GameObject();

	void Update();

	void SetTexture(ID3D11ShaderResourceView* newTexture) { _texture = newTexture; }
	void SetMeshData(MeshData newMeshData) { _meshData = newMeshData; }

	void SetWorld(XMFLOAT4X4 newWorld) { _world = newWorld; }

	void SetPosition(XMFLOAT3 newPosition);
	void SetRotation(XMFLOAT3 newRotation);
	void SetScale(XMFLOAT3 newScale);

	void SetColor(XMFLOAT4 newColor) { _color = newColor; }

	ID3D11ShaderResourceView** GetTexture() { return &_texture; }
	MeshData* GetMeshData() { return &_meshData; }
	XMFLOAT4X4* GetWorld() { return &_world; }
	XMFLOAT4 GetColor() { return _color; }

};

