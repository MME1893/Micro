#include <avr/io.h>
#include <util/delay.h>
#include "define.h"
#ifndef LCD_H_
#define LCD_H_


void lcd_command( unsigned char cmnd )
{
	LCD_DPRT = cmnd; //send cmnd to data port
	LCD_CPRT &= ~ (1<<LCD_RS); //RS = 0 for command
	LCD_CPRT &= ~ (1<<LCD_RW); //RW = 0 for write
	LCD_CPRT |= (1<<LCD_EN); //EN = 1 for H-to-L pulse
	_delay_us(SLEEP_TIME); //wait to make enable wide
	LCD_CPRT &= ~ (1<<LCD_EN); //EN = 0 for H-to-L pulse
	_delay_us(SLEEP_TIME); //wait to make enable wide
}

void lcd_data( unsigned char data )
{
	LCD_DPRT = data; //send data to data port
	LCD_CPRT |= (1<<LCD_RS); //RS = 1 for data
	LCD_CPRT &= ~ (1<<LCD_RW); //RW = 0 for write
	LCD_CPRT |= (1<<LCD_EN); //EN = 1 for H-to-L pulse
	_delay_us(SLEEP_TIME); //wait to make enable wide 2000
	LCD_CPRT &= ~ (1<<LCD_EN); //EN = 0 for H-to-L pulse
	_delay_us(SLEEP_TIME); //wait to make enable wide 2000
}

void lcd_init()
{
	LCD_DDDR = 0xFF;
	LCD_CDDR = 0xFF;
	LCD_CPRT &=~(1<<LCD_EN); //LCD_EN = 0
	_delay_us(SLEEP_TIME); //wait for init.
	lcd_command(0x38); //init. LCD 2 line, 5 × 7 matrix
	lcd_command(0x0C); //display on, cursor off
	lcd_command(0x01); //clear LCD
	_delay_us(SLEEP_TIME); //wait
	lcd_command(0x06); //shift cursor right
	
	
}void lcd_print( char * str )
{
	unsigned char i = 0;
	while(str[i]!=0)
	{
		lcd_data(str[i]);
		i++ ;
	}
}

//void lcd_create_char(uint8_t location, uint8_t charmap[]) {
	//location &= 0x07; // Only 8 locations (0-7) available in CGRAM
	//lcd_command(0x40 + (location * 8)); // Set CGRAM address
//
	//for (int i = 0; i < 8; i++) {
		//lcd_data(charmap[i]); // Write character pattern
	//}
//
	//lcd_command(0x80); // Return to DDRAM
//}
//
//// Function to display a custom character
//void lcd_print_custom_char(uint8_t location) {
	//lcd_data(location); // Display the custom character
//}

#endif