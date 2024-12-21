#ifndef EEPROM_H_
#define EEPROM_H_

#include <avr/eeprom.h>
#include <string.h>
#include "define.h"
// Max size for a student code
int first_USART = 0;
void reset_eeprom() {
	for (uint16_t i = 0; i < E2END; i++) {
		eeprom_write_byte((uint8_t*)i, 0xFF);  // Reset each byte to default value (0xFF)
	}
}


uint16_t find_next_empty_address() {
	uint16_t addr = EEPROM_START_ADDR;
	char value;
	for (; addr < EEPROM_SIZE; addr += STUDENT_CODE_SIZE) {
		eeprom_read_block(&value, (void *)addr, 1);
		if (value == 0xFF) { 
			break;
		}
	}
	return addr;
}

uint8_t check_student_code(const char *code) {
	uint16_t addr = EEPROM_START_ADDR;
	char existing_code[STUDENT_CODE_SIZE + 1];

	for (uint8_t i = 0; i < EEPROM_MAX_RECORDS; i++) {
		eeprom_read_block(existing_code, (void *)addr, STUDENT_CODE_SIZE);
		existing_code[STUDENT_CODE_SIZE] = '\0';

		if (strlen(existing_code) == 0) {
			break; // Reached empty space
		}

		if (strcmp(existing_code, code) == 0) {
			return STUDENT_EXIST; // Code found
		}

		addr += STUDENT_CODE_SIZE;
	}

	return STUDENT_NOT_FOUNDED; // Code not found
}

uint8_t save_student_code(const char *code) {
	// Check if the code already exists
	if (check_student_code(code) == STUDENT_EXIST) {
		return ERROR_DUPLICATED_STD_CODE; // Duplicate code
	}

	// Find the next available address
	uint16_t addr = find_next_empty_address();
	if (addr + STUDENT_CODE_SIZE >= EEPROM_SIZE) {
		return ERROR_EEPROM_FULL; // EEPROM full
	}

	// Save the code at the found address
	eeprom_write_block((const void *)code, (void *)addr, STUDENT_CODE_SIZE);
	return SUCCESS_SAVED_STD_CODE; // Success
}

uint8_t delete_student_code(char *code) {
	uint16_t addr = EEPROM_START_ADDR;
	char buffer[STUDENT_CODE_SIZE + 1] = {0};

	while (addr < EEPROM_START_ADDR + EEPROM_SIZE) {
		for (uint8_t i = 0; i < STUDENT_CODE_SIZE; i++) {
			buffer[i] = eeprom_read_byte((uint8_t *)(addr + i));
		}
		buffer[STUDENT_CODE_SIZE] = '\0';
		
		if (strcmp(buffer, code) == 0) {
			// Clear the block
			for (uint8_t i = 0; i < STUDENT_CODE_SIZE; i++) {
				eeprom_write_byte((uint8_t *)(addr + i), 0xFF);
			}
			return SUCCESS_STUDENT_DELETED; // Successfully deleted
		}
		addr += STUDENT_CODE_SIZE;
	}
	
	// Code not found
	lcd_command(0x01); // Clear screen
	lcd_print("Error: Not Found");
	return ERROR_DELETE_STUDENT; // Error: Code not found
}

void display_student_codes(){
	
	 uint16_t addr = EEPROM_START_ADDR; // Start of EEPROM
	 char code[STUDENT_CODE_SIZE + 1];                      // Buffer to read student codes

	 while (addr < EEPROM_START_ADDR + EEPROM_SIZE)
	 {
		 // Read the 8-byte block and check if it's valid
		 uint8_t is_valid = 0;
		 for (uint8_t i = 0; i < STUDENT_CODE_SIZE; i++) {
			 code[i] = eeprom_read_byte((uint8_t *)(addr + i));
			 if (code[i] != 0xFF) {
				 is_valid = 1; // At least one byte is not 0xFF
			 }
		 }
		 code[STUDENT_CODE_SIZE] = '\0'; // Null-terminate the string

		 if (is_valid) {
			 // Display valid student code
			lcd_command(0x01); // Clear screen
			lcd_print(" Attendees:");
			 lcd_command(0xC0); // Move to second line
			 lcd_print(code);
			 _delay_ms(SLEEP_TIME);
		 }
		 //else{
			 //return; // shloud be deleted if wanna add delete std
		 //}

		 addr += STUDENT_CODE_SIZE; // Move to the next block
	 }
	
}




/************************************************************************/
/*                       USART Protocol                                 */
/************************************************************************/


// USART protocol :)

void USART_init(unsigned int ubrr) {
	UBRRL = (unsigned char)ubrr;
	UBRRH = (unsigned char)(ubrr >> 8);
	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << UCSZ1) | (1 << UCSZ0); // Set UCSZ1 and UCSZ0 for 8-bit data
}

void USART_Transmit(unsigned char data)
{
	while(!(UCSRA &(1<<UDRE)));
	UDR = data;
}


void USART_Transmit_String(char *str) {
	while (*str) {
		USART_Transmit(*str);
		str++;
	}
}

void int_to_string(int num, char *str) {
	sprintf(str, "%d", num);
}

void put_line() {
	for( int i = 0 ; i < 25 ; i ++){
					USART_Transmit('-*'); 
	}
	USART_Transmit('\r'); 
	USART_Transmit('\n');
}

void send_all_student_info_usrt() {
	if (first_USART ==0)
	{
		USART_init(0x33);	// Initialize USART with a baud rate (e.g., for 9600 bps with F_CPU = 1 MHz)
		first_USART = 1;
	}
	
	
	
	uint16_t addr = EEPROM_START_ADDR; // Start address in EEPROM
	char code[STUDENT_CODE_SIZE + 1];  // Buffer to read student codes
	uint8_t number = 0;                // Count of attendees
	char sNumber[4];                   // Buffer to convert number to string

	while (addr < EEPROM_START_ADDR + EEPROM_SIZE) {
		// Read the 8-byte block and check if it's valid
		uint8_t is_valid = 0;
		for (uint8_t i = 0; i < STUDENT_CODE_SIZE; i++) {
			code[i] = eeprom_read_byte((uint8_t *)(addr + i));
			if (code[i] != 0xFF) {
				is_valid = 1; // At least one byte is not 0xFF
			}
		}
		code[STUDENT_CODE_SIZE] = '\0'; // Null-terminate the string

		if (is_valid) {
			number++; // Increment the attendee number

			// Transmit "Attendees "
			USART_Transmit_String("Attendees number : ");
			
			// Transmit <number>
			int_to_string(number, sNumber);
			USART_Transmit_String(sNumber);
			USART_Transmit(' '); // Add a space

			// Transmit <code>
			USART_Transmit_String(code);
			USART_Transmit('\r'); // Add a newline for readability
			USART_Transmit('\n');
		}

		addr += STUDENT_CODE_SIZE; // Move to the next block
	}

	USART_Transmit_String(" End of List\n"); // Indicate the end of the data
	USART_Transmit('\r'); // Add a newline for readability
	USART_Transmit('\n');
	put_line();
}

#endif
