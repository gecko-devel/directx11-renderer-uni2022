#pragma once
#include "BaseCamera.h"
class LookToCamera : public BaseCamera
{
protected:
	XMFLOAT3 _forward;
	
public:
	LookToCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 to, UINT windowWidth, UINT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	void Update();

	void SetTo(XMFLOAT3 newTo);
	XMFLOAT3 GetTo() { return _forward; }
};

