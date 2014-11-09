//***************************************************************************
//
//  Author(s)...: Pashgan    http://ChipEnable.Ru
//
//  Target(s)...: ATMega8535
//
//  Compiler....: WINAVR
//
//  Description.: USART/UART. Используем кольцевой буфер
//
//  Data........: 3.01.10
//
//***************************************************************************
#include "RGB_LED_USART.h"

//инициализация usart`a
void USART_Init(void)
{
	UCSRA=0x00;
	UCSRB=0x90;
	UCSRC=0x0E;
	UBRRH=0x00;
	UBRRL = (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1); //скорость обмена
}

buffer buf;
unsigned char dmx_state=0;
unsigned int dmxcount=0;

unsigned char dmx_received() {
	return dmx_state==3;
}

buffer get_buf(void) {
	return buf;
}


//прерывание по завершению приема
ISR(USART_RXC_vect)
{
	static unsigned char status,data;
	status = UCSRA;  //считываем состояние регистра USART
	data = UDR;      //и пришедший байт
	//сначала надо получить метку пакета данных
		if (status&0x10) //0x10 проверяем флаг ошибки стоп бита FE
		{
			dmx_state = 1;
			//    UCSRA=0x20;
			return;
		}
	//получили BREAK, ждем первый нулевой байт
	if (dmx_state == 1)
	{
		if (data == 0)
		{
			dmx_state = 2;
			dmxcount=0;
			return;
		}
		else
		{
			dmx_state=0;
		};
	};
	//если метка получена, значит дальше идут данные о каналах
	if (dmx_state==2)
	{
		++dmxcount;
		if((dmxcount<DMX_ADDRESS+CHANNELS_COUNT) && (dmxcount>=DMX_ADDRESS)) {
			buf.Channels[dmxcount-DMX_ADDRESS]=data;
		};
		if(dmxcount==DMX_ADDRESS+CHANNELS_COUNT) {
			dmx_state=3;
		}
	};
}
