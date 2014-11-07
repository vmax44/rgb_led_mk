/*
* RGB_LED.c
*
* Created: 08.08.2013 22:32:45
*  Author: maxim

Hello World!

Device controls the color and brightness of the RGB LEDs
To the device can be connected to up to 4 RGB LEDs or up to 16 single-color LEDs.

The device is built on the following chips:
ATMEGA8 - provides device control and data exchange with a PC;
PCA9635PW - performs PWM-dimming LEDs;
PT4115B89E - LED drive power - on each channel
HC06 (optionally) - Wireless Serial 6 Pin Bluetooth RF Transceiver Module

Communication protocol:

test: 0x55 0x6B 0x00 0x03 0x02 0x00 0x07 0x5F

Request from PC: 0x55 CSl CSh Command Nl Nh P1 ... Pn
CS - Check sum (begins from Command)
Nl,Nh - num of parameters in bytes
P1 ... Pn - n bytes of parameters, depending on the Command

Response to PC: 0x66 CSl CSh OK P1 ... Pn
CS - Check sum (begins from OK)
OK - 1 if Ok, 0 if error
P1 ... Pn - n bytes of parameters, depending on the Command

Commands:
Command = 1 : Turn off all LEDs
Params: no params
Response: 1 without params

Command = 2 : Turn on all LEDS
Params: no params
Response: 1 without params

Command = 3 : Set bright single channel
Params: P1 - channel number
P2 - bright (PWM value) for this channel
Response: 1 without params

Command = 4 : Save program for single channel
Params: P1 - channel number
P2 - bright (PWM value) for this channel
P3,P4 - delay in mSec to next bright (P3 - low byte, P4 - hi byte)
Response: 1 without params

Command = 5 : Set bright for all channels
Params: P1 .. P16 - bright (PWM value) for channels
response: 1 without params

Command = 6 : Save program for all channels
Params: P1 - bright for first channel
P2 -

Command = 10 : Program Version
Params: no params
Response: 1
Params: P1 - version number

Main loop:



*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "TWI_Master.h"
#include "RGB_LED_USART.h"
#include "RGB_LED_init.h"
#include "mrtos.h"
#include "main_func.h"

#define PWM_Values_buf_size 16

unsigned char PWM_Values_buf[PWM_Values_buf_size];

void task_ReceiveLoop(void);

// Send commands to TWI
void task_SendLoop(void);

void task_CommanderLoop(void);

int main(void)
{
    RGB_LED_init();
    TWI_Master_Initialise();
    USART_Init();

    sei();

    mRTOS_Init();
    mRTOS_CreateTask(task_ReceiveLoop,10,ACTIVE);
	mRTOS_CreateTask(task_CommanderLoop,10,ACTIVE);
	mRTOS_CreateTask(task_SendLoop,10,ACTIVE);
    

    
    mRTOS_Scheduler();
    
    return 0;
}
