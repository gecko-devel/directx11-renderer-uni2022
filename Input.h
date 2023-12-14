#pragma once
#include <DirectXMath.h>
#include <Windows.h>

using namespace DirectX;

static class Input
{
public:
    static int GetVerticalAxis();
    static int GetHorizontalAxis();
    static int GetUpDownAxis();
	static XMFLOAT3 Get3DInputVector();


};

