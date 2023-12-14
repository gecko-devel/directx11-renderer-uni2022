#pragma once
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

struct PointLight
{
	XMFLOAT4 Color;
	XMFLOAT3 Pos;
	FLOAT Radius;
	FLOAT Attenuation;
	XMFLOAT3 PaddingLol;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 AmbLight;
	XMFLOAT4 AmbMat;

	XMFLOAT4 DiffLight;
	XMFLOAT4 DiffMat;

	XMFLOAT4 SpecMat;
	XMFLOAT4 SpecLight;
	XMFLOAT3 EyePosW;
	FLOAT SpecPower;

	XMFLOAT3 DirToLight;

	float mT;

	PointLight PointLight1;
};