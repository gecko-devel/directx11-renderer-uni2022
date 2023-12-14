#include "GameObject.h"

GameObject::GameObject()
{
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
}

void GameObject::SetPosition(XMFLOAT3 newPosition)
{
	_world.m[3][0] = newPosition.x;
	_world.m[3][1] = newPosition.y;
	_world.m[3][2] = newPosition.z;
}

void GameObject::SetScale(XMFLOAT3 newScale)
{
	XMVECTOR oldScaleVec;
	XMVECTOR newScaleVec = XMLoadFloat3(&newScale);
	XMMATRIX worldMatrix = XMLoadFloat4x4(&_world);

	// Calculate the scale by getting the sum of each column
	float oldScale[3];
	for (int i = 0; i < 3; i++)
	{
		float sum = 0.0f;
		for (float f : _world.m[0])
		{
			sum += f;
		}
		oldScale[i] = sum;
	}

	oldScaleVec = XMLoadFloat3(&XMFLOAT3(oldScale[0], oldScale[1], oldScale[2]));

	XMStoreFloat4x4(&_world, worldMatrix * XMMatrixScalingFromVector(newScaleVec - oldScaleVec));
}