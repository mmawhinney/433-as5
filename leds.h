#ifndef _LEDS_H_
#define _LEDS_H_

void Leds_init(void);
void Leds_updateSpeed(uint32_t newSpeed);
uint32_t Leds_getSpeed(void);
void Leds_setMode(uint8_t newMode);
void Leds_doWork(void);


#endif