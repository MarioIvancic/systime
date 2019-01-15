/*  Copyright (c) 2018, Mario Ivancic
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "systime.h"

// define this if you have integer divide instruction
//#define SYSTIME_HAVE_DIV_INST


/*
    systime is infrastructure for time measurement on small embedded systems.
    Systime must have access to some time source. It can be free running timer or
    variable incremented in periodic interrupt.

    Two key functions in systime are systicks() and systime3().

    Function systicks() returns state of hardware timer widened to unsigned. This function
    measure time in units of timer ticks. This function have finest time resolution.

    Function systime3() fills systime3_t structure with current value of ticks, miliseconds
    and seconds. It returns current value of miliseconds.

    All other systime time functions are based on systime3().

    Before returning to caller seconds are adjusted by offset. By using functions systime_adj()
    and systime_set() caller can change offset and effectively seconds. It is not possible to
    addjust ticks and miliseconds.

    Note: it is important to call some of systime time functions (systicks(), systime3(),
    systime_ms(), systime()) at least once in timer full period. Otherwise, systime will
    lose some time.

    Note: If tick_multiplier is not 1 there will be some error in miliseconds, but we use
    Bresenham's Algorithm so average error will be 0.
    See https://www.romanblack.com/one_sec.htm
*/


static unsigned ticks;
static unsigned tickmult;
static unsigned mask;
static unsigned ticks1ms;
#if !defined(SYSTIME_HAVE_DIV_INST)
static unsigned ticks10ms;
static unsigned ticks100ms;
#endif // SYSTIME_HAVE_DIV_INST
static unsigned milisec;
static uint32_t sec;
static uint32_t sec_offset;
static int      use_hw_systicks;
static unsigned last_timer_ticks;
static unsigned last_systime_ticks;
static unsigned last_ms;
static unsigned (*systickshw)(void);

/*
    systime init function
    fcn is pointer to function that returns current timer state
    hw_bits is number of bits in timer register
    tick_multiplier is number of internal ticks that is added to state for every timer tick
    ticks_for_1ms is number of internal ticks for 1 milisecond

    Example: timer clock is 1MHz, timer register is 16 bits and unsigned is 32 bits
    systime_init(timer_read, 16, 1, 1000):

    Example: timer clock is 10MHz, timer register is 32 bits and unsigned is 32 bits
    systime_init(timer_read, 32, 1, 10000):

    Example: timer clock is 11.0592MHz, timer register is 16 bits and unsigned is 32 bits
    systime_init(timer_read, 16, 10, 110592):

    If tick_multiplier is not 1 there will be some error in miliseconds, but we use
    Bresenham's Algorithm so average error will be 0.
    See https://www.romanblack.com/one_sec.htm
*/
void systime_init(unsigned (*fcn)(void), unsigned hw_bits, unsigned tick_multiplier, unsigned ticks_for_1ms)
{
    // if timer register is wide as unsigned we can directly call systickshw() instead of systicks()
    if(hw_bits == 8 * sizeof(unsigned) && tick_multiplier == 1) use_hw_systicks = 1;
    else
    {
        use_hw_systicks = 0;
        mask = (1UL << hw_bits) - 1;
    }
    tickmult = tick_multiplier;

    systickshw = fcn;
    ticks1ms = ticks_for_1ms;
#if ! defined(SYSTIME_HAVE_DIV_INST)
    ticks10ms = 10 * ticks_for_1ms;
    ticks100ms = 100 * ticks_for_1ms;
#endif // SYSTIME_HAVE_DIV_INST

    systicks();
}


// returns current systime internal tick count
// it have period of full unsigned int
unsigned systicks(void)
{
    if(use_hw_systicks) return systickshw();
    else
    {
        unsigned now = systickshw();
        unsigned diff = (now - last_timer_ticks) & mask;
        ticks += diff * tickmult;
        last_timer_ticks = now;
        return ticks;
    }
}


// fills systime3_t structure (if pointer is not NULL) with curent
// internal ticks, miliseconds and seconds.
// returns current miliseconds
unsigned systime3(systime3_t* t)
{
    unsigned now;

    if(use_hw_systicks) now = systickshw();
    else now = systicks();

#if defined(SYSTIME_HAVE_DIV_INST)
    unsigned diff = (now - last_systime_ticks) / ticks1ms;
    milisec += diff;
    last_systime_ticks += diff * ticks1ms;

    diff = (milisec - last_ms) / 1000;
    sec += diff;
    last_ms += diff * 1000;
#else
    // we are using more while loops to have fewer iterations
    while((now - last_systime_ticks) > ticks100ms)
	{
		last_systime_ticks += ticks100ms;
		milisec += 100;
	}
    while((now - last_systime_ticks) > ticks10ms)
	{
		last_systime_ticks += ticks10ms;
		milisec += 10;
	}
    while((now - last_systime_ticks) > ticks1ms)
	{
		last_systime_ticks += ticks1ms;
		milisec++;
	}
    while((milisec - last_ms) > 1000)
	{
		last_ms += 1000;
		sec++;
	}
#endif  // SYSTIME_HAVE_DIV_INST

    if(t)
    {
        t->sec = sec + sec_offset;
        t->msec = milisec;
        t->ticks = now;
    }

	return milisec;
}



// fills systime3_t structure (if pointer is not NULL) with last
// internal ticks, miliseconds and seconds.
// returns current miliseconds
// there will be some small error in ticks
unsigned systimelast3(systime3_t* t)
{
	if(t)
    {
        t->sec = sec + sec_offset;
        t->msec = milisec;
        t->ticks = last_systime_ticks;
    }

	return milisec;
}


// returns current seconds
uint32_t systime(void)
{
    systime3_t t;
    systime3(&t);
	return t.sec;
}


// returns last seconds
uint32_t systimelast(void)
{
    systime3_t t;
    systimelast3(&t);
	return t.sec;
}


// Adjust internal offset for adj seconds
// Calculate adj as: adj = new_time - old_time
void systime_adj(uint32_t adj)
{
    sec_offset += adj;
}

