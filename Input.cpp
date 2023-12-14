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
