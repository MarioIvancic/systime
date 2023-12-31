// systime_tick.h

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



#ifndef __SYSTIME_TICK_H__
#define __SYSTIME_TICK_H__

#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

extern unsigned systime_curr_ticks;
extern unsigned systime_curr_ms;
extern unsigned systime_curr_sec;



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
void systime_tick_init(unsigned (*fcn)(void), unsigned hw_bits, unsigned tick_multiplier);


// returns current system time internal tick count
// it have period of full unsigned int
unsigned systime_tick(void);


/*
    initialize systime time

    ticks_for_1ms is number of internal ticks ( result of systime_tick() ) for 1 milisecond.

    Example: timer clock is 1kHz, timer register is 16 bits and unsigned is 32 bits
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
void systime_time_init(unsigned ticks_for_1ms);


// returns current value of miliseconds free running counter
unsigned systime_ms(void);


// returns current value of seconds free running counter
unsigned systime_sec(void);

// set current seconds
void systime_sec_set(unsigned current_time);

// number of elapsed ticks since start
#define systime_tick_elapsed(start) (systime_tick() - (start))

// true if elapsed >= interval ticks since start
#define systime_tick_expired(start, interval) ( (systime_tick() - (start) ) >= (interval) )

// number of elapsed ms since start
#define systime_ms_elapsed(start) (systime_ms() - (start))

// true if elapsed >= interval ms since start
#define systime_ms_expired(start, interval) ( (systime_ms() - (start) ) >= (interval) )

// number of elapsed seconds since start
#define systime_sec_elapsed(start) (systime_sec() - (start))

// true if elapsed >= interval seconds since start
#define systime_sec_expired(start, interval) ( (systime_sec() - (start) ) >= (interval) )



#ifdef __cplusplus
}
#endif // _cplusplus

#endif // __SYSTIME_TICK_H__
