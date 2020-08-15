/*
 Flash2Verify - AM29F016B
 Version: 1.0
 Author: Alex from insideGadgets (www.insidegadgets.com)
 Created: 13/08/2020
 Last Modified: 15/08/2020
 
 Flash 2 Verify allows you insert supported 5V flash chips to a TSOP package to write and read the contents back to 
 verify that they work correctly. Useful if you would like to know the flash chip works before soldering it on a PCB.
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "setup.h" // See defines, variables, constants, functions here

int main(int argc, char **argv) {
	
	printf("Flash2Verify v1.0 by insideGadgets\n");
	printf("##################################\n");
	
	read_config();
	
	// Open COM port
	if (com_test_port() == 0) {
		printf("Device not connected and couldn't be auto detected\n");
		read_one_letter();
		return 1;
	}
	printf("Connected on COM port: %i\n", cport_nr+1);
	
	// Get firmware version
	firmwareVersion = request_value(READ_FIRMWARE_VERSION);
	printf("Firmware: R%i\n\n", firmwareVersion);
	
	printf("AM29F016B Flash Chip\n");
	
	// Power up Flash chip
	set_mode(TSOP_POWER_UP);
	
	// Read ROM and check Flash ID
	printf("\nChecking Flash ID...\n");
	set_address(0x00);
	set_mode(READ_64_BYTES);
	delay_ms(5);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	printf("         Read ROM: 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
	
	write_data(0x555, 0xAA);
	write_data(0x2AA, 0x55);
	write_data(0x555, 0x90);
	delay_ms(5);
	set_mode(READ_64_BYTES);
	delay_ms(5);
	com_read_bytes(READ_BUFFER, 64);
	com_read_stop(); // End read
	write_data(0x555, 0xF0);
	printf(" Flash ID (0x555): 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3]);
	
	
	// Check Flash ID
	if (readBuffer[0] != 0x01 || readBuffer[1] != 0xAD) {
		printf("Flash ID doesn't match 0x01 0xAD. Please unplug and re-seat the flash chip.\n");
		read_one_letter();
		return 0;
	}
	
	
	// Erase flash
	printf("\nErasing Flash... ");
	write_data(0x555, 0xAA);
	write_data(0x2AA, 0x55);
	write_data(0x555, 0x80);
	write_data(0x555, 0xAA);
	write_data(0x2AA, 0x55);
	write_data(0x555, 0x10);
	wait_for_flash_chip_erase_ff(1);
	printf("Done\n");
	
	
	// Write random ROM file
	FILE *romFile = fopen("random.gb", "rb");
	if (romFile == NULL) {
		printf("\nrandom.gb file not found\n");
		read_one_letter();
		return 1;
	}
	
	printf("\nWriting random ROM file\n");
	printf("[             25%%             50%%             75%%            100%%]\n[");
	
	currAddr = 0x0000;
	endAddr = 2097152;
	set_address(0x00);
	while (currAddr < endAddr) {
		com_write_bytes_from_file(WRITE_64_BYTES, romFile, 64);
		com_wait_for_ack();
		currAddr += 64;
		
		// Print progress
		print_progress_percent(currAddr, endAddr / 64);
	}
	
	printf("]");
	fclose(romFile);
	
	
	// Backup ROM
	printf("\n\nReading ROM back\n");
	printf("[             25%%             50%%             75%%            100%%]\n[");
	
	FILE *romReadFile = fopen("read.gb", "wb");
	currAddr = 0x0000;
	endAddr = 2097152;
	set_address(0x00);
	while (currAddr < endAddr) {
		set_mode(READ_64_BYTES);
		com_read_bytes(romReadFile, 64);
		currAddr += 64;
		
		// Print progress
		print_progress_percent(currAddr, endAddr / 64);
	}
	
	printf("]");
	fclose(romReadFile);
	printf("\n\n");
	
	
	// Compare ROMs
	FILE *compareFile1 = fopen("read.gb", "rb");
	FILE *compareFile2 = fopen("random.gb", "rb");
	uint32_t fileSize = 2097152;
	uint32_t fileSizeCounter = 0;
	uint8_t compareOk = 1;
	
	while (fileSizeCounter < fileSize) {
		if (fgetc(compareFile1) != fgetc(compareFile2)) {
			compareOk = 0;
			break;
		}
		fileSizeCounter++;
	}
	
	if (compareOk == 1) {
		printf("File compare completed. Flash appears to be ok.\n");
	}
	else {
		printf("!!! File compare failed !!!\n");
	}
	
	// Power down Flash chip
	set_mode(TSOP_POWER_DOWN);
	
	read_one_letter();
	
	return 0;
}
