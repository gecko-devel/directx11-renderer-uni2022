#pragma once
#include <vector>
#include <DirectXMath.h>
#include <yaml-cpp/yaml.h>

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

struct Fog
{
private:
	XMFLOAT2 padding;
public:
	float Start;
	float Range;
	XMFLOAT4 Color;
};

struct MeshData
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

struct DirectionalLight
{
	XMFLOAT4 Color;
	XMFLOAT3 Direction;
};

struct PointLight
{
	XMFLOAT4 Color;
	XMFLOAT3 Position;
	FLOAT Attenuation;
};


struct SpotLight
{
public:
	XMFLOAT4 Color;
	XMFLOAT3 Direction;
	FLOAT MaxAngle;
	XMFLOAT3 Position;
	FLOAT Attenuation;
private:
	XMFLOAT4 _padding;
};

struct Material
{
	ID3D11ShaderResourceView* AlbedoTexture = nullptr;
	ID3D11ShaderResourceView* SpecularMapTexture = nullptr;

	XMFLOAT4 AmbientReflectivity;
	XMFLOAT4 DiffuseReflectivity;
	XMFLOAT4 SpecularReflectivity;
	float SpecularPower; // Power to raise falloff by. Harshness of the light, basically.

	bool IsTranslucent = false;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	SpotLight spotLights[20];

	XMFLOAT4 ambientLight;

	Fog fog;

	DirectionalLight directionalLights[20];
	PointLight pointLights[20];

	XMFLOAT4 AmbMat;
	XMFLOAT4 DiffMat;
	XMFLOAT4 SpecMat;

	int hasAlbedoTextue = 0;
	int hasSpecularMapTextue = 0;

	float specularPower;

	int numSpotLights;
	
	XMFLOAT3 EyePosW;

	int numPointLights;
	int numDirectionalLights;
};

// Conversion functions for YAML yoinked and edited from here:
// https://github.com/jbeder/yaml-cpp/wiki/Tutorial#converting-tofrom-native-data-types
namespace YAML
{
	template<>
	struct convert<XMFLOAT3>
	{
		static Node encode(const XMFLOAT3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, XMFLOAT3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<XMFLOAT4>
	{
		static Node encode(const XMFLOAT4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, XMFLOAT4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}
