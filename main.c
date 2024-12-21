#include "lcd.h"
#include "keypad.h"
#include "eeprom.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

// Global Variables

/* Buffer */
volatile char student_code[STUDENT_CODE_SIZE + 1]; // Buffer for keypad input
volatile uint8_t code_index = 0; // Index for the input buffer
volatile uint8_t exit_to_menu = 0; // Flag to signal exit from a mode
volatile char last_key = '\0';   // Last key pressed

/* Menu */
volatile char is_in_function = 0;
volatile char current_page = 0;
volatile char last_page = 0;


/* Ultra sonic */
volatile int count = 0;


/* Timer */
volatile uint16_t elapsed_time = 0; // Timer counter
volatile uint8_t time_over = 0;     // Flag for timeout
volatile char is_first_time_att_start = 1;     // Attendence mode 1st time


// Function Prototypes
void interrupt_init();
void show_menu(char * page);
void attendance_mode();
void check_attendance();
void display_attendees();
void temperature_info();
void student_info_via_usart();
void sonar_info();
void delete_attendance();
void timer1_init();
void previous_menu_page();
void next_menu_page();
void buzzerr();
void setup_buzzerr();



void HCSR04Init()
{
	US_DDR |= (1 << US_TRIG_POS);  // Trigger pin as output
	US_DDR &= ~(1 << US_ECHO_POS); // Echo pin as input
}

void HCSR04Trigger()
{
	US_PORT |= (1 << US_TRIG_POS);  // Set trigger pin high
	_delay_ms(SLEEP_TIME);          // Wait for 0.250 second 
	US_PORT &= ~(1 << US_TRIG_POS); // Set trigger pin low
}

uint16_t GetPulseWidth()
{
	uint32_t i, result;

	// Wait for rising edge on Echo pin
	for (i = 0; i < 4800000; i++)  // 600000 8Mhz
	{
		if (!(US_PIN & (1 << US_ECHO_POS)))
		continue;
		else
		break;
	}

	if (i == 4800000)
	return US_ERROR; // Timeout error if no rising edge detected

	// Start timer with prescaler 8
	TCCR1A = 0x00;
	TCCR1B = (1 << CS11) | (1 << CS10);
	TCNT1 = 0x00; // Reset timer

	// Wait for falling edge on Echo pin
	for (i = 0; i < 4800000; i++)
	{
		if (!(US_PIN & (1 << US_ECHO_POS)))
		break; // Falling edge detected
		if (TCNT1 > 7500) // 60000
		return US_NO_OBSTACLE; // No obstacle in range
	}

	result = TCNT1; // Capture timer value
	TCCR1B = 0x00;  // Stop timer

	if (result > 7500) // 600000
	return US_NO_OBSTACLE;
	else
	return (result >> 1); // Return the measured pulse width
}


void clear_and_go_home(){
	lcd_command(0x01);
	lcd_command(0x02);
	make_buffer_empty();
}


void make_buffer_empty(){
	int i = 0;
	for (; i < STUDENT_CODE_SIZE + 1 ; i++ ){
		student_code[i] = '\0';
	}
	last_key = '\0';
	code_index = 0;
	exit_to_menu = 0;
}

// ISR for Timer
ISR(TIMER1_COMPA_vect) {
	if (elapsed_time < ATTENDANCE_TIME_LIMIT) {
		elapsed_time++;
		} else {
		time_over = 1;         // Set the time-over flag
		TCCR1B &= ~((1 << CS12) | (1 << CS10)); // Stop the timer by clearing prescaler bits
	}
}

// ISR for Keypad Input
ISR(INT2_vect) {
	char key = keypad_scan(); // Get the pressed key
	
	// If '#' is pressed, set exit flag
	if (key == '#' && is_in_function) {
		exit_to_menu = 1;
		keypad_init();
		return;
	}

	// Save key to the buffer if not full
	if (code_index < STUDENT_CODE_SIZE && key != '\0') {
		student_code[code_index++] = key;
	}

	// Store last key for general menu navigation
	last_key = key;
	keypad_init();
}



