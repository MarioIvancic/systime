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

// systime.h

#ifndef __SYSTIME_H__
#define __SYSTIME_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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

#include <stdint.h>

typedef struct systime3_s
{
    uint32_t sec;
    unsigned msec;
    unsigned ticks;
} systime3_t;


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
void systime_init(unsigned (*fcn)(void), unsigned hw_bits, unsigned tick_multiplier, unsigned ticks_for_1ms);


// returns current systime internal tick count
// it have period of full unsigned int
unsigned systicks(void);


// fills systime3_t structure (if pointer is not NULL) with curent 
// internal ticks, miliseconds and seconds.
// returns current miliseconds
unsigned systime3(systime3_t* t);


// fills systime3_t structure (if pointer is not NULL) with last
// internal ticks, miliseconds and seconds.
// returns current miliseconds
// there will be some small error in ticks
unsigned systimelast3(systime3_t* t);


// returns current miliseconds
static inline unsigned systime_ms(void)
{
    return systime3(0);
}


// returns last miliseconds
static inline unsigned systimelast_ms(void)
{
    return systimelast3(0);
}


// returns current seconds
uint32_t systime(void);


// returns last seconds
uint32_t systimelast(void);


// Adjust internal offset for adj seconds
// Calculate adj as: adj = new_time - old_time
void systime_adj(uint32_t adj);


// Set current time to now seconds
// It will internaly adjust offset for seconds
static inline void systime_set(uint32_t now)
{
    systime_adj(now - systime());
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SYSTIME_H__

