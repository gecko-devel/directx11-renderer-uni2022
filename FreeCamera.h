#pragma once
#include "Camera.h"
class FreeCamera : public Camera
{
public:
	FreeCamera(XMFLOAT3 position, XMFLOAT3 to, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, LookVector lookVector, float cameraSpeed);

	void Update(float* deltaTime);

private:
	float _cameraSpeed;
};

