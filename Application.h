#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
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
	XMFLOAT3 DirToLight;

	XMFLOAT4 SpecMat;
	XMFLOAT4 SpecLight;
	FLOAT SpecPower;
	XMFLOAT3 EyePosW;

	float mT;
};

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
	ID3D11Buffer*			_pPyramidVertexBuffer;
	ID3D11Buffer*           _pCubeIndexBuffer;
	ID3D11Buffer*			_pPyramidIndexBuffer;
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world, _world2, _world3;
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;

	ID3D11RasterizerState* _wireframe;

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	float _t;

	// lighting vars
	XMFLOAT4 AmbientLight;
	XMFLOAT4 AmbientMaterial;

	// Diffuse vars
	XMFLOAT4 DiffuseMaterial;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 directionToLight;

	// Specular vars
	XMFLOAT4 SpecularMaterial;
	XMFLOAT4 SpecularLight;
	FLOAT SpecularPower; // Power to raise falloff by. Harshness of the light, basically.
	XMFLOAT3 EyeWorldPos;

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

