// Module defines a 10ms timer interrupt and sets the watchdog timeout to 5 seconds
#ifndef _TIMERS_H_
#define _TIMERS_H_
#include <stdbool.h>

// Timer controls
void Timers_timerInit(void);
_Bool Timers_isIsrFlagSet(void);
void Timers_clearIsrFlag(void);

// Watchdog timer controls
void Timers_watchdogInit(void);
void Timers_hitWatchdog(void);
_Bool Timers_shouldHitWatchdog(void);
void Timers_disableWatchdogHit();


#endif