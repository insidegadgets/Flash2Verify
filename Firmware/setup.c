/*
 Flash2Verify
 PCB version: 1.0
 Firmware version: R1
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 13/08/2020
 Last Modified: 15/08/2020
 
 */
 
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

#define ACTIVITY_LED PD4

#define READ_FIRMWARE_VERSION 'V'
#define TSOP_POWER_UP 'U'
#define TSOP_POWER_DOWN 'D'
#define FLASH_ID_555 'i'
#define FLASH_ID_5555 'I'
#define READ_64_BYTES 'R'
#define WRITE_64_BYTES 'W'
#define WRITE_DATA 'X'
#define SET_ADDRESS 'A'
#define SEND_ACK '1'

#define AM29F016B_A0 PC3 // 30
#define AM29F016B_A1 PC7 // 29
#define AM29F016B_A2 PA2 // 28
#define AM29F016B_A3 PC4 // 27
#define AM29F016B_A4 PE5 // 22
#define AM29F016B_A5 PB7 // 21
#define AM29F016B_A6 PB3 // 20
#define AM29F016B_A7 PE4 // 19
#define AM29F016B_A8 PG3 // 18
#define AM29F016B_A9 PB4 // 17
#define AM29F016B_A10 PE3 // 16
#define AM29F016B_A11 PG4 // 15
#define AM29F016B_A12 PF0 // 10
#define AM29F016B_A13 PD1 // 9
#define AM29F016B_A14 PB0 // 8
#define AM29F016B_A15 PF1 // 7
#define AM29F016B_A16 PD2 // 6
#define AM29F016B_A17 PE7 // 5
#define AM29F016B_A18 PF2 // 4
#define AM29F016B_A19 PD3 // 3
#define AM29F016B_A20 PF4 // 46

#define AM29F016B_D0 PA1 // 31
#define AM29F016B_D1 PC6 // 32
#define AM29F016B_D2 PC2 // 33
#define AM29F016B_D3 PA0 // 34
#define AM29F016B_D4 PA7 // 38
#define AM29F016B_D5 PC0 // 39
#define AM29F016B_D6 PF6 // 40
#define AM29F016B_D7 PA6 // 41

#define AM29F016B_RD PF5 // 43
#define AM29F016B_WE PA5 // 44
#define AM29F016B_CE PB1 // 11

#define AM29F016B_RESET PB5 // 14
#define AM29F016B_5V_1 PD0 // 12
#define AM29F016B_5V_2 PF7 // 37
#define AM29F016B_GND_1 PC1 // 36
#define AM29F016B_GND_2 PC5 // 35

#define AM29F016B_wePin_high	PORTA |= (1<<AM29F016B_WE);
#define AM29F016B_wePin_low	PORTA &= ~(1<<AM29F016B_WE);
#define AM29F016B_rdPin_high	PORTF |= (1<<AM29F016B_RD);
#define AM29F016B_rdPin_low	PORTF &= ~(1<<AM29F016B_RD);
#define AM29F016B_cePin_high	PORTB |= (1<<AM29F016B_CE);
#define AM29F016B_cePin_low	PORTB &= ~(1<<AM29F016B_CE);

char receivedBuffer[256];
char receivedChar;

// Receive USART data
uint8_t USART_Receive(void) {
	while ( !(UCSRA & (1<<RXC)) ); // Wait for data to be received
	return UDR; // Get and return received data from buffer
}

// Transmit USART data
void USART_Transmit(unsigned char data) {
	while ( !( UCSRA & (1<<UDRE)) ); // Wait for empty transmit buffer
	UDR = data;
}

// Read 1-256 bytes from the USART 
void usart_read_bytes(int count) {
	for (int x = 0; x < count; x++) {
		receivedBuffer[x] = USART_Receive();
	}
}

// Read the USART until a 0 (string terminator byte) is received
void usart_read_chars(void) {
	int x = 0;
	while (1) {
		receivedBuffer[x] = USART_Receive();
		if (receivedBuffer[x] == 0) {
			break;
		}
		x++;
		if (x >= 10) {
			break;
		}
	}
}

