#pragma once
#include "BaseCamera.h"
class LookAtCamera : BaseCamera
{
public:
	LookAtCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 at, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	void Update();

	void SetAt(XMFLOAT3 newAt);
	XMFLOAT3 GetAt() { return _at; }

private:
	XMFLOAT3 _at;
};

