#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "define.h"
#ifndef KEYPAD_H_
#define KEYPAD_H_


unsigned char keypad[4][3] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};

void keypad_init() {
	KEY_DDR = 0xF0; //makes usart port input again but it's ok
	KEY_PORT |= 0xFE;
	KEY_PORT &= 0x0E; //ground all rows at once
}

char keypad_scan() {
	unsigned char colloc, rowloc;
	
	
	do {
		do {
			
			_delay_ms(20); //call delay
			colloc = (KEY_PIN & 0x0E); //see if any key is pressed
		} while (colloc == 0x0E); //keep checking for key press
		_delay_ms(20); //call delay for debounce
		colloc = (KEY_PIN & 0x0E); //read columns
	} while (colloc == 0x0E); //wait for key press
	
	while (1) {
		KEY_PORT = 0xEE; //ground row 0
		colloc = (KEY_PIN & 0x0E); //read the columns

		if (colloc != 0x0E) //column detected
		{
			rowloc = 0; //save row location
			break; //exit while loop
		}
		KEY_PORT = 0xDE; //ground row 1
		colloc = (KEY_PIN & 0x0E); //read the columns
		if (colloc != 0x0E) //column detected
		{
			rowloc = 1; //save row location
			break; //exit while loop
		}
		KEY_PORT = 0xBE; //ground row 2
		colloc = (KEY_PIN & 0x0E); //read the columns
		if (colloc != 0x0E) //column detected
		{
			rowloc = 2; //save row location
			break; //exit while loop
		}
		KEY_PORT = 0x7E; //ground row 3
		colloc = (KEY_PIN & 0x0E); //read the columns
		rowloc = 3; //save row location
		break; //exit while loop
	}
	
	char key = (colloc == 0x0C) ? keypad[rowloc][0] :
	(colloc == 0x0A) ? keypad[rowloc][1] : keypad[rowloc][2];

	_delay_ms(50); // Final debounce delay
	return key;
}

#endif /* KEYPAD_H_ */