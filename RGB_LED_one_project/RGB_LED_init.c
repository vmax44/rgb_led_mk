/*
 * RGB_LED_init.c
 *
 * Created: 19.08.2013 21:33:44
 *  Author: maxim
 */ 
#include "avr/io.h"

void RGB_LED_init() {
PORTB = 0x00;
DDRB = 0x00;

PORTD = 0b00000000;
DDRD = 0b11111100;

TCCR0 = 0x00;
TCNT0 = 0x00;

TCCR1A = 0x00;
TCCR1B = 0x00;
TCNT1 = 0x00;
OCR1A = 0x00;
OCR1B = 0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
GIMSK = 0x00;
MCUCR = 0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK = 0x00;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR = 0x80;
    

    
}
