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

void GameObject::SetScale(XMFLOAT3 newScale)
{
	XMStoreFloat4x4(&_scale, XMMatrixScaling(newScale.x, newScale.y, newScale.z));
}

void GameObject::SetRotation(XMFLOAT3 newRotation)
{
	XMStoreFloat4x4(&_rot, XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&newRotation)));
}
