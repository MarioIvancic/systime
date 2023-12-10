# System time tick

The systime library is infrastructure for time measurement on small embedded systems.Systime must have access to some time source. It can be free running timer or variable incremented in periodic interrupt.

### Implemented functions

Three key functions in systime are `systime_tick()`, `systime_ms()` and `systime_sec()`.:
```
void systime_tick_init(unsigned (*fcn)(void), unsigned hw_bits, unsigned tick_multiplier);
void systime_time_init(unsigned ticks_for_1ms);
unsigned systime_tick(void);
unsigned systime_ms(void);
unsigned systime_sec(void);
```

### Implemented macros
```
systime_tick_elapsed(start)
systime_tick_expired(start, interval)
systime_ms_elapsed(start)
systime_ms_expired(start, interval)
systime_sec_elapsed(start)
systime_sec_expired(start, interval)
```

Function `systime_tick()` returns state of hardware timer widened to unsigned. This function measure time in units of internal timer ticks. This function have finest time resolution.

Function `systime_ms()` return current value of milliseconds free running counter.

Function `systime_sec()` return current value of seconds free running counter.

Note: it is important to call some of systime time functions (systime_tick(), systime_ms(), systime_sec()) at least once in timer full period. Otherwise, systime will lose some time.

Note: If tick_multiplier is not 1 there will be some error in miliseconds, but we use Bresenham's Algorithm so average error will be 0.
See https://www.romanblack.com/one_sec.htm

### Example use

Timer clock is 1MHz, timer register is 16 bits and unsigned is 32 bits:
```
systime_tick_init(timer_read, 16, 1):
systime_time_init(1);
```

Timer clock is 10MHz, timer register is 32 bits and unsigned is 32 bits:
```
systime_tick_init(timer_read, 32, 1):
systime_time_init(10000);
```

Timer clock is 11.0592MHz, timer register is 16 bits and unsigned is 32 bits:
```
systime_tick_init(timer_read, 16, 10):
systime_time_init(110592);
```
