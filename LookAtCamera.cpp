#include "LookAtCamera.h"

LookAtCamera::LookAtCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 at, UINT windowWidth, UINT windowHeight, FLOAT nearDepth, FLOAT farDepth) : BaseCamera::BaseCamera(position, up, windowWidth, windowHeight, nearDepth, farDepth)
{

	_at = at;
}

void LookAtCamera::Update()
{
	// Create the view matrix
	XMVECTOR positionVec = XMVectorSet(_position.x, _position.y, _position.z, 0.0f);
	XMVECTOR upVec = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);
	XMVECTOR atVec = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(positionVec, atVec, upVec));

	BaseCamera::Update();
}

void LookAtCamera::SetAt(XMFLOAT3 newAt)
{
	_at = newAt;
}
