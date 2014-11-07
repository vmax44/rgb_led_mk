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

PORTD = 0x06;
DDRD = 0x06;

TCCR0 = 0x00;
TCNT0 = 0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer 1 Stopped
// Mode: Normal top=FFFFh
// OC1 output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare Match Interrupt: Off
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
TIMSK = 0x01;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR = 0x80;
    

    
}
