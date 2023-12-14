#include "Camera.h"

Camera::Camera(XMFLOAT3 pos, UINT windowWidth, UINT windowHeight, float nearDepth, float farDepth)
{
	_pos = pos;
	_windowHeight = windowHeight;
	_windowWidth = windowWidth;
	_nearDepth = nearDepth;
	_farDepth = farDepth;

	XMStoreFloat4x4(&_view, XMMatrixIdentity());
	XMStoreFloat4x4(&_projection, XMMatrixIdentity());

	_right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	_forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
}

Camera::~Camera() {}

void Camera::Update()
{
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(70.0f), (float)_windowWidth / (float)_windowHeight, _nearDepth, _farDepth));
}

void Camera::RotateEulerAngles(XMFLOAT3 rotationAxes)
{
	//Apply a rotation, build a rotation matrix
	//Find the up, save into _up
	//Find the right, save into _right
	//Find the forward, save into _forward
	//..

	rotationAxes.x = XMConvertToRadians(rotationAxes.x);
	rotationAxes.y = XMConvertToRadians(rotationAxes.y);
	rotationAxes.z = XMConvertToRadians(rotationAxes.z);

	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotationAxes));

	// Update right, up, and forwards, so the basis stays.
	// As a reminder: a basis is just three vectors representing forward, up, and right.
	XMStoreFloat3(&_right, XMVector3TransformNormal(XMLoadFloat3(&_right), rot));
	XMStoreFloat3(&_up, XMVector3TransformNormal(XMLoadFloat3(&_up), rot));
	XMStoreFloat3(&_forward, XMVector3TransformNormal(XMLoadFloat3(&_forward), rot));

	XMStoreFloat4x4(&_view, XMMatrixLookToLH(XMLoadFloat3(&_pos), XMLoadFloat3(&_forward), XMLoadFloat3(&_up)));
}

void Camera::Move(XMFLOAT3 velocity)
{

	XMStoreFloat3(&_pos, XMLoadFloat3(&_pos) + XMLoadFloat3(&velocity));
}

// Not sure this works yet...
void Camera::LookAt(XMFLOAT3 newCameraPos, XMFLOAT3 targetWorldPos, XMFLOAT3 newUp)
{
	XMMATRIX newRot = XMMatrixLookAtLH(XMLoadFloat3(&newCameraPos), XMLoadFloat3(&targetWorldPos), XMLoadFloat3(&newUp));

	XMMATRIX rotDelta = newRot - XMLoadFloat4x4(&_view);

	// Set basis to track up, right, and forward
	XMStoreFloat3(&_right, XMVector3TransformNormal(XMLoadFloat3(&_right), rotDelta));
	XMStoreFloat3(&_up, XMVector3TransformNormal(XMLoadFloat3(&_up), rotDelta));
	XMStoreFloat3(&_forward, XMVector3TransformNormal(XMLoadFloat3(&_forward), rotDelta));

	// Set view
	XMStoreFloat4x4(&_view, newRot);
}
