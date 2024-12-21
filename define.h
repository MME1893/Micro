/*
 * define.h
 *
 * Created: 26/09/1403 12:07:46
 *  Author: ARTA PC
 */ 


#ifndef DEFINE_H_
#define DEFINE_H_

/*
*
*	Main Code
*
*/


#define F_CPU 8000000UL  // 8 MHz clock frequency
#define BAUD 9600
#define MYUBRR ((F_CPU / 16 / BAUD) - 1)
#define SLEEP_TIME  2000  
#define CLEARE_VIRTUAL_TERMINAL 1
#define ATTENDANCE_TIME_LIMIT 15 * 60 // 15 minutes in seconds
#define MENU_PAGE_SIZE 4

#define US_PORT PORTD     // Ultrasonic sensor connected to PORTD
#define US_PIN PIND       // Ultrasonic PIN register
#define US_DDR DDRD     // Ultrasonic data direction register

#define US_TRIG_POS PD5   // Trigger pin connected to PD5
#define US_ECHO_POS PD6   // Echo pin connected to PD6



// Error indicators
#define US_ERROR -1       // Error indicator
#define US_NO_OBSTACLE -2 // No obstacle indicator
#define BUZZER_PIN PB7



/*
*
*	 EEPROM
*
*/

#define STUDENT_CODE_SIZE 4
#define EEPROM_START_ADDR 0x00
#define EEPROM_MAX_RECORDS 100
#define EEPROM_SIZE 1024
#define ERROR_DUPLICATED_STD_CODE 0
#define SUCCESS_SAVED_STD_CODE 1
#define ERROR_EEPROM_FULL 2
#define STUDENT_EXIST 1
#define STUDENT_NOT_FOUNDED 0
#define SUCCESS_STUDENT_DELETED 0
#define ERROR_DELETE_STUDENT 1

/*
*
*   KeyPad
*
*/

#define KEY_PORT PORTA //keyboard PORT
#define KEY_DDR DDRA //keyboard DDR
#define KEY_PIN PINA //keyboard PIN



/*
*
*	LCD
*
*/

#define LCD_DPRT PORTC //LCD DATA PORT
#define LCD_DDDR DDRC //LCD DATA DDR
#define LCD_DPIN PINC //LCD DATA PIN
#define LCD_CPRT PORTD //LCD COMMANDS PORT
#define LCD_CDDR DDRD //LCD COMMANDS DDR
#define LCD_CPIN PIND //LCD COMMANDS PIN
#define LCD_EN 0 //LCD EN
#define LCD_RW 3 //LCD RW
#define LCD_RS 4 //LCD RS



#endif /* DEFINE_H_ */