#include "FreeCamera.h"

FreeCamera::FreeCamera(float speed, float rotationSpeed, XMFLOAT3 pos, UINT windowWidth, UINT windowHeight, float nearDepth, float farDepth)
	: Camera(pos, windowWidth, windowHeight, nearDepth, farDepth)
{
	_speed = speed;
	_rotationSpeed = rotationSpeed;
}

FreeCamera::~FreeCamera() {}

void FreeCamera::Update()
{
	// Translation
	MoveZ(Input::GetVerticalAxis() * _speed * Time::GetDeltaTime());
	MoveX(Input::GetHorizontalAxis() * _speed * Time::GetDeltaTime());
	MoveY(Input::GetUpDownAxis() * _speed * Time::GetDeltaTime());


	// Rotation
	XMFLOAT2 rotationInput = Input::GetLookInputVector();
	XMFLOAT2 rotationalVelocity;
	XMStoreFloat2(&rotationalVelocity, XMVector3Normalize(XMLoadFloat2(&rotationInput)) * _rotationSpeed * Time::GetDeltaTime());

	Pitch(rotationalVelocity.x);
	Yaw(rotationalVelocity.y);

	Camera::Update();
}

void FreeCamera::MoveZ(float distance)
{
	XMVECTOR distanceRepVec = XMVectorReplicate(distance);
	XMVECTOR forwardVec = XMLoadFloat3(&_forward);
	XMVECTOR posVec = XMLoadFloat3(&_pos);
	XMStoreFloat3(&_pos, XMVectorMultiplyAdd(distanceRepVec, forwardVec, posVec));
}

void FreeCamera::MoveX(float distance)
{
	XMVECTOR distanceRepVec = XMVectorReplicate(distance);
	XMVECTOR rightVec = XMLoadFloat3(&_right);
	XMVECTOR posVec = XMLoadFloat3(&_pos);
	XMStoreFloat3(&_pos, XMVectorMultiplyAdd(distanceRepVec, rightVec, posVec));
}

void FreeCamera::MoveY(float distance)
{
	XMVECTOR distanceRepVec = XMVectorReplicate(distance);
	XMVECTOR upVec = XMLoadFloat3(&_up);
	XMVECTOR posVec = XMLoadFloat3(&_pos);
	XMStoreFloat3(&_pos, XMVectorMultiplyAdd(distanceRepVec, upVec, posVec));
}

void FreeCamera::Pitch(float angle)
{
	// Rotate up and forward around the right.
	XMMATRIX rot = XMMatrixRotationAxis(XMLoadFloat3(&_right), XMConvertToRadians(angle));

	XMStoreFloat3(&_up, XMVector3TransformNormal(XMLoadFloat3(&_up), rot));
	XMStoreFloat3(&_forward, XMVector3TransformNormal(XMLoadFloat3(&_forward), rot));
}

void FreeCamera::Yaw(float angle)
{
	XMMATRIX rot = XMMatrixRotationY(XMConvertToRadians(angle));

	XMStoreFloat3(&_right, XMVector3TransformNormal(XMLoadFloat3(&_right), rot));
	XMStoreFloat3(&_up, XMVector3TransformNormal(XMLoadFloat3(&_up), rot));
	XMStoreFloat3(&_forward, XMVector3TransformNormal(XMLoadFloat3(&_forward), rot));
}
