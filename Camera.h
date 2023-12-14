#pragma once
#include <DirectXMath.h>
#include <Windows.h>

using namespace DirectX;

class Camera
{
public:
	Camera(XMFLOAT3 pos, UINT windowWidth, UINT windowHeight, float nearDepth, float farDepth);
	~Camera();

	virtual void Update();

	void RotateEulerAngles(XMFLOAT3 rotationAxes);
	void Move(XMFLOAT3 velocity);

	void LookAt(XMFLOAT3 newCameraPos, XMFLOAT3 targetWorldPos, XMFLOAT3 newUp);

	void SetPosition(XMFLOAT3 newPos) { _pos = newPos; }
	XMFLOAT3 GetPosition() { return _pos; }

	XMFLOAT4X4 GetView() { return _view; }
	XMMATRIX GetViewMatrix() { return XMLoadFloat4x4(&_view); }
	XMFLOAT4X4 GetProjection() { return _projection; }
	XMMATRIX GetProjectionMatrix() { return XMLoadFloat4x4(&_projection); }
	FLOAT GetNear() { return _nearDepth; }
	FLOAT GetFar() { return _farDepth; }

protected:
	XMFLOAT3 _pos;

	XMFLOAT3 _forward;
	XMFLOAT3 _up;
	XMFLOAT3 _right;

	unsigned int _windowHeight;
	unsigned int _windowWidth;
	float _nearDepth;
	float _farDepth;

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

	
};

