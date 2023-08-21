
#pragma once

#ifndef COMMON_H
#define COMMON_H

#define NO_OF_TIMERS 10
#define timeZone 5.5

struct TimerDataStruct
{
    uint8_t hourOn[NO_OF_TIMERS];
    uint8_t minuteOn[NO_OF_TIMERS];
    uint8_t hourOff[NO_OF_TIMERS];
    uint8_t minuteOff[NO_OF_TIMERS];
    bool timerOn[NO_OF_TIMERS];
    uint8_t gpioPin[NO_OF_TIMERS];
    bool activeHigh[NO_OF_TIMERS];
    bool allOff;
    float timezone;
    char hostname[10];
};

#endif 
