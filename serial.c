#include "uart_irda_cir.h"
#include "soc_AM335x.h"
#include "interrupt.h"
#include "beaglebone.h"
#include "consoleUtils.h"
#include "hw_types.h"
#include <stdint.h>
#include "watchdog.h"

#include "serial.h"



#define BAUD_RATE	115200
#define UART_MODULE_INPUT_CLK	48000000

static void UARTIsr(void);
static void UartInterruptEnable(void);
static void UartBaudRateSet(void);


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
		} else if(rxByte >= 48 && rxByte <= 57) {
			ConsoleUtilsPrintf("Set speed to %d\n", rxByte - 48);
		} else if(rxByte == 'a' || rxByte == 'A') {
			ConsoleUtilsPrintf("Set pattern a\n");
		} else if(rxByte == 'b' || rxByte == 'B') {
			ConsoleUtilsPrintf("Set pattern b\n");
		} else if(rxByte == 'x' || rxByte == 'X') {
			ConsoleUtilsPrintf("Disable hitting watchdog\n");
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

	// IntAINTCInit();

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
