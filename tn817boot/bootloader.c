/**
 * \file
 *
 *
 (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms,you may use this software and
    any derivatives exclusively with Microchip products.It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIPs TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "bootloader.h"
#include "command.h"
#include "device_specific.h"
#include <usart_basic.h>
#include <nvmctrl_basic.h>
#include <avr/eeprom.h>
#include "eeprom_map.h"
#include <util/delay_basic.h>
#include <avr/wdt.h> 

void BlockLoad(uint8_t buffer[], uint8_t mem, uint16_t size, native_pointer_t *address);
void BlockRead(unsigned int size, unsigned char mem, native_pointer_t *address);
void purge(uint8_t n);
void check_crc_eop();


#define BOOTLOADER_VERSION 0x1000



uint8_t EEMEM user_byte;

uint8_t buffer[32];

void bootloader(void)
{
	native_pointer_t address;
	uint16_t         length;
	uint8_t          memtype;
	unsigned char    val;
	unsigned char    parameter;
	uint8_t			 node_addr;
	uint8_t			 skip_loader;

	node_addr=FLASH_0_read_eeprom_byte((eeprom_adr_t)&user_byte+EEAD_CRIS)&0x0F; //0-16, default 16
	skip_loader=FLASH_0_read_eeprom_byte((eeprom_adr_t)&user_byte+EEAD_SKLD); //If 0xFF, run bootloader without hwc.

	//	device_init();
	#define LED_R	0x10
	#define LED_G	0x20
	PORTA.DIR=LED_R|LED_G;
	PORTA.OUTCLR=LED_R|LED_G;
	
	// Switch to Autolin mode. Await start.
	USART0.CTRLB=0xC6;
		
	for (uint8_t x=(node_addr<<1); x>0; x--) // 2 loops for each address
	{
		for (uint8_t cs=0; cs<25; cs++) // Delay 0.5s
		{
			_delay_loop_2((10*F_CPU/(1000*4)));
		}
		if (skip_loader==0xFF)
		{
			PORTA.OUTTGL=LED_G;			
		}
		else
		{
			PORTA.OUTTGL=LED_R;
		}
	}
	for (uint8_t cs=0; cs<50; cs++) // Delay 0.5s
	{
		_delay_loop_2((10*F_CPU/(1000*4)));
	}

	/* Branch to bootloader or application code? */
	/* If we got BREAK+SYNC+0x3D+parity and SKIP_LOADER is unprogrammed */
	/* OR HWR pin is LOW */
	if (((USART0.RXDATAH==0x80)&&(USART0.RXDATAL==0x7D)&&(skip_loader==0xFF))||((PORTC.IN&HWR_PIN)==0))
	{
		PORTA.OUTSET=LED_R|LED_G;
		wdt_reset();   
		_PROTECTED_WRITE(WDT_CTRLA, WDT_PERIOD_8KCLK_gc);
		
		// Switch back to uart mode. 
		USART0.CTRLB=0xC0;
		
		/* Main loop */
		for (;;) {
			val = USART_0_read(); // Wait for command character.
			switch (val) {
			case Cmnd_STK_GET_PARAMETER:
				parameter = USART_0_read();
				check_crc_eop();

				if (parameter == Parm_STK_SW_MINOR) {
					USART_0_write(BOOTLOADER_VERSION & 0xFF);
				} else if (parameter == Parm_STK_SW_MAJOR) {
					USART_0_write(BOOTLOADER_VERSION >> 8);
				} else {
					USART_0_write(0); // Dummy reply
				}
				break;

			case Cmnd_STK_SET_DEVICE:
				purge(20);
				check_crc_eop();
				break;

			case Cmnd_STK_SET_DEVICE_EXT:
				purge(5);
				check_crc_eop();
				break;

			case Cmnd_STK_LOAD_ADDRESS:
				address = USART_0_read() | (USART_0_read() << 8); // Read address low and high byte.
				check_crc_eop();
				wdt_reset();
				break;

			case Cmnd_STK_UNIVERSAL:
				purge(4);
				check_crc_eop();
				USART_0_write(0); // Dummy reply
				break;

			case Cmnd_STK_PROG_PAGE:
				length  = (USART_0_read() << 8) | USART_0_read(); // Read length high and low byte.
				memtype = USART_0_read();
				wdt_reset();
				BlockLoad(buffer, memtype, length, &address);
				check_crc_eop();
				break;

			case Cmnd_STK_READ_PAGE:
				length  = (USART_0_read() << 8) | USART_0_read(); // Read length high and low byte.
				memtype = USART_0_read();
				check_crc_eop();
				wdt_reset();
				BlockRead(length, memtype, &address);
				break;

			case Cmnd_STK_READ_SIGN:
				check_crc_eop();
				USART_0_write(SIGNATURE_BYTE_1);
				USART_0_write(SIGNATURE_BYTE_2);
				USART_0_write(SIGNATURE_BYTE_3);
				wdt_reset();
				break;

			case Cmnd_STK_LEAVE_PROGMODE:
				check_crc_eop();
				USART_0_write(Resp_STK_OK);
				reset();
				break;

			default:
				check_crc_eop();
				break;

			} // End of switch-case

			// All commands returns an OK response
			USART_0_write(Resp_STK_OK);

		} // end for()

	} // end if()

	// No wish to enter bootloader.
	call_application();

	// Never return, this saves code space since stack
	// need not be preserved on function entry.
	while (1)
		;

} // end: bootloader()

