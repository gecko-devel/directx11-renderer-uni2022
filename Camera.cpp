#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_position = position;
	_at = at;
	_up = up;
	_windowHeight = windowHeight;
	_windowWidth = windowWidth;
	_nearDepth = nearDepth;
	_farDepth = farDepth;

	Camera::Update();
}

Camera::~Camera() {}

void Camera::Update()
{
	// Create the view matrix
	XMVECTOR positionVec = XMVectorSet(_position.x, _position.y, _position.z, 0.0f);
	XMVECTOR atVec = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);
	XMVECTOR upVec = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(positionVec, atVec, upVec));

	// Create the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, 0.01f, 100.0f));
}

void Camera::SetPosition(XMFLOAT3 newPosition)
{
	_position = newPosition;
}

void Camera::SetAt(XMFLOAT3 newAt)
{
	_at = newAt;
}

void Camera::SetUp(XMFLOAT3 newUp)
{
	_up = newUp;
}

XMFLOAT4X4 Camera::GetViewProjectionCombined()
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

void Camera::ReShape(FLOAT newWindowWidth, FLOAT newWindowHeight, FLOAT newNearDepth, FLOAT newFarDepth)
{
	_windowWidth = newWindowWidth;
	_windowHeight = newWindowHeight;
	_nearDepth = newNearDepth;
	_farDepth = newFarDepth;
}
