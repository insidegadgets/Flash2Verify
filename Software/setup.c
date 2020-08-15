/*
 Flash2Verify
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 13/08/2020
 Last Modified: 15/08/2020
 
 */

#ifdef _WIN32
#include <Windows.h>
#else
#define _XOPEN_SOURCE 600
#include <time.h>
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
#define RS232_PORTNR  57
#else
#define RS232_PORTNR  30
#endif

#include <stdio.h>
#include "setup.h"

// COM Port settings (default)
#include "rs232/rs232.h"
int cport_nr = 7; // /dev/ttyS7 (COM8 on windows)
int bdrate = 1000000; // 1,000,000 baud

// Common vars
uint8_t firmwareVersion = 0;
uint8_t readBuffer[257];
uint8_t writeBuffer[257];
uint32_t currAddr = 0x0000;
uint32_t endAddr = 0x7FFF;

// Read the config.ini file for the COM port to use and baud rate
void read_config(void) {
	FILE* configfile = fopen ("config.ini" , "rt");
	if (configfile != NULL) {
		if (fscanf(configfile, "%d\n%d", &cport_nr, &bdrate) != 2) {
			fprintf(stderr, "Config file is corrupt\n");
		}
		else {
			cport_nr--;
		}
		fclose(configfile);
	}
	else {
		fprintf(stderr, "Config file not found\n");
	}
}

// Write the config.ini file for the COM port to use and baud rate
void write_config(void) {
	FILE *configfile = fopen("config.ini", "wt");
	if (configfile != NULL) {
		fprintf(configfile, "%d\n%d\n", cport_nr+1, bdrate);
		fclose(configfile);
	}
}

void delay_ms(uint16_t ms) {
	#if defined (_WIN32)
		Sleep(ms);
	#else
		struct timespec ts;
		ts.tv_sec = ms / 1000;
		ts.tv_nsec = (ms * 1000000) % 1000000000;
		nanosleep(&ts, NULL);
	#endif
}

// Read one letter from stdin
char read_one_letter (void) {
	char c = getchar();
	while (getchar() != '\n' && getchar() != EOF);
	return c;
}

// Print progress
void print_progress_percent (uint32_t bytesRead, uint32_t hashNumber) {
	if ((bytesRead % hashNumber == 0) && bytesRead != 0) {
		if (hashNumber == 64) {
			printf("########");
			fflush(stdout);
		}
		else {
			printf("#");
			fflush(stdout);
		}
	}
}

// Wait for a "1" acknowledgement from the ATmega
void com_wait_for_ack (void) {
	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	
	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);
		
		if (rxBytes > 0) {
			if (buffer[0] == '1') {
				break;
			}
			rxBytes = 0;
		}
	}
}

// Stop reading blocks of data
void com_read_stop() {
	RS232_cputs(cport_nr, "0"); // Stop read
	RS232_drain(cport_nr);
}

// Continue reading the next block of data
void com_read_cont() {
	RS232_cputs(cport_nr, "1"); // Continue read
	RS232_drain(cport_nr);
}

// Test opening the COM port,if can't be open, try autodetecting device on other COM ports
uint8_t com_test_port(void) {
	// Check if COM port responds correctly
	if (RS232_OpenComport(cport_nr, bdrate, "8N1") == 0) { // Port opened
		set_mode('0');
		uint8_t cartridgeMode = request_value(READ_FIRMWARE_VERSION);
		
		// Responded ok
		if (cartridgeMode == 1) {
			return 1;
		}
	}
	
	// If port didn't get opened or responded wrong
	for (uint8_t x = 0; x <= RS232_PORTNR; x++) {
		if (RS232_OpenComport(x, bdrate, "8N1") == 0) { // Port opened
			cport_nr = x;
			
			// See if device responds correctly
			set_mode('0');
			uint8_t cartridgeMode = request_value(READ_FIRMWARE_VERSION);
			
			// Responded ok, save the new port number
			if (cartridgeMode == 1) {
				write_config();
				return 1;
			}
			else {
				RS232_CloseComport(x);
			}
		}
	}
	
	return 0;
}