void AM29F016B_set_address (uint32_t setAddress) {
	// All off
	PORTG &= ~((1<<AM29F016B_A8) | (1<<AM29F016B_A11));
	PORTF &= ~((1<<AM29F016B_A12) | (1<<AM29F016B_A15) | (1<<AM29F016B_A18) | (1<<AM29F016B_A20));
	PORTE &= ~((1<<AM29F016B_A4) | (1<<AM29F016B_A7) | (1<<AM29F016B_A10) | (1<<AM29F016B_A17));
	PORTD &= ~((1<<AM29F016B_A13) | (1<<AM29F016B_A16) | (1<<AM29F016B_A19));
	PORTC &= ~((1<<AM29F016B_A0) | (1<<AM29F016B_A1) | (1<<AM29F016B_A3));
	PORTB &= ~((1<<AM29F016B_A5) | (1<<AM29F016B_A6) | (1<<AM29F016B_A9) | (1<<AM29F016B_A14));
	PORTA &= ~((1<<AM29F016B_A2));
	
	// Set
	if (setAddress & 0x01) {
		PORTC |= (1<<AM29F016B_A0);
	}
	if (setAddress & 0x02) {
		PORTC |= (1<<AM29F016B_A1);
	}
	if (setAddress & 0x04) {
		PORTA |= (1<<AM29F016B_A2);
	}
	if (setAddress & 0x08) {
		PORTC |= (1<<AM29F016B_A3);
	}
	if (setAddress & 0x010) {
		PORTE |= (1<<AM29F016B_A4);
	}
	if (setAddress & 0x20) {
		PORTB |= (1<<AM29F016B_A5);
	}
	if (setAddress & 0x40) {
		PORTB |= (1<<AM29F016B_A6);
	}
	if (setAddress & 0x80) {
		PORTE |= (1<<AM29F016B_A7);
	}
	if (setAddress & 0x100) {
		PORTG |= (1<<AM29F016B_A8);
	}
	if (setAddress & 0x200) {
		PORTB |= (1<<AM29F016B_A9);
	}
	if (setAddress & 0x400) {
		PORTE |= (1<<AM29F016B_A10);
	}
	if (setAddress & 0x800) {
		PORTG |= (1<<AM29F016B_A11);
	}
	if (setAddress & 0x1000) {
		PORTF |= (1<<AM29F016B_A12);
	}
	if (setAddress & 0x2000) {
		PORTD |= (1<<AM29F016B_A13);
	}
	if (setAddress & 0x4000) {
		PORTB |= (1<<AM29F016B_A14);
	}
	if (setAddress & 0x8000) {
		PORTF |= (1<<AM29F016B_A15);
	}
	if (setAddress & 0x10000) {
		PORTD |= (1<<AM29F016B_A16);
	}
	if (setAddress & 0x20000) {
		PORTE |= (1<<AM29F016B_A17);
	}
	if (setAddress & 0x40000) {
		PORTF |= (1<<AM29F016B_A18);
	}
	if (setAddress & 0x80000) {
		PORTD |= (1<<AM29F016B_A19);
	}
	if (setAddress & 0x100000) {
		PORTF |= (1<<AM29F016B_A20);
	}
}

void AM29F016B_set_data (uint8_t data) {
	// All off
	PORTF &= ~((1<<AM29F016B_D6));
	PORTC &= ~((1<<AM29F016B_D5) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1));
	PORTA &= ~((1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4));
	
	// Set
	if (data & (1<<0)) {
		PORTA |= (1<<AM29F016B_D0);
	}
	if (data & (1<<1)) {
		PORTC |= (1<<AM29F016B_D1);
	}
	if (data & (1<<2)) {
		PORTC |= (1<<AM29F016B_D2);
	}
	if (data & (1<<3)) {
		PORTA |= (1<<AM29F016B_D3);
	}
	if (data & (1<<4)) {
		PORTA |= (1<<AM29F016B_D4);
	}
	if (data & (1<<5)) {
		PORTC |= (1<<AM29F016B_D5);
	}
	if (data & (1<<6)) {
		PORTF |= (1<<AM29F016B_D6);
	}
	if (data & (1<<7)) {
		PORTA |= (1<<AM29F016B_D7);
	}
}

void AM29F016B_set_data_outputs(void) {
	DDRF |= (1<<AM29F016B_D6);
	DDRC |= (1<<AM29F016B_D5) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1);
	DDRA |= (1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4);
}

void AM29F016B_set_data_inputs(void) {
	PORTF &= ~((1<<AM29F016B_D6));
	PORTC &= ~((1<<AM29F016B_D5) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1));
	PORTA &= ~((1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4));
	DDRF &= ~((1<<AM29F016B_D6));
	DDRC &= ~((1<<AM29F016B_D5) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1));
	DDRA &= ~((1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4));
}

