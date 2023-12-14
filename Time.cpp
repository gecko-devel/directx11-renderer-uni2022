#include "Time.h"

float Time::previousTime = 0.0f;
float Time::time = 0.0f;
float Time::deltaTime = 0.0f;

void Time::Update()
{
    previousTime = time;

    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();

    if (dwTimeStart == 0)
        dwTimeStart = dwTimeCur;

    time = (dwTimeCur - dwTimeStart) / 1000.0f;

    deltaTime = time - previousTime;
}