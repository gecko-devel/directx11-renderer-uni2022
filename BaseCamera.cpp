#include "BaseCamera.h"

BaseCamera::BaseCamera(XMFLOAT3 position, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_position = position;
	_up = up;
	_windowHeight = windowHeight;
	_windowWidth = windowWidth;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
}

BaseCamera::~BaseCamera() {}

void BaseCamera::Update()
{
	// Create the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, 0.01f, 100.0f));
}

void BaseCamera::SetPosition(XMFLOAT3 newPosition)
{
	_position = newPosition;
}

void BaseCamera::SetUp(XMFLOAT3 newUp)
{
	_up = newUp;
}

XMFLOAT4X4 BaseCamera::GetViewProjectionCombined()
{
	// Convert float4x4 to matrix so that math can be performed
	XMMATRIX viewMatrix = XMLoadFloat4x4(&_view);
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&_projection);

	// Combine the two matrices
	XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;

	// Store them on the stack for returning
	XMFLOAT4X4 _viewProjectionCombined;
	XMStoreFloat4x4(&_viewProjectionCombined, viewProjectionMatrix);

	return _viewProjectionCombined;
}

void BaseCamera::ReShape(FLOAT newWindowWidth, FLOAT newWindowHeight, FLOAT newNearDepth, FLOAT newFarDepth)
{
	_windowWidth = newWindowWidth;
	_windowHeight = newWindowHeight;
	_nearDepth = newNearDepth;
	_farDepth = newFarDepth;
}
