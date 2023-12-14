#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "OBJLoader.h"
#include "Structures.h"
#include "Camera.h"
#include "yaml-cpp/yaml.h"


using namespace DirectX;

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pCubeVertexBuffer;
	ID3D11Buffer*           _pCubeIndexBuffer;
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world, _world2, _world3;

	ID3D11RasterizerState* _wireframe;

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	float _t;
	float _deltaTime;

	// Make Camera
	Camera* _currentCamera;
	std::vector<Camera*> _cameras;

	// lighting vars
	GlobalLight _globalLight;
	PointLight _pointLights[20];

	XMFLOAT4 _ambientMaterial;
	XMFLOAT4 _diffuseMaterial;
	XMFLOAT4 _specularMaterial;

	// Texture vars
	ID3D11ShaderResourceView* _pColorTextureRV = nullptr;
	ID3D11ShaderResourceView* _pSpecularTextureRV = nullptr;
	ID3D11ShaderResourceView* _pNormalTextureRV = nullptr;

	ID3D11SamplerState* _pSamplerLinear = nullptr;

	// Imported model
	MeshData _yippeeMeshData;

	// Input vector
	XMFLOAT3 _input;

	FLOAT _cameraSpeed;

	// Config file
	YAML::Node config;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

