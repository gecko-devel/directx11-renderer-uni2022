#include "Input.h"

int Input::GetVerticalAxis()
{
    return  (int)(bool)GetAsyncKeyState(0x57) - (int)(bool)GetAsyncKeyState(0x53); // W and S
}

int Input::GetHorizontalAxis()
{
    return (int)(bool)GetAsyncKeyState(0x44) - (int)(bool)GetAsyncKeyState(0x41); // D and A
}

int Input::GetUpDownAxis()
{
    return (int)(bool)GetAsyncKeyState(0x45) - (int)(bool)GetAsyncKeyState(0x51); // Q and E
}

XMFLOAT3 Input::Get3DInputVector()
{
    return XMFLOAT3(Input::GetHorizontalAxis(), Input::GetUpDownAxis(), Input::GetVerticalAxis());
}

int Input::GetXAxis()
{
    return (int)(bool)GetAsyncKeyState(VK_DOWN) - (int)(bool)GetAsyncKeyState(VK_UP); // Down and Up arrows
}

int Input::GetYAxis()
{
    return (int)(bool)GetAsyncKeyState(VK_RIGHT) - (int)(bool)GetAsyncKeyState(VK_LEFT); // Left and Right arrows
}

XMFLOAT2 Input::GetLookInputVector()
{
    return XMFLOAT2(Input::GetXAxis(), Input::GetYAxis());
}
