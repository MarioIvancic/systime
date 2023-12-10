// systime_tick.c

#include "systime_tick.h"


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


static unsigned systime_tick_internal(void);

static unsigned (*systickshw)(void);
static unsigned (*systime_tick_current)(void) = &systime_tick_internal;

static unsigned last_timer_ticks;
unsigned systime_curr_ticks;
static unsigned mask;
static unsigned tickmult;




// returns current system time internal tick count.
// it has period of full unsigned int.
unsigned systime_tick(void)
{
    return systime_tick_current();
}



//##############################################################################################

/*
    initialize systime tick

    fcn is pointer to function that returns current timer state.

    hw_bits is number of bits in timer state.

    tick_multiplier is number of internal ticks that is added to state for every timer tick.

    Example: timer clock is 1MHz, timer register is 16 bits and unsigned is 32 bits
    systime_tick_init(timer_read, 16, 1):

    Example: timer clock is 10MHz, timer register is 32 bits and unsigned is 32 bits
    systime_tick_init(timer_read, 32, 1):

    Example: timer clock is 11.0592MHz, timer register is 16 bits and unsigned is 32 bits
    systime_tick_init(timer_read, 16, 10):

    If tick_multiplier is not 1 there will be some error in miliseconds, but we use
    Bresenham's Algorithm so average error will be 0.
    See https://www.romanblack.com/one_sec.htm
*/
void systime_tick_init(unsigned (*fcn)(void), unsigned hw_bits, unsigned tick_multiplier)
{
    // if timer state is wide as unsigned we can directly call systickshw() instead of systime_tick()
    if(hw_bits == 8 * sizeof(unsigned) && tick_multiplier == 1)
	{
		systime_tick_current = fcn;
	}
    else
    {
		systime_tick_current = &systime_tick_internal;
		tickmult = tick_multiplier;
		systickshw = fcn;
        mask = (1UL << hw_bits) - 1;
    }
    
    systime_tick();
}



static unsigned systime_tick_internal(void)
{
	unsigned now = systickshw();
	unsigned diff = (now - last_timer_ticks) & mask;
	systime_curr_ticks += diff * tickmult;
	last_timer_ticks = now;
	return systime_curr_ticks;
}