int main() {
	
	HCSR04Init();     
	lcd_init();
	keypad_init();
	interrupt_init();
	setup_buzzerr();


	show_menu(&current_page);
	
	while (1) {
		
		// Wait for user to select an option
		while (last_key == '\0'){
					
		};
		char option = last_key;
		last_key = '\0'; // Reset last key
		clear_and_go_home();
		
	
		switch (option) {
			case '1':		
				is_in_function = 1;	
				attendance_mode();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '2':
				is_in_function = 1;
				check_attendance();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '3':
				is_in_function = 1;
				display_attendees();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '4':
				is_in_function = 1;
				temperature_info();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '5':
				is_in_function = 1;
				student_info_via_usart();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '6':
				is_in_function = 1;
				sonar_info();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '7':
				is_in_function = 1;
				reset_eeprom();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '8':
				is_in_function = 1;
				delete_attendance();
				is_in_function = 0;
				show_menu(&current_page);
				break;
			case '#':
				 last_page = current_page;
				 current_page = (current_page + 1) % MENU_PAGE_SIZE;
				 show_menu(&current_page);
				break;
			case '*':
				last_page = current_page;
				current_page = (current_page - 1 + MENU_PAGE_SIZE) % MENU_PAGE_SIZE;
				show_menu(&current_page);
				break;
			default:
				lcd_print(" Invalid Option");
				_delay_ms(SLEEP_TIME);
				show_menu(&current_page);
				break;
		}

	}

	return 0;
}



void sonar_info() {
	char numberString[16];  // Buffer to hold distance string
	uint16_t pulseWidth;    // Pulse width from echo
	int distance;           // Calculated distance
	int previous_count = -1;

	while (1) {
		
		_delay_ms(100);  // Delay for sensor stability
		//HCSR04Init(); 
		if (exit_to_menu)
		{
			exit_to_menu = 0;
			clear_and_go_home();
			return;
		}
		HCSR04Trigger();              // Send trigger pulse
		pulseWidth = GetPulseWidth();  // Measure echo pulse

		if (pulseWidth == US_ERROR) {
			clear_and_go_home();
			lcd_command(0x00);
			lcd_print("Error");        // Display error message
			} else if (pulseWidth == US_NO_OBSTACLE) {
			clear_and_go_home();
			lcd_command(0x00);
			lcd_print("No Obstacle");  // Display no obstacle message
			} else {
				
			distance = (int)((pulseWidth * 0.035 * 8 * 2 / 2) + 0.5); // 8Mhz

			// Display distance on LCD
			sprintf(numberString, "%d", distance); // Convert distance to string
			clear_and_go_home();
			lcd_command(0x00);
			lcd_print("Distance: ");
			lcd_print(numberString);
			lcd_print(" cm");
			 
			// Counting logic based on distance
			if (distance < 10) {
				count++;  // Increment count if distance is below threshold
			}

			
			// Update count on LCD only if it changes
			if (count != previous_count) {
				previous_count = count;
				lcd_command(0xC0); // Move to second line
				sprintf(numberString, "%d", count);
				lcd_print("Count: ");
				lcd_print(numberString);
			}
						
			_delay_ms(5000);
		}
	}
}


void interrupt_init() {
	DDRB &= ~(1 << 2);  // Set INT2 pin as input
	PORTB |= (1 << 2);  // Enable pull-up on INT2
	MCUCSR |= (1 << ISC2); // Trigger INT2 on rising edge
	GICR = (1 << INT2);  // Enable INT2 interrupt
	sei();               // Enable global interrupts
}


void show_menu(char * page) {
	switch(*page)
	{
		case 0:	
				lcd_command(0x01); // Clear LCD
				lcd_print(" 1. Attendance");
				lcd_command(0xC0);
				lcd_print(" 2. Check-in");
		break;
		case 1:
				lcd_command(0x01);
				lcd_print(" 3. View Students");
				lcd_command(0xC0);
				lcd_print(" 4. Temp Sensor");
		break;
		case 2:
				lcd_command(0x01);
				lcd_print(" 5. Student Info");
				lcd_command(0xC0);
				lcd_print(" 6. Sonar Info");
		break;
		case 3:		
				lcd_command(0x01);
				lcd_print(" 7. Delete EEPROM");
				lcd_command(0xC0);
				lcd_print(" 8. Delete Student");
		break;
	}
}


void attendance_mode() {
	lcd_command(0x01);
	lcd_print(" Attendance Ready");
	_delay_ms(SLEEP_TIME);
	if (is_first_time_att_start == 1){
			timer1_init();
			lcd_command(0x01);
			lcd_print(" Timer started:)");
			_delay_ms(SLEEP_TIME);
			is_first_time_att_start = 0;
	}

	code_index = 0;
	exit_to_menu = 0;

	lcd_command(0x01);
	lcd_print(" Enter Code:");

	while (!time_over) 
	{
		if (exit_to_menu)
		{
			exit_to_menu = 0; 
			clear_and_go_home();
			return;
		}

		// Display the code entered so far
		lcd_command(0xC0); // Move to second line
		for (uint8_t i = 0; i < code_index; i++)
		{
			lcd_data(student_code[i]);
		}
		
		//TODO PUT DELAY HERE ...
		_delay_ms(SLEEP_TIME);
		
		// If 8 digits entered, process the code
		if (code_index == STUDENT_CODE_SIZE) 
		{
			student_code[STUDENT_CODE_SIZE] = '\0'; // Null-terminate, to use in eeprom functions
			uint8_t status = save_student_code((char *)student_code);
			
			lcd_command(0x01); // Clear screen
			if (status == SUCCESS_SAVED_STD_CODE)
			{
				lcd_print(" Code Saved!");
			}
			else
			if (status == ERROR_DUPLICATED_STD_CODE)
			{
				lcd_print(" Duplicate Code");
				buzzerr();
			}
			else if (status == ERROR_EEPROM_FULL) 
			{
				lcd_print(" EEPROM Full!");
				buzzerr();
			}
			_delay_ms(SLEEP_TIME);

			code_index = 0; // Reset buffer
			lcd_command(0x01);
			lcd_print(" Enter Code:");
		}
	}
	// it means time is over here :(
	
	lcd_command(0x01);
	lcd_print(" TIME IS OVER...");
	_delay_ms(SLEEP_TIME);
	clear_and_go_home();
	return;
}