// Read 1 to 256 bytes from the COM port and write it to the global read buffer or to a file if specified. 
// When polling the com port it return less than the bytes we want, keep polling and wait until we have all bytes requested. 
// We expect no more than 256 bytes.
uint16_t com_read_bytes (FILE *file, int count) {
	uint8_t buffer[257];
	uint16_t rxBytes = 0;
	uint16_t readBytes = 0;
	
	#if defined(__APPLE__)
	uint8_t timeout = 0;
	#else
	uint16_t timeout = 0;
	#endif
	
	while (readBytes < count) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 64);
		
		if (rxBytes > 0) {
			buffer[rxBytes] = 0;
			
			if (file == NULL) {
				memcpy(&readBuffer[readBytes], buffer, rxBytes);
			}
			else {
				fwrite(buffer, 1, rxBytes, file);
			}
			
			readBytes += rxBytes;
		}
		#if defined(__APPLE__)
		else {
			delay_ms(5);
			timeout++;
			if (timeout >= 50) {
				return readBytes;
			}
		}
		#else
		else {
			timeout++;
			if (timeout >= 20000) {
				return readBytes;
			}
		}
		#endif
	}
	return readBytes;
}

// Read 1-256 bytes from the file (or buffer) and write it the COM port with the command given
void com_write_bytes_from_file(uint8_t command, FILE *file, int count) {
	uint8_t buffer[257];
	buffer[0] = command;

	if (file == NULL) {
		memcpy(&buffer[1], writeBuffer, count);
	}
	else {
		fread(&buffer[1], 1, count, file);
	}
	
	RS232_SendBuf(cport_nr, buffer, (count + 1)); // command + 1-128 bytes
	RS232_drain(cport_nr);
}

// Send a single command byte
void set_mode (char command) {
	char modeString[5];
	sprintf(modeString, "%c", command);
	
	RS232_cputs(cport_nr, modeString);
	RS232_drain(cport_nr);
	
	delay_ms(1);
	
	#if defined(__APPLE__)
	delay_ms(5);
	#endif
}

// Send a command with a hex number and a null terminator byte
void set_number (uint32_t number, uint8_t command) {
	char numberString[20];
	sprintf(numberString, "%c%x", command, number);
	
	RS232_cputs(cport_nr, numberString);
	RS232_SendByte(cport_nr, 0);
	RS232_drain(cport_nr);
	delay_ms(1);
	
	//printf("%s\n", numberString);
	
	#if defined(__APPLE__) || defined(__linux__)
	delay_ms(5);
	#endif
}

// Send 1 byte and read 1 byte
uint8_t request_value (uint8_t command) {
	set_mode(command);
	
	uint8_t buffer[2];
	uint8_t rxBytes = 0;
	uint8_t timeoutCounter = 0;
	
	while (rxBytes < 1) {
		rxBytes = RS232_PollComport(cport_nr, buffer, 1);
		
		if (rxBytes > 0) {
			return buffer[0];
		}
		
		delay_ms(10);
		timeoutCounter++;
		if (timeoutCounter >= 25) { // After 250ms, timeout
			return 0;
		}
	}
	
	return 0;
}

void set_address(uint32_t address) {
	uint8_t dataBuffer[4] = {SET_ADDRESS, 0, 0, 0};
	dataBuffer[3] = address >> 16;
	dataBuffer[2] = address >> 8;
	dataBuffer[1] = address & 0xFF;
	RS232_SendBuf(cport_nr, dataBuffer, 4);
	RS232_drain(cport_nr);
	delay_ms(5);
}

void write_data(uint32_t address, uint8_t data) {
	uint8_t dataBuffer[5] = {WRITE_DATA, 0, 0, 0, 0};
	dataBuffer[3] = address >> 16;
	dataBuffer[2] = address >> 8;
	dataBuffer[1] = address & 0xFF;
	dataBuffer[4] = data;
	RS232_SendBuf(cport_nr, dataBuffer, 5);
	RS232_drain(cport_nr);
	delay_ms(5);
}


// Wait for first byte of Flash to be 0xFF, that's when we know the sector has been erased
void wait_for_flash_chip_erase_ff(uint8_t printProgress) {
	uint16_t timeout = 0;
	readBuffer[0] = 0;
	
	while (readBuffer[0] != 0xFF) {
		set_address(0x00);
		set_mode(READ_64_BYTES);
		delay_ms(5);
		
		uint8_t comReadBytes = com_read_bytes(READ_BUFFER, 64);
		com_read_stop(); // End read
		
		if (comReadBytes != 64) {
			fflush(stdin);
			delay_ms(500);
			
			// Flush buffer
			RS232_PollComport(cport_nr, readBuffer, 64);			
		}
		
		if (printProgress == 1) {
			//printf(".");
			printf("0x%X ", readBuffer[0]);
			fflush(stdout);
		}
		
		if (readBuffer[0] != 0xFF) {
			delay_ms(500);
			
			timeout++;
			if (timeout >= 600) {
				printf("\n\n Waiting for chip erase has timed out. Please unplug GBxCart RW, re-seat the cartridge and try again.\n");
				read_one_letter();
				exit(1);
			}
		}
	}
}