uint8_t AM29F016B_read_data(void) {
	uint8_t dataRead = 0;
	
	if (PINA & (1<<AM29F016B_D7)) {
		dataRead |= (1<<7);
	}
	if (PINF & (1<<AM29F016B_D6)) {
		dataRead |= (1<<6);
	}
	if (PINC & (1<<AM29F016B_D5)) {
		dataRead |= (1<<5);
	}
	if (PINA & (1<<AM29F016B_D4)) {
		dataRead |= (1<<4);
	}
	if (PINA & (1<<AM29F016B_D3)) {
		dataRead |= (1<<3);
	}
	if (PINC & (1<<AM29F016B_D2)) {
		dataRead |= (1<<2);
	}
	if (PINC & (1<<AM29F016B_D1)) {
		dataRead |= (1<<1);
	}
	if (PINA & (1<<AM29F016B_D0)) {
		dataRead |= (1<<0);
	}
	
	return dataRead;
}

void AM29F016B_write_address_with_data(uint32_t writeAddress, uint8_t writeData) {
	AM29F016B_set_address(writeAddress);
	AM29F016B_set_data(writeData);
	AM29F016B_cePin_low;
	AM29F016B_wePin_low;
	asm volatile("nop");
	asm volatile("nop");
	AM29F016B_cePin_high;
	AM29F016B_wePin_high;
}

uint8_t AM29F016B_read_address(uint32_t readAddress) {
	uint8_t data = 0;
	
	AM29F016B_set_address(readAddress);
	AM29F016B_cePin_low;
	AM29F016B_rdPin_low;
	asm volatile("nop");
	asm volatile("nop");
	data = AM29F016B_read_data();
	AM29F016B_cePin_high;
	AM29F016B_rdPin_high;
	
	return data;
}

uint8_t AM29F016B_read_address_fast(void) {
	uint8_t data = 0;
	
	AM29F016B_cePin_low;
	AM29F016B_rdPin_low;
	asm volatile("nop");
	asm volatile("nop");
	data = AM29F016B_read_data();
	AM29F016B_cePin_high;
	AM29F016B_rdPin_high;
	
	return data;
}

