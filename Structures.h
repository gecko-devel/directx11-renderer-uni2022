#pragma once
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};


struct MeshData
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

struct GlobalLight
{
	XMFLOAT4 AmbientLight;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 SpecularLight;
	XMFLOAT3 DirectionToLight;
	FLOAT SpecularPower; // Power to raise falloff by. Harshness of the light, basically.
};

struct PointLight
{
	XMFLOAT4 Color;
	XMFLOAT3 Pos;
	FLOAT Attenuation;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	GlobalLight globalLight;
	PointLight PointLights[20];

	XMFLOAT4 AmbMat;
	XMFLOAT4 DiffMat;
	XMFLOAT4 SpecMat;
	
	XMFLOAT3 EyePosW;

	float mT;
	int numPointLights;
};