#include "../header/timer.h"

#include <SDL.h>

#include <stdio.h>


Timer::Timer() : prevTimePoint(0){}

void Timer::start()
{
    // Get current time point that will be the previous time point when stop() is called
    prevTimePoint = SDL_GetPerformanceCounter();
}

float Timer::getSeconds()
{
    // Get current time point
    Uint64 currentTimePoint = SDL_GetPerformanceCounter();

    // Find the seconds between the current time and past time point
    float deltaTime = static_cast<float>(static_cast<double>(currentTimePoint - prevTimePoint) / static_cast<double>(SDL_GetPerformanceFrequency()));
    return deltaTime;
}