void BlockLoad(uint8_t buffer[], uint8_t mem, uint16_t size, native_pointer_t *address)
{

	native_pointer_t tempaddress;
	uint8_t          i;

	// EEPROM memory type.
	if ((mem == 'E') && ENABLE_EEPROM_SUPPORT) {
		/* Fill buffer first, as EEPROM is too slow to copy with UART speed */
		for (tempaddress = 0; tempaddress < size; tempaddress++)
			buffer[tempaddress] = USART_0_read();

		/* Then program the EEPROM */
		FLASH_0_write_eeprom_block(*address, buffer, size);
		*address = *address + size;
		return;
	}

	// Flash memory type.
	else if (mem == 'F') {        // NOTE: For flash programming, 'address' is given in words.
		(*address) <<= 1;         // Convert address to bytes temporarily.
		tempaddress = (*address); // Store address in page.

		for (i = 0; i < size; i++) {
			/* Programmer always writes complete pages, so finalize parameter is set to false. */
			FLASH_0_write_flash_stream(tempaddress + i, USART_0_read(), false);
		}

		*address = *address + size;
		(*address) >>= 1; // Convert address back to Flash words again.
		return;
	}
}

void BlockRead(unsigned int size, unsigned char mem, native_pointer_t *address)
{
	// EEPROM memory type.
	if ((mem == 'E') && ENABLE_EEPROM_SUPPORT) {
		do {
			USART_0_write(FLASH_0_read_eeprom_byte(*address));
			(*address)++; // Select next EEPROM byte
			size--;       // Decrease number of bytes to read
		} while (size);   // Repeat until all block has been read
	}

	// Flash memory type.
	else if (mem == 'F') {
		(*address) <<= 1; // Convert address to bytes temporarily.

		do {
			USART_0_write(FLASH_0_read_flash_byte(*address));

			USART_0_write(FLASH_0_read_flash_byte((*address + 1)));
			(*address) += 2; // Select next word in memory.
			size -= 2;       // Subtract two bytes from number of bytes to read
		} while (size);      // Repeat until all block has been read

//		(*address) >>= 1; // Convert address back to Flash words again.
	}
}

void purge(uint8_t n)
{
	do {
		USART_0_read();
	} while (--n);
}

void check_crc_eop()
{
	if (USART_0_read() == Sync_CRC_EOP)
	{
		USART_0_write(Resp_STK_INSYNC);
		PORTA.OUTSET=0x10;
	}
	else
	{
		USART_0_write(Resp_STK_NOSYNC);
		PORTA.OUTCLR=0x20;
	}
}
