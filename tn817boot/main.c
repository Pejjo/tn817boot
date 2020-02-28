#include <compiler.h>
#include <clock_config.h>

#include <usart_basic.h>

#include <nvmctrl_basic.h>
#include <ccp.h>
#include <string.h>

#include <slpctrl.h>
#include <bod.h>

#include "bootloader.h"

int main(void) __attribute__((OS_main));

int main(void)
{
	// System init
	/* Initializes MCU, drivers and middleware */
	// Enable pullup on all ports.
	for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTA + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}

	for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTB + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}

	for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTC + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}

	// Set pin direction to input
	PORTC.DIRCLR=HWR_PIN;

	ccp_write_io((void *)&(CLKCTRL.MCLKCTRLB), CLKCTRL_PDIV_6X_gc | 1 << CLKCTRL_PEN_bp); /* Prescaler enable, div 8 */

	//USART_0_initialization
	// Set pin direction to input
	PORTB.DIRCLR=PIN3_bm;

	// Set pin direction to output for enable and TX
	PORTB.OUTSET=PIN1_bm|PIN2_bm;
	PORTB.DIRSET=PIN1_bm|PIN2_bm;

	USART_0_init();
	
	// End USART init

	FLASH_0_init();

	SLPCTRL_init();

	BOD_init();

	// End system init

	bootloader();

	return 1;
}
