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
	XMFLOAT3 input = Input::Get3DInputVector();
	XMFLOAT3 velocity;
	XMStoreFloat3(&velocity, XMVector3Normalize(XMLoadFloat3(&input)) * _speed * Time::GetDeltaTime());

	Move(velocity);

	XMFLOAT2 rotationInput = Input::GetLookInputVector();
	XMFLOAT2 rotationalVelocity;
	XMStoreFloat2(&rotationalVelocity, XMVector3Normalize(XMLoadFloat2(&rotationInput)) * _rotationSpeed * Time::GetDeltaTime());	
	RotateEulerAngles(XMFLOAT3(rotationalVelocity.x, rotationalVelocity.y, 0.0f));
	

	Camera::Update();
}