void AM29F016B_flash_write_cycle_555(void) {
	// 0x555
	PORTG &= ~((1<<AM29F016B_A8) | (1<<AM29F016B_A11));
	PORTF &= ~((1<<AM29F016B_A12) | (1<<AM29F016B_A15) | (1<<AM29F016B_A18) | (1<<AM29F016B_A20));
	PORTE &= ~((1<<AM29F016B_A4) | (1<<AM29F016B_A7) | (1<<AM29F016B_A10) | (1<<AM29F016B_A17));
	PORTD &= ~((1<<AM29F016B_A13) | (1<<AM29F016B_A16) | (1<<AM29F016B_A19));
	PORTC &= ~((1<<AM29F016B_A0) | (1<<AM29F016B_A1) | (1<<AM29F016B_A3));
	PORTB &= ~((1<<AM29F016B_A5) | (1<<AM29F016B_A6) | (1<<AM29F016B_A9) | (1<<AM29F016B_A14));
	PORTA &= ~((1<<AM29F016B_A2));
	PORTC |= (1<<AM29F016B_A0);
	PORTA |= (1<<AM29F016B_A2);
	PORTE |= (1<<AM29F016B_A4);
	PORTB |= (1<<AM29F016B_A6);
	PORTG |= (1<<AM29F016B_A8);
	PORTE |= (1<<AM29F016B_A10);
	
	// 0xAA
	PORTF &= ~((1<<AM29F016B_D6));
	PORTC &= ~((1<<AM29F016B_D5) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1));
	PORTA &= ~((1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4));
	PORTC |= (1<<AM29F016B_D1);
	PORTA |= (1<<AM29F016B_D3);
	PORTC |= (1<<AM29F016B_D5);
	PORTA |= (1<<AM29F016B_D7);
	
	AM29F016B_cePin_low;
	AM29F016B_wePin_low;
	asm volatile("nop");
	asm volatile("nop");
	AM29F016B_cePin_high;
	AM29F016B_wePin_high;
	
	
	// 0x2AA
	PORTG &= ~((1<<AM29F016B_A8) | (1<<AM29F016B_A11));
	PORTF &= ~((1<<AM29F016B_A12) | (1<<AM29F016B_A15) | (1<<AM29F016B_A18) | (1<<AM29F016B_A20));
	PORTE &= ~((1<<AM29F016B_A4) | (1<<AM29F016B_A7) | (1<<AM29F016B_A10) | (1<<AM29F016B_A17));
	PORTD &= ~((1<<AM29F016B_A13) | (1<<AM29F016B_A16) | (1<<AM29F016B_A19));
	PORTC &= ~((1<<AM29F016B_A0) | (1<<AM29F016B_A1) | (1<<AM29F016B_A3));
	PORTB &= ~((1<<AM29F016B_A5) | (1<<AM29F016B_A6) | (1<<AM29F016B_A9) | (1<<AM29F016B_A14));
	PORTA &= ~((1<<AM29F016B_A2));
	PORTE |= (1<<AM29F016B_A7);
	PORTC |= (1<<AM29F016B_A1) | (1<<AM29F016B_A3);
	PORTB |= (1<<AM29F016B_A5) | (1<<AM29F016B_A9);
	
	// 0x55
	PORTF &= ~((1<<AM29F016B_D6));
	PORTC &= ~((1<<AM29F016B_D5) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1));
	PORTA &= ~((1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4));
	PORTF |= (1<<AM29F016B_D6);
	PORTC |= (1<<AM29F016B_D2);
	PORTA |= (1<<AM29F016B_D0) | (1<<AM29F016B_D4);
	
	
	AM29F016B_cePin_low;
	AM29F016B_wePin_low;
	asm volatile("nop");
	asm volatile("nop");
	AM29F016B_cePin_high;
	AM29F016B_wePin_high;
	
	
	// 0x555
	PORTG &= ~((1<<AM29F016B_A8) | (1<<AM29F016B_A11));
	PORTF &= ~((1<<AM29F016B_A12) | (1<<AM29F016B_A15) | (1<<AM29F016B_A18) | (1<<AM29F016B_A20));
	PORTE &= ~((1<<AM29F016B_A4) | (1<<AM29F016B_A7) | (1<<AM29F016B_A10) | (1<<AM29F016B_A17));
	PORTD &= ~((1<<AM29F016B_A13) | (1<<AM29F016B_A16) | (1<<AM29F016B_A19));
	PORTC &= ~((1<<AM29F016B_A0) | (1<<AM29F016B_A1) | (1<<AM29F016B_A3));
	PORTB &= ~((1<<AM29F016B_A5) | (1<<AM29F016B_A6) | (1<<AM29F016B_A9) | (1<<AM29F016B_A14));
	PORTA &= ~((1<<AM29F016B_A2));
	PORTC |= (1<<AM29F016B_A0);
	PORTA |= (1<<AM29F016B_A2);
	PORTE |= (1<<AM29F016B_A4);
	PORTB |= (1<<AM29F016B_A6);
	PORTG |= (1<<AM29F016B_A8);
	PORTE |= (1<<AM29F016B_A10);
	
	// 0xA0
	PORTF &= ~((1<<AM29F016B_D6));
	PORTC &= ~((1<<AM29F016B_D5) | (1<<AM29F016B_D2) | (1<<AM29F016B_D1));
	PORTA &= ~((1<<AM29F016B_D0) | (1<<AM29F016B_D3) | (1<<AM29F016B_D7) | (1<<AM29F016B_D4));
	PORTC |= (1<<AM29F016B_D5);
	PORTA |= (1<<AM29F016B_D7);
	
	AM29F016B_cePin_low;
	AM29F016B_wePin_low;
	asm volatile("nop");
	asm volatile("nop");
	AM29F016B_cePin_high;
	AM29F016B_wePin_high;
	
}


// Setup
void setup(void) {
	// Light LED
	DDRD |= (1<<ACTIVITY_LED);
	PORTD |= (1<<ACTIVITY_LED);
	_delay_ms(500);
	PORTD &= ~(1<<ACTIVITY_LED);
	_delay_ms(500);
	
	// Setup USART
	UBRRL = 0; // 1Mbps Baud rate
	sbi(UCSRB, RXEN); // Receiver enable
	sbi(UCSRB, TXEN); // Transmitter enable
	
	// Turn on interrupts
	sei();
}
