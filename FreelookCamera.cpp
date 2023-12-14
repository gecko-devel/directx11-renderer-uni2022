#include "FreelookCamera.h"

FreelookCamera::FreelookCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 to, UINT windowWidth, UINT windowHeight, FLOAT nearDepth, FLOAT farDepth, float speed) : LookToCamera(position, up, to, windowWidth, windowHeight, nearDepth, farDepth)
{
	_speed = speed;
	_rotationSpeed = 5.0f;
}

FreelookCamera::~FreelookCamera() {}

void FreelookCamera::Update()
{
	XMFLOAT3 moveInput = Input::Get3DInputVector();
	XMFLOAT2 lookInput = Input::GetLookInputVector();

	// Movement
	XMFLOAT3 freeCamVelocity;
	XMStoreFloat3(&freeCamVelocity, XMVector3Normalize(XMLoadFloat3(&moveInput)) * _speed * Time::GetDeltaTime());
	XMStoreFloat3(&_position, XMLoadFloat3(&_position) + XMLoadFloat3(&freeCamVelocity));
	
	// Rotation
	XMFLOAT2 freeCamRotVelocity;
	XMStoreFloat2(&freeCamRotVelocity, XMVector2Normalize(XMLoadFloat2(&lookInput)) * _rotationSpeed * Time::GetDeltaTime());
	XMStoreFloat3(&_forward, XMVector3Rotate(XMLoadFloat3(&_forward), XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), freeCamRotVelocity.y)));

	XMStoreFloat3(&_forward, XMVector3Rotate(XMLoadFloat3(&_forward), XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), freeCamRotVelocity.x)));


	LookToCamera::Update();
}

