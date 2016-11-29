#include "soc_AM335x.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"
#include "consoleUtils.h"
#include "hw_cm_per.h"

#include "joystick.h"
#include "leds.h"

#define BUTTON_GPIO_2_BASE (SOC_GPIO_2_REGS)
#define BUTTON_PIN_LEFT (1)
#define BUTTON_GPIO_1_BASE (SOC_GPIO_1_REGS)
#define BUTTON_PIN_DOWN (14)
#define BUTTON_GPIO_0_BASE (SOC_GPIO_0_REGS)
#define BUTTON_PIN_UP (26)

static void initializeButtonPinLeft(void);
static void initializeButtonPinUp(void);
static void initializeButtonPinDown(void);
static _Bool readButtonLeft(void);
static _Bool readButtonUp(void);
static _Bool readButtonDown(void);

void Joystick_init()
{
	initializeButtonPinLeft();
	initializeButtonPinUp();
	initializeButtonPinDown();
}

static void GPIO2ModuleClkConfig(void)
{

    /* Writing to MODULEMODE field of CM_PER_GPIO1_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_MODULEMODE));
    /*
    ** Writing to OPTFCLKEN_GPIO_2_GDBCLK bit in CM_PER_GPIO2_CLKCTRL
    ** register.
    */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK;

    /*
    ** Waiting for OPTFCLKEN_GPIO_1_GDBCLK bit to reflect the desired
    ** value.
    */
    while(CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK));

    /*
    ** Waiting for IDLEST field in CM_PER_GPIO2_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_GPIO2_CLKCTRL_IDLEST_FUNC <<
           CM_PER_GPIO2_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
            CM_PER_GPIO2_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_GPIO_2_GDBCLK bit in CM_PER_L4LS_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK));
}

static void initializeButtonPinLeft() 
{
	GPIO2ModuleClkConfig();

	GPIOModuleEnable(BUTTON_GPIO_2_BASE);

	GPIOModuleReset(BUTTON_GPIO_2_BASE);

	GPIODirModeSet(BUTTON_GPIO_2_BASE, BUTTON_PIN_LEFT, GPIO_DIR_INPUT);

}

static void initializeButtonPinUp() 
{
	GPIO0ModuleClkConfig();

	GPIOModuleEnable(BUTTON_GPIO_0_BASE);

	GPIOModuleReset(BUTTON_GPIO_0_BASE);

	GPIODirModeSet(BUTTON_GPIO_0_BASE, BUTTON_PIN_UP, GPIO_DIR_INPUT);
}

static void initializeButtonPinDown() 
{
	// GPIO 1 already initialized in leds.c
	GPIODirModeSet(BUTTON_GPIO_1_BASE, BUTTON_PIN_DOWN, GPIO_DIR_INPUT);
}

void Joystick_buttonPressedLeft()
{
	static _Bool pressedLeft = false;
	if(readButtonLeft()) {
		pressedLeft = true;
	}
	if(pressedLeft && !readButtonLeft()) {
		Leds_swapMode();
		pressedLeft = false;
	}
}


void Joystick_buttonPressedUp()
{
	static _Bool pressedUp = false;
	if(readButtonUp()) {
		pressedUp = true;
	}
	if(pressedUp && !readButtonUp()) {
		Leds_updateSpeed(Leds_getSpeed() + 1);
		pressedUp = false;
	}
}

void Joystick_buttonPressedDown()
{
	static _Bool pressedDown = false;
	if(readButtonDown()) {
		pressedDown = true;
	}
	if(pressedDown && !readButtonDown()) {
		Leds_updateSpeed(Leds_getSpeed() - 1);
		pressedDown = false;
	}
}

static _Bool readButtonLeft()
{
	return GPIOPinRead(BUTTON_GPIO_2_BASE, BUTTON_PIN_LEFT) == 0;
}

static _Bool readButtonUp()
{
	return GPIOPinRead(BUTTON_GPIO_0_BASE, BUTTON_PIN_UP) == 0;
}

static _Bool readButtonDown()
{
	return GPIOPinRead(BUTTON_GPIO_1_BASE, BUTTON_PIN_DOWN) == 0;
}
