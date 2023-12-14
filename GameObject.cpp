#include "GameObject.h"

GameObject::GameObject()
{
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
}

void GameObject::Update()
{
	XMStoreFloat4x4(&_world, XMLoadFloat4x4(&_trans) * XMLoadFloat4x4(&_rot) * XMLoadFloat4x4(&_scale));
}

void GameObject::SetPosition(XMFLOAT3 newPosition)
{
	XMStoreFloat4x4(&_trans, XMMatrixTranslation(newPosition.x, newPosition.y, newPosition.z));
}

void GameObject::SetRotation(XMFLOAT3 newRotation)
{
	newRotation.x = XMConvertToRadians(newRotation.x);
	newRotation.y = XMConvertToRadians(newRotation.y);
	newRotation.z = XMConvertToRadians(newRotation.z);

	XMStoreFloat4x4(&_rot, XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&newRotation)));
}

void GameObject::SetScale(XMFLOAT3 newScale)
{
	XMStoreFloat4x4(&_scale, XMMatrixScaling(newScale.x, newScale.y, newScale.z));
}

void GameObject::RotateOnAxes(XMFLOAT3 rotationDegrees)
{
	rotationDegrees.x = XMConvertToRadians(rotationDegrees.x);
	rotationDegrees.y = XMConvertToRadians(rotationDegrees.y);
	rotationDegrees.z = XMConvertToRadians(rotationDegrees.z);

	XMStoreFloat4x4(&_rot, XMLoadFloat4x4(&_rot) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotationDegrees)));
}

XMFLOAT3 GameObject::GetPosition()
{
	return XMFLOAT3(_trans._14, _trans._24, _trans._34);
}

float GameObject::GetDistanceToCamera(BaseCamera* camera)
{
	float distance;
	XMStoreFloat(&distance, XMVector3Length(XMLoadFloat3(&this->GetPosition()) - XMLoadFloat3(&camera->GetPosition())));
	return distance;
}
