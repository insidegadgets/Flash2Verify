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
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdio.h>

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

// COM Port settings (default)
#include "rs232/rs232.h"
extern int cport_nr;
extern int bdrate;

#define READ_FIRMWARE_VERSION 'V'
#define TSOP_POWER_UP 'U'
#define TSOP_POWER_DOWN 'D'
#define FLASH_ID_555 'i'
#define FLASH_ID_5555 'I'
#define READ_64_BYTES 'R'
#define WRITE_64_BYTES 'W'
#define WRITE_DATA 'X'
#define SET_ADDRESS 'A'

// Common vars
#define READ_BUFFER 0

extern uint8_t firmwareVersion;
extern uint8_t readBuffer[257];
extern uint8_t writeBuffer[257];
extern uint32_t currAddr;
extern uint32_t endAddr;

// Read the config.ini file for the COM port to use and baud rate
void read_config(void);

// Write the config.ini file for the COM port to use and baud rate
void write_config(void);

void delay_ms(uint16_t ms);

// Read one letter from stdin
char read_one_letter(void);

// Print progress
void print_progress_percent(uint32_t bytesRead, uint32_t hashNumber);

// Wait for a "1" acknowledgement from the ATmega
void com_wait_for_ack (void);

// Stop reading blocks of data
void com_read_stop(void);

// Continue reading the next block of data
void com_read_cont(void);

// Test opening the COM port,if can't be open, try autodetecting device on other COM ports
uint8_t com_test_port(void);


// Read 1 to 64 bytes from the COM port and write it to the global read buffer or to a file if specified. 
// When polling the com port it return less than the bytes we want, keep polling and wait until we have all bytes requested. 
// We expect no more than 64 bytes.
uint16_t com_read_bytes(FILE *file, int count);

// Read 1-128 bytes from the file (or buffer) and write it the COM port with the command given
void com_write_bytes_from_file(uint8_t command, FILE *file, int count);

// Send a single command byte
void set_mode (char command);

// Send a command with a hex number and a null terminator byte
void set_number (uint32_t number, uint8_t command);

// Read the cartridge mode
uint8_t read_cartridge_mode (void);

// Send 1 byte and read 1 byte
uint8_t request_value (uint8_t command);

void set_address(uint32_t address);

void write_data(uint32_t address, uint8_t data);

void wait_for_flash_chip_erase_ff(uint8_t printProgress);
