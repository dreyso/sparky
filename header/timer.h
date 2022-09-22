#pragma once
#include <SDL.h>

#include <stdio.h>


class Timer
{
public:
    Timer();
    ~Timer() = default;

    // Records the starting point of the interval
    void start();

    // Returns seconds passed since last start
    // Will return arbitrary number if timer was never started
    float getSeconds();   

private:
    // Gives stime intervals if subtracted from likewise value, meaningless on its own
    Uint64 prevTimePoint;
};