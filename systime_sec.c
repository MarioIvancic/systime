// systime_sec.c

#include "systime_tick.h"

// define this if you have integer divide instruction
//#define SYSTEM_TIME_HAVE_DIV_INST


/*
    systime is infrastructure for time measurement on small embedded systems.
    Systime must have access to some time source. It can be free running timer or
    variable incremented in periodic interrupt.

    Three key functions in systime are systime_tick(), systime_ms() and systime_sec().

    Function systime_tick() returns state of hardware timer widened to unsigned. This function
    measure time in units of internal timer ticks. This function have finest time resolution.

    Function systime_ms() return current value of miliseconds free running counter.

    Function systime_sec() return current value of seconds free running counter.

    Note: it is important to call some of systime time functions (systime_tick(), systime_ms(),
    systime_sec()) at least once in timer full period. Otherwise, systime will lose some time.

    Note: If tick_multiplier is not 1 there will be some error in miliseconds, but we use
    Bresenham's Algorithm so average error will be 0.
    See https://www.romanblack.com/one_sec.htm
*/



unsigned systime_curr_sec;
static unsigned last_ms;

// returns current value of seconds free running counter
unsigned systime_sec(void)
{
#if defined(SYSTEM_TIME_HAVE_DIV_INST)
    unsigned diff = (systime_ms() - last_ms) / 1000U;
    systime_curr_sec += diff;
    last_ms += diff * 1000;
#else
    while((systime_ms() - last_ms) >= 1000)
    {
        last_ms += 1000;
        systime_curr_sec++;
    }
#endif  // SYSTEM_TIME_HAVE_DIV_INST

    return systime_curr_sec;
}

// set current seconds
void systime_sec_set(unsigned current_time)
{
    unsigned now = systime_sec();

    if(now > current_time)
    {
        systime_curr_sec -= (now - current_time);
    }
    else
    {
        systime_curr_sec += (current_time - now);
    }
}
