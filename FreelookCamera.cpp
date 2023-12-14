#include "FreelookCamera.h"

FreelookCamera::FreelookCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 to, UINT windowWidth, UINT windowHeight, FLOAT nearDepth, FLOAT farDepth, float speed) : LookToCamera(position, up, to, windowWidth, windowHeight, nearDepth, farDepth)
{
	_speed = speed;
}

FreelookCamera::~FreelookCamera() {}

void FreelookCamera::Update()
{
	XMFLOAT3 input = Input::Get3DInputVector();

	XMFLOAT3 freeCamVelocity;
	XMStoreFloat3(&freeCamVelocity, XMVector3Normalize(XMLoadFloat3(&input)) * _speed * Time::GetDeltaTime());
	XMStoreFloat3(&_position, XMLoadFloat3(&_position) + XMLoadFloat3(&freeCamVelocity));
	
	LookToCamera::Update();
}

