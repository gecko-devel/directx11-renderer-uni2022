#include "Application.h"

//using namespace YAML;

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

// Sort gameobject distance to camera in descending order
bool Application::CompareDistanceToCamera(GameObject* i1, GameObject* i2)
{
    float i1Distance;
    XMStoreFloat(&i1Distance, XMVector3Length(XMLoadFloat3(&i1->GetPosition()) - XMLoadFloat3(&_currentCamera->GetPosition())));

    float i2Distance;
    XMStoreFloat(&i2Distance, XMVector3Length(XMLoadFloat3(&i2->GetPosition()) - XMLoadFloat3(&_currentCamera->GetPosition())));

    return i1Distance < i2Distance;
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
	_pConstantBuffer = nullptr;

    YAML::Node config = YAML::LoadFile("config.yml");
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

    // Read the config and generate objects within
    ParseConfig("config.yml");

    // Set current camera
    _currentCamera = _cameras[0];

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

    // Create blend state descriptor
    D3D11_BLEND_DESC translucentDesc;
    ZeroMemory(&translucentDesc, sizeof(translucentDesc));
    translucentDesc.AlphaToCoverageEnable = false;
    translucentDesc.IndependentBlendEnable = false;

    translucentDesc.RenderTarget[0].BlendEnable = true;

    translucentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // Set factor to alpha so that it becomes translucent
    translucentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // Set to (- source alpha) so the two can be added together to make a mixed, full colour
    translucentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // Specify to add the two blended colours together

    translucentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // Only use the alpha channel of the source colour...
    translucentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // ...and not the destination.
    translucentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // Add them together - it's just 1 + 0, but it stops the blend from using 1 * 0.
    
    translucentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // Allow the blend to action on *all* colour values.
    

    // Create Blend State from the descriptor
    _pd3dDevice->CreateBlendState(&translucentDesc, &_pTransparentBlendState);

    // Specify empty and fake manual blend factor so that it sets the blend state nicely (directx is pedantic) and set the state at initialisation time.
    // The *real* blend factors are already specified in the render target's SrcBlend, DestBlend, and alpha variants of both.
    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    _pImmediateContext->OMSetBlendState(_pTransparentBlendState, blendFactor, 0xffffffff);

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
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

void Application::ParseConfig(std::string configPath)
{
    _config = YAML::LoadFile(configPath);

    // Read fog settings
    _fog.Start = _config["fog"]["start"].as<float>();
    _fog.Range = _config["fog"]["range"].as<float>();
    _fog.Color = _config["fog"]["color"].as<XMFLOAT4>();

    // Read cameras
    Camera* cam = nullptr;

    for (YAML::Node camNode : _config["cameras"])
    {
        std::string camType = camNode["type"].as<std::string>();

        XMFLOAT3 camPos = camNode["position"].as<XMFLOAT3>();
        XMFLOAT3 camRot = camNode["rotation"].as<XMFLOAT3>();
        float camNear = camNode["near"].as<float>();
        float camFar = camNode["far"].as<float>();

        if (camType == "normal")
        {
            Camera* normalCamera = new Camera(camPos, _WindowWidth, _WindowHeight, camNear, camFar);
            normalCamera->RotateEulerAngles(camRot);
            cam = normalCamera;
        }
        else if (camType == "free")
        {
            float camSpeed = camNode["speed"].as<float>();
            float camRotSpeed = camNode["rotationSpeed"].as<float>();
            FreeCamera* freeCam = new FreeCamera(camSpeed, camRotSpeed, camPos, _WindowWidth, _WindowHeight, camNear, camFar);
            freeCam->RotateEulerAngles(camRot);
            cam = (Camera*)freeCam;
        }
        else
        {
            // Invalid camera!
            delete(cam);
            cam = nullptr;
            continue;
        }

        _cameras.push_back(cam);
    }
    cam = nullptr;

    // Read materials
    // TODO: Maybe make each of these its own conversion function in Structures.h
    for (YAML::Node matNode : _config["materials"])
    {
        Material material;

        // Get albedo texture
        std::string albedoPath = matNode["albedoPath"].as<std::string>();
        if (albedoPath != "")
        {
            ID3D11ShaderResourceView* albedo;
            CreateDDSTextureFromFile(_pd3dDevice, std::wstring(albedoPath.begin(), albedoPath.end()).c_str(), nullptr, &albedo); // Read DDS image file and write data to stack
            material.AlbedoTexture = albedo;
        }

        // Get specular map texture
        std::string specularMapPath = matNode["specularMapPath"].as<std::string>();
        if (specularMapPath != "")
        {
            ID3D11ShaderResourceView* specularMap;
            CreateDDSTextureFromFile(_pd3dDevice, std::wstring(specularMapPath.begin(), specularMapPath.end()).c_str(), nullptr, &specularMap); // Read DDS image file and write data to stack
            material.SpecularMapTexture = specularMap;
        }

        // Set reflectivity variables
        material.AmbientReflectivity = matNode["ambient"].as<XMFLOAT4>();
        material.DiffuseReflectivity = matNode["diffuse"].as<XMFLOAT4>();
        material.SpecularReflectivity = matNode["specular"].as<XMFLOAT4>();
        material.SpecularPower = matNode["specularPower"].as<float>();

        // Set translucent var
        material.IsTranslucent = matNode["isTranslucent"].as<bool>();

        // Add material to the map, called by its name
        std::string matName = matNode["name"].as<std::string>();
        _materials[matName] = material;
    }

    // Read game objects
    for (YAML::Node goNode : _config["gameObjects"])
    {
        GameObject* go = new GameObject();

        go->SetMeshData(OBJLoader::Load(const_cast<char*>(goNode["modelPath"].as<std::string>().c_str()), _pd3dDevice, false));
        go->SetMaterial(_materials[goNode["material"].as<std::string>()]);
        go->SetPosition(goNode["position"].as<XMFLOAT3>());
        go->SetScale(goNode["scale"].as<XMFLOAT3>());
        go->SetRotation(goNode["rotation"].as<XMFLOAT3>());

        // Sort translucent objects
        if (go->GetMaterial()->IsTranslucent)
        {
            _translucentGameObjects.push_back(go);
        }
        else
        {
            // Load gameobject into the vector
            _gameObjects.push_back(go);
        }
    }

    // Read ambient light
    _ambientLight = _config["lighting"]["ambientLight"].as<XMFLOAT4>();

    // Read lights
    int i = 0;
    for (YAML::Node dlNode : _config["lighting"]["directionalLights"])
    {
        Light light;
        light.lightType = DIRECTIONAL_LIGHT;

        light.Color = dlNode["color"].as<XMFLOAT4>();
        light.Direction = dlNode["direction"].as<XMFLOAT3>();

        _lights[i] = light;
        i++;
    }

    // Read point lights
    for (YAML::Node plNode : _config["lighting"]["pointLights"])
    {
        Light light;
        light.lightType = POINT_LIGHT;

        light.Color = plNode["color"].as<XMFLOAT4>();
        light.Position = plNode["position"].as<XMFLOAT3>();
        light.Attenuation = plNode["attenuation"].as<float>();

        _lights[i] = light;
        i++;
    }

    // Read spotlights
    for (YAML::Node slNode : _config["lighting"]["spotLights"])
    {
        Light light;
        light.lightType = SPOT_LIGHT;

        light.Color = slNode["color"].as<XMFLOAT4>();
        light.Position = slNode["position"].as<XMFLOAT3>();
        light.Attenuation = slNode["attenuation"].as<float>();
        light.Direction = slNode["direction"].as<XMFLOAT3>();
        light.SpotAngle = slNode["maxAngle"].as<float>();

        _lights[i] = light;
        i++;
    }
    _numLights = i;

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
    RECT rc = {0, 0, 1920, 1080};
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
    Time::Update();

    // Sort GameObjects into order of distance to camera and tranclucency:

    // OK, pause. I know you're wondering why there is a lambda here.
    // Don't worry about it. It's all okay. Everything is fine.
    std::sort(_translucentGameObjects.begin(),
        _translucentGameObjects.end(),
        [this](GameObject* lhs, GameObject* rhs)
        {
            return CompareDistanceToCamera(lhs, rhs);
        });

    _orderedGameObjects = _gameObjects;
    _orderedGameObjects.insert(_orderedGameObjects.end(), _translucentGameObjects.begin(), _translucentGameObjects.end());

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

    // Update the camera
    _currentCamera->Update();

    // Update GameObjects
    for (GameObject* go : _orderedGameObjects)
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

    for (GameObject* go : _orderedGameObjects)
    {
        // Set the world matrix
        XMMATRIX world = XMLoadFloat4x4(go->GetWorld());

        // Make a constant buffer template with new shader variable values
        ConstantBuffer cb;
        cb.World = XMMatrixTranspose(world);
        cb.View = XMMatrixTranspose(view);

        cb.Projection = XMMatrixTranspose(projection);

        cb.AmbientLight = _ambientLight;

        cb.fog = _fog;

        std::copy(std::begin(_lights), std::end(_lights), std::begin(cb.lights)); // copy lights to constant buffer
        cb.numLights = _numLights;

        cb.AmbMat = go->GetMaterial()->AmbientReflectivity;
        cb.DiffMat = go->GetMaterial()->DiffuseReflectivity;
        cb.SpecMat = go->GetMaterial()->SpecularReflectivity;

        cb.specularPower = go->GetMaterial()->SpecularPower;

        cb.EyePosW = _currentCamera->GetPosition();
        
        // Check for albedo texture
        if ((go->GetMaterial()->AlbedoTexture) != nullptr)
        {
            // Set the textures to use
            _pImmediateContext->PSSetShaderResources(0, 1, &go->GetMaterial()->AlbedoTexture); // Assign texture slot
            cb.hasAlbedoTextue = 1;
        }
        else
        {
            cb.hasAlbedoTextue = 0;
        }

        // Check for specular map texture
        if ((go->GetMaterial()->SpecularMapTexture) != nullptr)
        {
            // Set the textures to use
            _pImmediateContext->PSSetShaderResources(1, 1, &go->GetMaterial()->SpecularMapTexture); // Assign texture slot
            cb.hasSpecularMapTextue = 1;
        }
        else
        {
            cb.hasSpecularMapTextue = 0;
        }

        // Update the shader variables using the constant buffer struct
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

        // Make stride and offset
        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;

        // Fill the index and vertex buffers with the index list and the vertices of the model respectively.
        _pImmediateContext->IASetVertexBuffers(0, 1, &go->GetMeshData()->VertexBuffer, &go->GetMeshData()->VBStride, &go->GetMeshData()->VBOffset);
        _pImmediateContext->IASetIndexBuffer(go->GetMeshData()->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // Set the shaders to use while drawing the model
        _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
        _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);

        // Set the constant buffer on each shader so it uses the piped in data
        _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
        _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);        

        // DRAW!
        _pImmediateContext->DrawIndexed(go->GetMeshData()->IndexCount, 0, 0);
    }
        

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(1, 0);
}