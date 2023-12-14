#pragma once
#include <windows.h> // TODO: Figure out why this is needed to have FLOAT types.
#include <DirectXMath.h>

using namespace DirectX;

class BaseCamera
{
public:

	BaseCamera(XMFLOAT3 position, XMFLOAT3 up, UINT windowWidth, UINT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~BaseCamera();

	// To have the camera move every frame.
	virtual void Update();

	void SetPosition(XMFLOAT3 newPosition);
	XMFLOAT3 GetPosition() { return _position; }

	void SetUp(XMFLOAT3 newUp);
	XMFLOAT3 GetUp() { return _up; }

	XMFLOAT4X4 GetView() { return _view; }
	XMFLOAT4X4 GetProjection() { return _projection; }
	XMFLOAT4X4 GetViewProjectionCombined();

	// In case the window resizes at runtime
	void ReShape(FLOAT newWindowWidth, FLOAT newWindowHeight, FLOAT newNearDepth, FLOAT newFarDepth);

protected:
	// Camera projection values
	XMFLOAT3 _position;

	XMFLOAT3 _forward;
	XMFLOAT3 _up;
	XMFLOAT3 _right;

	FLOAT _windowHeight;
	FLOAT _windowWidth;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	// projection and view matrices to be passed to the shaders
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
};

