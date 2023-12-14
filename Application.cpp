#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pCubeVertexBuffer = nullptr;
	_pCubeIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;

    _cameraSpeed = 30.0f;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

    // Load the config values and create gameobjects
    LoadConfig("config.yml");

    // Make cameras
    XMFLOAT3 cameraPos = XMFLOAT3(0.0f, 0.0f, -30.0f);
    XMFLOAT3 cameraAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 cameraUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
    FLOAT cameraNear = 0.01f;
    FLOAT cameraFar = 100.0f;


    // Load config values into first camera
    XMFLOAT3 yamlCameraPos = _config["debugCamera"]["position"].as<XMFLOAT3>();
    _cameras.push_back(new Camera(yamlCameraPos, cameraAt, cameraUp, _WindowWidth, _WindowHeight, cameraNear, cameraFar, LookVector::To));
    // Use values above for second camera
    _cameras.push_back(new Camera(cameraPos, cameraAt, cameraUp, _WindowWidth, _WindowHeight, cameraNear, cameraFar, LookVector::At));
    _currentCamera = _cameras.at(0);

    // Make Global Light
    _globalLight.AmbientLight = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    _globalLight.DiffuseLight = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    _globalLight.SpecularLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    _globalLight.DirectionToLight = XMFLOAT3(-0.5f, 0.5f, 0.0f);
    _globalLight.SpecularPower = 5.0f;

    // Add point lights
    PointLight pointLight1;
    pointLight1.Pos = XMFLOAT3(-30.0f, 0.0f, -30.0f);
    pointLight1.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    pointLight1.Attenuation = 0.01f;
    _pointLights[0] = pointLight1;

    PointLight pointLight2;
    pointLight2.Pos = XMFLOAT3(15.0f, 0.0f, 0.0f);
    pointLight2.Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
    pointLight2.Attenuation = 0.01f;
    _pointLights[1] = pointLight2;

    // Create mip-map sampler using DirectX 11
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc)); // Fill with zeros. Makes sure it's allocated but empty.
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Sets to use linear mip mapping.
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

    // Assign texture sampler to the shader register, so it knows to use it
    _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.hlsl", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The HLSL file cannot be compiled. Check VS Outpot for Error Log.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.hlsl", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The HLSL file cannot be compiled. Check VS Outpot for Error Log.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        // labelled from the direction you'd look at the face
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)}, //0 Front Top Left
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },  //1 Front Top Right
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },//2 Front Bottom Left
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)}, //3 Front Bottom Right

        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },      //4 Back Top Right
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },       //5 Back Top Left
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },     //6 Back Bottom Right
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },      //7 Back Bottom Left
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pCubeVertexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
        // back
        0,1,2,
        2,1,3,
        // right
        1,5,3,
        3,5,7,
        // front
        5,4,7,
        4,6,7,
        //left
        4,0,6,
        6,0,2,
        // top
        4,5,0,
        0,5,1,
        // bottom
        2,3,6,
        6,3,7
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pCubeIndexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

void Application::LoadConfig(std::string configPath)
{
    _config = YAML::LoadFile(configPath);

    // Read game objects
    for (YAML::Node goNode : _config["gameObjects"])
    {
        GameObject* go = new GameObject();

        // Set mesh
        // I HATE STRINGS AND THEIR MANY FORMS
        go->SetMeshData(OBJLoader::Load(const_cast<char*>(goNode["modelPath"].as<std::string>().c_str()), _pd3dDevice, false));
        
        // Set texture
       std::string texturePath = goNode["texturePath"].as<std::string>(); // convert to string literal. idk what the means tbh
        // ^ THIS MIGHT NOT WORK
       if (texturePath != "")
        {
            ID3D11ShaderResourceView* texture; // Make texture on stack
            CreateDDSTextureFromFile(_pd3dDevice, std::wstring(texturePath.begin(), texturePath.end()).c_str(), nullptr, &texture); // Read DDS image file and write data to stack

            go->SetTexture(texture);
        }

        // Set position
        go->SetPosition(goNode["position"].as<XMFLOAT3>());

        // Set scale
        go->SetScale(goNode["scale"].as<XMFLOAT3>());

        // Set Rot
        go->SetRotation(goNode["rotation"].as<XMFLOAT3>());

        // Set colour
        go->SetColor(goNode["color"].as<XMFLOAT4>());

        // Load gameobject into the vector
        _gameObjects.push_back(go);
    }
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;


    //Depth creation here
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	hr = InitShadersAndInputLayout();
    if (FAILED(hr))
    {
        return S_FALSE;
    }

	InitVertexBuffer();

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireframe);


    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pCubeVertexBuffer) _pCubeVertexBuffer->Release();
    if (_pCubeIndexBuffer) _pCubeIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();

    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
}

