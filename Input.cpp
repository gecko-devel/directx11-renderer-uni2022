#include "Input.h"

int Input::GetVerticalAxis()
{
    return (int)GetAsyncKeyState(0x53) - (int)GetAsyncKeyState(0x57); // S and W
}

int Input::GetHorizontalAxis()
{
    return (int)GetAsyncKeyState(0x41) - (int)GetAsyncKeyState(0x44); // A and D
}

int Input::GetUpDownAxis()
{
    return (int)GetAsyncKeyState(0x51) - (int)GetAsyncKeyState(0x45); // Q and E
}

XMFLOAT3 Input::Get3DInputVector()
{
    return XMFLOAT3(Input::GetHorizontalAxis(), Input::GetUpDownAxis(), Input::GetVerticalAxis());
}

int Input::GetXAxis()
{
    return (int)GetAsyncKeyState(VK_UP) - (int)GetAsyncKeyState(VK_DOWN); // Down and Up arrows
}

int Input::GetYAxis()
{
    return (int)GetAsyncKeyState(VK_LEFT) - (int)GetAsyncKeyState(VK_RIGHT); // Left and Right arrows
}

XMFLOAT2 Input::GetLookInputVector()
{
    return XMFLOAT2(Input::GetXAxis(), Input::GetYAxis());
}
