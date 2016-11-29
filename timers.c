
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "error.h"
#include "interrupt.h"
#include "dmtimer.h"
#include "watchdog.h"
#include <stdint.h>

#include "leds.h"
#include "timers.h"
#include "joystick.h"

#define TIMER_INITIAL_COUNT 0xFFFC2F6F
#define TIMER_RLD_COUNT TIMER_INITIAL_COUNT

static void DMTimerAintcConfigure(void);
static void DMTimerSetup(void);
static void DMTimerIsr(void);

static volatile _Bool flagIsr = 0;
static _Bool hitWatchdog = true;

void Timers_timerInit() 
{
	DMTimer2ModuleClkConfig();

	IntMasterIRQEnable();

	DMTimerAintcConfigure();

	DMTimerSetup();

	DMTimerPreScalerClkDisable(SOC_DMTIMER_2_REGS);

	DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);

	DMTimerEnable(SOC_DMTIMER_2_REGS);
}

_Bool Timers_isIsrFlagSet()
{
	return flagIsr;
}

void Timers_clearIsrFlag() 
{
	flagIsr = false;
}

static void DMTimerIsr()
{
	DMTimerIntDisable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);

	DMTimerIntStatusClear(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_IT_FLAG);

	flagIsr = true;

	Leds_doWork();

	Joystick_buttonPressed();

	DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
}

static void DMTimerAintcConfigure()
{
	IntRegister(SYS_INT_TINT2, DMTimerIsr);

	IntPrioritySet(SYS_INT_TINT2, 0, AINTC_HOSTINT_ROUTE_IRQ);

	IntSystemEnable(SYS_INT_TINT2);
}

static void DMTimerSetup()
{
	DMTimerCounterSet(SOC_DMTIMER_2_REGS, TIMER_INITIAL_COUNT);

	DMTimerReloadSet(SOC_DMTIMER_2_REGS, TIMER_RLD_COUNT);

	DMTimerModeConfigure(SOC_DMTIMER_2_REGS, DMTIMER_AUTORLD_NOCMP_ENABLE);
}

#define WD_CLOCK (32000L)
#define WD_TIMEOUT_S (5)
#define WD_TIMEOUT_TICKS (WD_TIMEOUT_S * WD_CLOCK)
#define WD_RESET_VALUE ((uint32_t)0xFFFFFFFF - WD_TIMEOUT_TICKS + 1)

void Timers_watchdogInit()
{
	WatchdogTimer1ModuleClkConfig();
	WatchdogTimerReset(SOC_WDT_1_REGS);
	WatchdogTimerDisable(SOC_WDT_1_REGS);
	WatchdogTimerPreScalerClkDisable(SOC_WDT_1_REGS);
	WatchdogTimerReloadSet(SOC_WDT_1_REGS, WD_RESET_VALUE);
	WatchdogTimerEnable(SOC_WDT_1_REGS);
}

void Timers_hitWatchdog()
{
	static uint32_t triggerCounter = 0;

	triggerCounter++;

	WatchdogTimerTriggerSet(SOC_WDT_1_REGS, triggerCounter);
}

_Bool Timers_shouldHitWatchdog()
{
	return hitWatchdog;
}

void Timers_disableWatchdogHit()
{
	hitWatchdog = false;
}