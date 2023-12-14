#pragma once
#include "BaseCamera.h"
class LookToCamera : BaseCamera
{
private:
	XMFLOAT3 _to;
	
public:
	LookToCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 to, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	void Update();

	void SetTo(XMFLOAT3 newTo);
	XMFLOAT3 GetTo() { return _to; }
};

