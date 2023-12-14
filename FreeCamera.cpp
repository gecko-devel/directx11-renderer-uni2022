#include "FreeCamera.h"

FreeCamera::FreeCamera(XMFLOAT3 position, XMFLOAT3 to, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, LookVector lookVector, float cameraSpeed) : Camera(position, to, up, windowWidth, windowHeight, nearDepth, farDepth, lookVector)
{
    _cameraSpeed = cameraSpeed;
}

void FreeCamera::Update(float* deltaTime)
{
    // Get input vector from WASD + QE for up/down
    XMFLOAT3 _input = XMFLOAT3((int)GetAsyncKeyState(0x41) - (int)GetAsyncKeyState(0x44), // A and D
                               (int)GetAsyncKeyState(0x51) - (int)GetAsyncKeyState(0x45), // Q and E
                               (int)GetAsyncKeyState(0x53) - (int)GetAsyncKeyState(0x57));// W and S

    // Move the Free camera
    XMFLOAT3 freeCamVelocity;
    XMStoreFloat3(&freeCamVelocity, XMVector3Normalize(XMLoadFloat3(&_input)) * (_cameraSpeed * *deltaTime));
    _position = XMFLOAT3(_position.x + freeCamVelocity.x, _position.y + freeCamVelocity.y, _position.z + freeCamVelocity.z);

    Camera::Update();
}
