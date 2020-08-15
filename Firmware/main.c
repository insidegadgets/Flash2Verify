/*
 Flash2Verify
 PCB version: 1.0
 Firmware version: R1
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 13/08/2020
 Last Modified: 15/08/2020
 
 Set fuse bits: External 16MHz crystal, divide clock by 8 is off, disable JTAG
 avrdude -p atmega169 -c usbasp -U lfuse:w:0xef:m -U hfuse:w:0x99:m
 
 */

#define F_CPU 16000000 // 16 MHz

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "setup.c" // See defines, variables, constants, functions here

#define FIRMWARE_VERSION 1

int main(void) {
	setup();
	
	uint32_t address = 0;
	
	while(1) {
		receivedChar = USART_Receive(); 
		
		if (receivedChar == TSOP_POWER_UP) {
			// Set AM29F016B I/O
			DDRG |= (1<<AM29F016B_A8) | (1<<AM29F016B_A11);
			DDRF |= (1<<AM29F016B_A12) | (1<<AM29F016B_A15) | (1<<AM29F016B_A18) | (1<<AM29F016B_A20) | (1<<AM29F016B_D6) | (1<<AM29F016B_5V_2) | (1<<AM29F016B_RD);
			DDRE |= (1<<AM29F016B_A4) | (1<<AM29F016B_A7) | (1<<AM29F016B_A10) | (1<<AM29F016B_A17);
			DDRD |= (1<<AM29F016B_A13) | (1<<AM29F016B_A16) | (1<<AM29F016B_A19) | (1<<AM29F016B_5V_1);
			DDRC |= (1<<AM29F016B_A0) | (1<<AM29F016B_A1) | (1<<AM29F016B_A3) | (1<<AM29F016B_GND_1) | (1<<AM29F016B_D5) | (1<<AM29F016B_GND_2) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1);
			DDRB |= (1<<AM29F016B_A5) | (1<<AM29F016B_A6) | (1<<AM29F016B_A9) | (1<<AM29F016B_A14) | (1<<AM29F016B_CE) | (1<<AM29F016B_RESET);
			DDRA |= (1<<AM29F016B_A2) | (1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4) | (1<<AM29F016B_WE);
			
			// Apply AM29F016B VCC/Reset
			PORTD |= (1<<AM29F016B_5V_1);
			PORTF |= (1<<AM29F016B_5V_2);
			PORTB |= (1<<AM29F016B_RESET);
			
			// WR, RD, CS high
			AM29F016B_wePin_high;
			AM29F016B_rdPin_high;
			AM29F016B_cePin_high;
		}
		else if (receivedChar == TSOP_POWER_DOWN) {
			// Ground AM29F016B VCC/Reset
			PORTB &= ~(1<<AM29F016B_RESET);
			PORTD &= ~(1<<AM29F016B_5V_1);
			PORTF &= ~(1<<AM29F016B_5V_2);
			
			// WR, RD, CS low
			AM29F016B_wePin_low;
			AM29F016B_rdPin_low;
			AM29F016B_cePin_low;
			
			_delay_ms(50);
			
			// Unset AM29F016B I/O
			DDRG &= ((1<<AM29F016B_A8) | (1<<AM29F016B_A11));
			DDRF &= ((1<<AM29F016B_A12) | (1<<AM29F016B_A15) | (1<<AM29F016B_A18) | (1<<AM29F016B_A20) | (1<<AM29F016B_D6) | (1<<AM29F016B_5V_2) | (1<<AM29F016B_RD));
			DDRE &= ((1<<AM29F016B_A4) | (1<<AM29F016B_A7) | (1<<AM29F016B_A10) | (1<<AM29F016B_A17));
			DDRD &= ((1<<AM29F016B_A13) | (1<<AM29F016B_A16) | (1<<AM29F016B_A19) | (1<<AM29F016B_5V_1));
			DDRC &= ((1<<AM29F016B_A0) | (1<<AM29F016B_A1) | (1<<AM29F016B_A3) | (1<<AM29F016B_GND_1) | (1<<AM29F016B_D5) | (1<<AM29F016B_GND_2) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1));
			DDRB &= ((1<<AM29F016B_A5) | (1<<AM29F016B_A6) | (1<<AM29F016B_A9) | (1<<AM29F016B_A14) | (1<<AM29F016B_CE) | (1<<AM29F016B_RESET));
			DDRA &= ((1<<AM29F016B_A2) | (1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4) | (1<<AM29F016B_WE));
		}
		else if (receivedChar == SET_ADDRESS) {
			usart_read_bytes(3);
			address = (uint32_t) ((uint32_t) receivedBuffer[2] << 16) | ((uint32_t) receivedBuffer[1] << 8) | (uint32_t) receivedBuffer[0];
			AM29F016B_set_address(address);
		}
		else if (receivedChar == READ_64_BYTES) {
			PORTD |= (1<<ACTIVITY_LED);
			AM29F016B_set_data_inputs();
			
			for (uint8_t x = 0; x < 64; x++) {
				USART_Transmit(AM29F016B_read_address(address));
				address++;
			}
			
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		else if (receivedChar == WRITE_64_BYTES) {
			usart_read_bytes(64);
			PORTD |= (1<<ACTIVITY_LED);
			
			for (uint8_t x = 0; x < 64; x++) {
				if (receivedBuffer[x] != 0xFF) {
					AM29F016B_set_data_outputs();
					AM29F016B_flash_write_cycle_555();
					AM29F016B_write_address_with_data(address, receivedBuffer[x]);
					_delay_us(5); // Wait byte program time
					
					// Verify data
					AM29F016B_set_data_inputs();
					uint8_t dataVerify = AM29F016B_read_address_fast();
					while (receivedBuffer[x] != dataVerify) {
						dataVerify = AM29F016B_read_address_fast();
						_delay_us(1);
					}
				}
				address++;
			}
			
			USART_Transmit(SEND_ACK); // Send back ack
			PORTD &= ~(1<<ACTIVITY_LED);
		}
		
		else if (receivedChar == WRITE_DATA) {
			usart_read_bytes(4);
			
			uint32_t receivedAddress = (uint32_t) ((uint32_t) receivedBuffer[2] << 16) | ((uint32_t) receivedBuffer[1] << 8) | (uint32_t) receivedBuffer[0];
			
			AM29F016B_set_data_outputs();
			AM29F016B_write_address_with_data(receivedAddress, receivedBuffer[3]);
		}
		
		// Send back the firmware version number
		else if (receivedChar == READ_FIRMWARE_VERSION) {
			USART_Transmit(FIRMWARE_VERSION);
		}
	}
}

