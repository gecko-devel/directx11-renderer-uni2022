#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 to, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, LookVector lookVector)
{
	_position = position;
	_to = to;
	_up = up;
	_windowHeight = windowHeight;
	_windowWidth = windowWidth;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
	_lookVector = lookVector;

	Camera::Update();
}

Camera::~Camera() {}

void Camera::Update()
{
	// Create the view matrix
	XMVECTOR positionVec = XMVectorSet(_position.x, _position.y, _position.z, 0.0f);
	XMVECTOR toVec = XMVectorSet(_to.x, _to.y, _to.z, 0.0f);
	XMVECTOR upVec = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);

	// Decide between using LookTo or LookAt
	if (_lookVector == LookVector::To)
	{
		// If length of vector is higher than 1, normalise it.
		// 
		// XMVector3Length() doesn't actually return a float with the length of the vector - it actually returns
		// *another* XMVECTOR with the length slapped into each component. So we need to extract it from one of
		// those components using XMVectorGetX(). Why? Because Microsoft.
		if (XMVectorGetX(XMVector3Length(toVec)) >= 1.0f)
			toVec = XMVector3Normalize(toVec); // In case the user passes a value higher
		
		XMStoreFloat4x4(&_view, XMMatrixLookToLH(positionVec, toVec, upVec));
	}
	else
	{
		XMStoreFloat4x4(&_view, XMMatrixLookAtLH(positionVec, toVec, upVec));
	}

	// Create the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, 0.01f, 100.0f));
}

void Camera::SetPosition(XMFLOAT3 newPosition)
{
	_position = newPosition;
}

void Camera::SetTo(XMFLOAT3 newTo)
{
	_to = newTo;
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
