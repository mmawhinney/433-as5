#include "soc_AM335x.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"     
#include "watchdog.h"
#include <stdint.h>
#include <stdbool.h>

#include "leds.h"

#define LED_GPIO_BASE	(SOC_GPIO_1_REGS)
#define LED0_PIN (21)
#define LED1_PIN (22)
#define LED2_PIN (23)
#define LED3_PIN (24)

#define LED_MASK ((1<<LED0_PIN) | (1<<LED1_PIN) | (1<<LED2_PIN) | (1<<LED3_PIN))

#define MAX_SPEED 9
#define DEFAULT_SPEED 5

static uint32_t speed;
static volatile uint32_t counter;
static uint32_t delay;
static uint8_t mode;
static uint32_t currentPin = LED0_PIN;

static void calcDelay(void);
static void bounceLeds(void);
static void barLeds(void);
static void turnAllLedsOff(void);

void Leds_init()
{
	GPIO1ModuleClkConfig();

	GPIOModuleEnable(LED_GPIO_BASE);

	GPIOModuleReset(LED_GPIO_BASE);

	GPIODirModeSet(LED_GPIO_BASE, LED0_PIN, GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE, LED1_PIN, GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE, LED2_PIN, GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE, LED3_PIN, GPIO_DIR_OUTPUT);

	speed = DEFAULT_SPEED;
	counter = 0;
	mode = MODE_A;
	calcDelay();
}

void Leds_setMode(uint8_t newMode) 
{
	mode = newMode;
	turnAllLedsOff();
}

void Leds_swapMode()
{
	if(mode == MODE_A) {
		Leds_setMode(MODE_B);
	} else if(mode == MODE_B) {
		Leds_setMode(MODE_A);
	}
}

void Leds_doWork()
{
	if(mode == MODE_A) {
		bounceLeds();
	} else if(mode == MODE_B) {
		barLeds();
	}
}
 
static void bounceLeds()
{
	static _Bool ledOn = false;
	static _Bool increment = true;
	counter++;
	if(ledOn == false) {
		GPIOPinWrite(LED_GPIO_BASE, currentPin, GPIO_PIN_HIGH);
		ledOn = true;
	} else {
		if(counter >= delay) {
			GPIOPinWrite(LED_GPIO_BASE, currentPin, GPIO_PIN_LOW);
			if(increment == true) {
				if(currentPin == LED3_PIN) {
					increment = false;
					currentPin--;
				} else {
					currentPin++;
				}
			} else {
				if(currentPin == LED0_PIN) {
					increment = true;
					currentPin++;
				} else {
					currentPin--;
				}
			}
			counter = 0;
			ledOn = false;
		}
	}
}

static void barLeds()
{
	static _Bool incrementBar = true;
	static uint32_t N = LED0_PIN;
	GPIOPinWrite(LED_GPIO_BASE, LED0_PIN, GPIO_PIN_HIGH);
	counter++;
	if(counter >= delay) {
		if(incrementBar == true) {
			GPIOPinWrite(LED_GPIO_BASE, N, GPIO_PIN_HIGH);
			if(N == LED3_PIN) {
				incrementBar = false;
				counter = 0;
				return;
			}
		} else {
			GPIOPinWrite(LED_GPIO_BASE, N, GPIO_PIN_LOW);
		}
		
		if(incrementBar == true) {
			if(N == LED3_PIN) {
				incrementBar = false;
				N--;
			} else {
				N++;
			}
		} else {
			if(N == LED1_PIN) {
				incrementBar = true;
			} else {
				N--;
			}
		}
		counter = 0;
	}
}

uint32_t Leds_getSpeed()
{
	return speed;
}

void Leds_updateSpeed(uint32_t newSpeed)
{
	if(newSpeed >= 0 && newSpeed <= 9) {
		speed = newSpeed;
	}
	counter = 0;
	calcDelay();
}

static void calcDelay()
{
	int i;
	int speedDivFactor = 1;
	for(i = 0; i < MAX_SPEED - speed; i++) {
		speedDivFactor *= 2;
	}
	delay = speedDivFactor;
}

static void turnAllLedsOff()
{
	HWREG(LED_GPIO_BASE + GPIO_DATAOUT) &= ~LED_MASK;
}