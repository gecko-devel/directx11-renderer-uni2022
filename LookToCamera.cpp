#include "LookToCamera.h"

LookToCamera::LookToCamera(XMFLOAT3 position, XMFLOAT3 up, XMFLOAT3 to, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth) : BaseCamera::BaseCamera(position, up, windowWidth, windowHeight, nearDepth, farDepth)
{
	_to = to;
}

void LookToCamera::Update()
{
	BaseCamera::Update();

	// Create the view matrix
	XMVECTOR positionVec = XMVectorSet(_position.x, _position.y, _position.z, 0.0f);
	XMVECTOR upVec = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);
	XMVECTOR toVec = XMVectorSet(_to.x, _to.y, _to.z, 0.0f);

	// If length of vector is higher than 1, normalise it.
	// 
	// XMVector3Length() doesn't actually return a float with the length of the vector - it actually returns
	// *another* XMVECTOR with the length slapped into each component. So we need to extract it from one of
	// those components using XMVectorGetX(). Why? Because Microsoft.
	if (XMVectorGetX(XMVector3Length(toVec)) >= 1.0f)
		toVec = XMVector3Normalize(toVec); // In case the user passes a value higher

	XMStoreFloat4x4(&_view, XMMatrixLookToLH(positionVec, toVec, upVec));

	BaseCamera::Update();
}

void LookToCamera::SetTo(XMFLOAT3 newTo)
{
	_to = newTo;
}
