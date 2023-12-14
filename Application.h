#pragma once

#include <map>
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "OBJLoader.h"
#include "Structures.h"
#include "BaseCamera.h"
#include "LookToCamera.h"
#include "LookAtCamera.h"
#include "FreelookCamera.h"
#include "GameObject.h"
#include "Input.h"
#include "Time.h"
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
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world;

	ID3D11RasterizerState* _wireframe;

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	// Make Camera
	BaseCamera* _currentCamera;
	std::vector<BaseCamera*> _cameras;

	// lighting vars
	XMFLOAT4 _ambientLight;

	DirectionalLight _directionalLights[20];
	PointLight _pointLights[20];

	ID3D11SamplerState* _pSamplerLinear = nullptr;

	// Input vector
	XMFLOAT3 _input;
	FLOAT _cameraSpeed;

	// Config file
	YAML::Node _config;

	// Material map
	std::map<std::string, Material> _materials;

	// GameObject list
	std::vector<GameObject*> _gameObjects;

private:
	void ParseConfig(std::string configPath);
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

