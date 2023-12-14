#pragma once
#include "LookToCamera.h"
#include "Input.h"
#include "Time.h"

class FreelookCamera : public LookToCamera
{
public:
	FreelookCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 to, UINT windowWidth, UINT windowHeight, FLOAT nearDepth, FLOAT farDepth, float speed);
	~FreelookCamera();

	void Update();

private:
	float _speed;
};

