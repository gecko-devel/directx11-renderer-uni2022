#pragma once
#include <windows.h>
#include <sysinfoapi.h>
class Time
{
public:
	static void Update();

	static float GetTime() { return time; }
	static float GetDeltaTime() { return deltaTime; }

private:
	static float previousTime;
	static float time;
	static float deltaTime;
};

