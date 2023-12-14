#pragma once
#include "Camera.h"
#include "Input.h"
#include "Time.h"

class FreeCamera : public Camera
{
public:
	// Inherit constructor
	FreeCamera(float speed, float rotationSpeed, XMFLOAT3 pos, UINT windowWidth, UINT windowHeight, float nearDepth, float farDepth);
	~FreeCamera();
	
	void Update();

	void MoveZ(float distance);
	void MoveX(float distance);
	void MoveY(float distance);

	void Pitch(float angle);
	void Yaw(float angle);

private:
	float _speed;
	float _rotationSpeed;
};