void Application::Update()
{
    float previousT = _t;

    // Update our time
    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        _t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        _t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    _deltaTime = _t - previousT;

    // If K is pressed, go to wireframe render state.
    if (GetAsyncKeyState(0x4B))
        _pImmediateContext->RSSetState(_wireframe);

    // If L is pressed, return to normal render state.
    if (GetAsyncKeyState(0x4C))
        _pImmediateContext->RSSetState(nullptr);

    // If 1 is pressed, switch to camera 0.
    if (GetAsyncKeyState(0x31))
        _currentCamera = _cameras.at(0);

    // If 2 is pressed, switch to camera 1.
    if (GetAsyncKeyState(0x32))
        _currentCamera = _cameras.at(1);

    // Get input vector from WASD + QE for up/down
    _input = XMFLOAT3((int)GetAsyncKeyState(0x41) - (int)GetAsyncKeyState(0x44), (int)GetAsyncKeyState(0x51) - (int)GetAsyncKeyState(0x45), (int)GetAsyncKeyState(0x53) - (int)GetAsyncKeyState(0x57));
    //                                      A                               D                            Q                            E                             W                             S

    // Animate the world
    XMStoreFloat4x4(&_world, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(_t));

    // Move the Free camera
    XMFLOAT3 freeCamPos = _cameras.at(0)->GetPosition();
    XMFLOAT3 freeCamVelocity;
    XMStoreFloat3(&freeCamVelocity, XMVector3Normalize(XMLoadFloat3(&_input)) * (_config["debugCamera"]["speed"].as<float>() * _deltaTime));
    _cameras.at(0)->SetPosition(XMFLOAT3(freeCamPos.x + freeCamVelocity.x, freeCamPos.y + freeCamVelocity.y, freeCamPos.z + freeCamVelocity.z));

    // Move the Orbit camera
    XMFLOAT3 orbitCamPos = _cameras.at(1)->GetPosition();
    _cameras.at(1)->SetPosition(XMFLOAT3(-30.0f * sin(_t), orbitCamPos.y, orbitCamPos.z));

    // Update the camera
    _currentCamera->Update();

    // Update GameObjects
    for (GameObject* go : _gameObjects)
    {
        go->Update();
    }
}

void Application::Draw()
{

    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set the view and projection matrices for later
    XMMATRIX view = XMLoadFloat4x4(&_currentCamera->GetView());
    XMMATRIX projection = XMLoadFloat4x4(&_currentCamera->GetProjection());

    // Render the gameobjects!
    for (GameObject* go : _gameObjects)
    {
        // Set the world matrix
        XMMATRIX world = XMLoadFloat4x4(go->GetWorld());

        // Make a constant buffer template with new shader variable values
        ConstantBuffer cb;
        cb.mWorld = XMMatrixTranspose(world);
        cb.mView = XMMatrixTranspose(view);
        cb.mProjection = XMMatrixTranspose(projection);
        cb.globalLight = _globalLight;
        std::copy(std::begin(_pointLights), std::end(_pointLights), std::begin(cb.PointLights)); // copy pointlights to constant buffer
        cb.AmbMat = go->GetColor();
        cb.DiffMat = go->GetColor();
        cb.SpecMat = go->GetColor();
        cb.EyePosW = _currentCamera->GetPosition();
        cb.mT = _t;
        cb.numPointLights = 2;
        
        // Check for texture
        if (*(go->GetTexture()) != nullptr)
        {
            // Set the textures to use
            _pImmediateContext->PSSetShaderResources(0, 1, go->GetTexture()); // Assign texture slot
            cb.hasTextue = 1;
        }
        else
        {
            cb.hasTextue = 0;
        }

        // Update the shader variables using the constant buffer struct
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;

        // Set vertex and index buffers to draw the model
        _pImmediateContext->IASetVertexBuffers(0, 1, &go->GetMeshData()->VertexBuffer, &go->GetMeshData()->VBStride, &go->GetMeshData()->VBOffset);
        _pImmediateContext->IASetIndexBuffer(go->GetMeshData()->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // Set the shaders to use
        _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
        _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);

        // Send in the constant buffer to the shaders
        _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
        _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);        

        // DRAW!
        _pImmediateContext->DrawIndexed(go->GetMeshData()->IndexCount, 0, 0);
    }
        

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}