void delete_attendance(){
	lcd_command(0x01);
	lcd_print(" Enter Code:");

	code_index = 0;
	exit_to_menu = 0;

	while (1) {
		if (exit_to_menu) {
			exit_to_menu = 0;
			clear_and_go_home();
			return;
		}

		lcd_command(0xC0);
		for (uint8_t i = 0; i < code_index; i++) {
			lcd_data(student_code[i]);
		}

		if (code_index == STUDENT_CODE_SIZE) {
			student_code[STUDENT_CODE_SIZE] = '\0';
			uint8_t found = delete_student_code((char *)student_code);

			lcd_command(0x01);
			if (found == SUCCESS_STUDENT_DELETED) {
				lcd_print(" Student Deleted!");
				} else {
				lcd_print(" Std not founded!");
				buzzerr();
			}
			_delay_ms(SLEEP_TIME);

			clear_and_go_home();
			
			return;
		}
	}
}


void check_attendance() {
	lcd_command(0x01);
	lcd_print(" Enter Code:");

	code_index = 0;
	exit_to_menu = 0;

	while (1) {
		if (exit_to_menu) {
			exit_to_menu = 0;
			clear_and_go_home();
			return;
		}

		lcd_command(0xC0);
		for (uint8_t i = 0; i < code_index; i++) {
			lcd_data(student_code[i]);
		}

		if (code_index == STUDENT_CODE_SIZE) {
			student_code[STUDENT_CODE_SIZE] = '\0'; 
			uint8_t found = check_student_code((char *)student_code);

			lcd_command(0x01);
			if (found) {
				lcd_print(" Present");
				} else {
				lcd_print(" Absent");
				buzzerr();
			}
			_delay_ms(SLEEP_TIME);

			clear_and_go_home();
			
			return;
		}
	}
}


void display_attendees() {
	lcd_command(0x01);
	lcd_print(" attendees:");
	 display_student_codes();
	_delay_ms(SLEEP_TIME); // /2
	clear_and_go_home();
}


void temperature_info() {
	lcd_command(0x01);
	lcd_print(" Temp:");
	int temp = 0;
	char buffer[16];
	ADMUX = 0xE0;
	ADCSRA = 0x87;
		
	while (1) {
		if (exit_to_menu) {
			exit_to_menu = 0;
			clear_and_go_home();
			ADCSRA = 0x0;
			return;
		}

		 //read_temperature(); // Function to read temperature
		ADCSRA |= 1 << ADSC;
		
		while((ADCSRA & (1 << ADIF)) == 0);
		
		if (ADCH != temp || 1){
			temp = ADCH;
			snprintf(buffer, sizeof(buffer), "%d C", temp);
			lcd_command(0xC0);
			lcd_print(buffer);
			
		}
	_delay_ms(SLEEP_TIME); // 2
	}
}


void student_info_via_usart() {
	lcd_command(0x01);
	lcd_print(" USART: Sending");
	send_all_student_info_usrt(); // Function to send all info via USART
	_delay_ms(SLEEP_TIME);
	clear_and_go_home();
}


void timer1_init() {
	TCCR1B |= (1 << WGM12);               // Configure Timer1 in CTC mode
	OCR1A = 15624;                        // Compare value for 1-second interrupt (16MHz/1024)
	TCCR1B |= (1 << CS12) | (1 << CS10);  // Prescaler of 1024
	TIMSK |= (1 << OCIE1A);               // Enable Timer1 compare match interrupt
	//sei();                                // Enable global interrupts
}


void buzzerr()
{
	PORTB |= (1 << BUZZER_PIN);
	_delay_ms(SLEEP_TIME);
	PORTB &= ~(1 << BUZZER_PIN);
	_delay_ms(SLEEP_TIME);
}


void setup_buzzerr() {
	DDRB |= (1 << BUZZER_PIN);
}
