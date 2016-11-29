#include "uart_irda_cir.h"
#include "soc_AM335x.h"
#include "interrupt.h"
#include "beaglebone.h"
#include "consoleUtils.h"
#include <stdint.h>

#include "serial.h"
#include "leds.h"
#include "timers.h"

#define CHAR_0 48
#define CHAR_9 57

static void UARTIsr(void);
static void UartInterruptEnable(void);

static volatile uint8_t rxByte = 0;

void Serial_init()
{
	UartInterruptEnable();
}

void Serial_processInput()
{
	if(rxByte != 0) {
		ConsoleUtilsPrintf("\n");
		if(rxByte == '?') {
			Serial_printCommandList();
		} else if(rxByte >= CHAR_0 && rxByte <= CHAR_9) {
			ConsoleUtilsPrintf("Setting LED speed to %c\n", rxByte);
			Leds_updateSpeed(rxByte - CHAR_0);
		} else if(rxByte == 'a' || rxByte == 'A') {
			ConsoleUtilsPrintf("Changing to bounce mode.\n");
			Leds_setMode(MODE_A);
		} else if(rxByte == 'b' || rxByte == 'B') {
			ConsoleUtilsPrintf("Changing to bar mode.\n");
			Leds_setMode(MODE_B);
		} else if(rxByte == 'x' || rxByte == 'X') {
			ConsoleUtilsPrintf("No longer hitting the watchdog.\n");
			Timers_disableWatchdogHit();
		} else {
			ConsoleUtilsPrintf("Invalid command\n");
			Serial_printCommandList();
		}
	}
	rxByte = 0;
}

void Serial_printCommandList()
{
	ConsoleUtilsPrintf("Commands:\n");
	ConsoleUtilsPrintf(" ?\t: Display this help message\n");
	ConsoleUtilsPrintf(" 0-9\t: Set speed 0 (slow) to 9 (fast)\n");
	ConsoleUtilsPrintf(" a\t: Select pattern A (bounce)\n");
	ConsoleUtilsPrintf(" b\t: Select pattern B (bar)\n");
	ConsoleUtilsPrintf(" x\t: Stop hitting the watchdog\n");
	ConsoleUtilsPrintf(" BTN\t: Push-button to toggle mode\n");
}

static void UartInterruptEnable()
{
	IntMasterIRQEnable();

	IntRegister(SYS_INT_UART0INT, UARTIsr);

	IntPrioritySet(SYS_INT_UART0INT, 0, AINTC_HOSTINT_ROUTE_IRQ);

	IntSystemEnable(SYS_INT_UART0INT);

	UARTIntEnable(SOC_UART_0_REGS, (UART_INT_RHR_CTI));
}

static void UARTIsr()
{
	uint32_t intId = 0;

	intId = UARTIntIdentityGet(SOC_UART_0_REGS);

	switch(intId) {
		case UART_INTID_RX_THRES_REACH:
			rxByte = UARTCharGetNonBlocking(SOC_UART_0_REGS);
			UARTCharPutNonBlocking(SOC_UART_0_REGS, rxByte);
			break;
		default:
			break;
	}
}
