#include "uart_irda_cir.h"
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "consoleUtils.h"
#include "hw_types.h"
#include "watchdog.h"
#include <stdint.h>

#include "timers.h"
#include "serial.h"

#define BAUD_RATE_115200 (115200)
#define UART_MODULE_INPUT_CLK (48000000)

#define RESET_SOURCE_REG 0x44E00F00
#define RESET_SOURCE_OFFSET 0x8 

#define EXTERNAL_RESET (5)
#define WATCHDOG_RESET (4)
#define SOFTWARE_RESET (1)
#define COLD_RESET (0)

static void UartInitialize(void);
static void UartBaudRateSet(void);
static void startup(void);

int main()
{
	UartInitialize();

	Timers_timerInit();
	Timers_watchdogInit();
	Serial_init();

	startup();

	Serial_printCommandList();

	while(1) {
		Serial_processInput();

		if(Timers_isIsrFlagSet()) {
			Timers_clearIsrFlag();
			Timers_hitWatchdog();
		}
	}

	return 0;
}

static void startup()
{
	ConsoleUtilsPrintf("Welcome to Light Bouncer\n");
	ConsoleUtilsPrintf("By Matheson Mawhinney and Alan Lee\n");
	ConsoleUtilsPrintf("==================================\n");

	uint32_t reset = HWREG(RESET_SOURCE_REG + RESET_SOURCE_OFFSET);

	ConsoleUtilsPrintf("Reset source (0x%x) = ", reset);
	if((reset & (1 << EXTERNAL_RESET)) != 0) {
		ConsoleUtilsPrintf("External reset, ");
	}
	if((reset & (1 << WATCHDOG_RESET)) != 0) {
		ConsoleUtilsPrintf("Watchdog reset, ");
	}
	if((reset & (1 << SOFTWARE_RESET)) != 0) {
		ConsoleUtilsPrintf("Software reset, ");
	} 
	if((reset & (1 << COLD_RESET)) != 0) {
		ConsoleUtilsPrintf("Cold reset, ");
	}
	HWREG(RESET_SOURCE_REG + RESET_SOURCE_OFFSET) |= reset;
	ConsoleUtilsPrintf("\n");
}

static void UartInitialize()
{
	/* Configuring the system clocks for UART0 instance. */
	UART0ModuleClkConfig();
	/* Performing the Pin Multiplexing for UART0 instance. */
	UARTPinMuxSetup(0);
	/* Performing a module reset. */
	UARTModuleReset(SOC_UART_0_REGS);
	/* Performing Baud Rate settings. */
	UartBaudRateSet();
	/* Switching to Configuration Mode B. */
	UARTRegConfigModeEnable(SOC_UART_0_REGS, UART_REG_CONFIG_MODE_B);
	/* Programming the Line Characteristics. */
	UARTLineCharacConfig(SOC_UART_0_REGS,
			(UART_FRAME_WORD_LENGTH_8 | UART_FRAME_NUM_STB_1),
			UART_PARITY_NONE);
	/* Disabling write access to Divisor Latches. */
	UARTDivisorLatchDisable(SOC_UART_0_REGS);
	/* Disabling Break Control. */
	UARTBreakCtl(SOC_UART_0_REGS, UART_BREAK_COND_DISABLE);
	/* Switching to UART16x operating mode. */
	UARTOperatingModeSelect(SOC_UART_0_REGS, UART16x_OPER_MODE);
	/* Select the console type based on compile time check */
	ConsoleUtilsSetType(CONSOLE_UART);
}


/*
 ** A wrapper function performing Baud Rate settings.
 */
static void UartBaudRateSet(void)
{
	unsigned int divisorValue = 0;

	/* Computing the Divisor Value. */
	divisorValue = UARTDivisorValCompute(UART_MODULE_INPUT_CLK,
			BAUD_RATE_115200,
			UART16x_OPER_MODE,
			UART_MIR_OVERSAMPLING_RATE_42);

	/* Programming the Divisor Latches. */
	UARTDivisorLatchWrite(SOC_UART_0_REGS, divisorValue);
}