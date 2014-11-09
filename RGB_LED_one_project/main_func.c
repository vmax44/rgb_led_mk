/*
* CFile1.c
*
* Created: 17.09.2013 21:51:45
*  Author: maxim
*/
#include "main_func.h"
#include "RGB_LED_USART.h"
#include "TWI_Master.h"
#include <util/delay.h>
//#include "mrtos.h"

Type_TWI to_TWI;

void Make_Command() {
	buffer buf;
	
	unsigned char tmp;
	buf=get_buf();

	to_TWI.string[0]=PCA_addr<<1;
	to_TWI.string[1]=0b10000010;
	for(tmp=0;tmp<USART_command_max_params;tmp++) {
		to_TWI.string[tmp+2]=buf.Channels[tmp];
	}
	to_TWI.count=18;
	to_TWI.ready=TRUE;
}

void Send_to_TWI() {
	if(to_TWI.ready) {

		TWI_Start_Transceiver_With_Data(to_TWI.string,to_TWI.count);
		PORTD=0b10000000;
		while(TWI_Transceiver_Busy()) {
		};
		//PORTD=0b00010000;
		to_TWI.ready=FALSE;
		
	}
}

void test() {
	Type_TWI t;
	
	t.string[0]=0b1110000<<1;
	t.string[1]=0b00010001;
	t.string[2]=0xFF;
	t.count=3;
	t.ready=TRUE;
	TWI_Start_Transceiver_With_Data(t.string,t.count);
	_delay_ms(1000);
	t.string[2]=0x22;
	TWI_Start_Transceiver_With_Data(t.string,t.count);
	while(1) {
		_delay_ms(500);
	}
}
