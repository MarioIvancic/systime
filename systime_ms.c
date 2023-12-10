// systime_ms.c

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



static unsigned lastsystime_ticks;
// we are dividing by ticks1ms so it must not be 0
static unsigned ticks1ms = 10000;
#if !defined(SYSTEM_TIME_HAVE_DIV_INST)
static unsigned ticks50ms;
#endif // SYSTEM_TIME_HAVE_DIV_INST
unsigned systime_curr_ms;

// returns current value of miliseconds free running counter
unsigned systime_ms(void)
{
    unsigned now = systime_tick();

#if defined(SYSTEM_TIME_HAVE_DIV_INST)
    unsigned diff = (now - lastsystime_ticks) / ticks1ms;
    systime_curr_ms += diff;
    lastsystime_ticks += diff * ticks1ms;
#else
    // we are using more while loops to have fewer iterations
    while((now - lastsystime_ticks) > ticks50ms)
	{
		lastsystime_ticks += ticks50ms;
		systime_curr_ms += 50;
	}
    while((now - lastsystime_ticks) > ticks1ms)
	{
		lastsystime_ticks += ticks1ms;
		systime_curr_ms++;
	}
#endif  // SYSTEM_TIME_HAVE_DIV_INST

    return systime_curr_ms;
}


//##############################################################################################


/*
    initialize systime time

    ticks_for_1ms is number of internal ticks ( result of systime_tick() ) for 1 milisecond.

    Example: timer clock is 1MHz, timer register is 16 bits and unsigned is 32 bits
    systime_tick_init(timer_read, 16, 1):
    systime_time_init(1);

    Example: timer clock is 10MHz, timer register is 32 bits and unsigned is 32 bits
    systime_tick_init(timer_read, 32, 1):
    systime_time_init(10000);

    Example: timer clock is 11.0592MHz, timer register is 16 bits and unsigned is 32 bits
    systime_tick_init(timer_read, 16, 10):
    systime_time_init(110592);

    we use Bresenham's Algorithm so average error will be 0.
    See https://www.romanblack.com/one_sec.htm
*/
void systime_time_init(unsigned ticks_for_1ms)
{
    ticks1ms = ticks_for_1ms;
#if ! defined(SYSTEM_TIME_HAVE_DIV_INST)
    ticks50ms = 50 * ticks_for_1ms;
#endif // SYSTEM_TIME_HAVE_DIV_INST

    systime_ms();
}

