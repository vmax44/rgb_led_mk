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

typedef struct
{
	unsigned char Buf[SIZE_BUF];
	unsigned char Tail;
	unsigned char Head;
	unsigned char Count;
} Buff;

//передающий буфер
volatile Buff txBuf;

//приемный буфер
volatile Buff rxBuf;

//инициализация usart`a
void USART_Init(void)
{
	UBRRH = 0;
	UBRRL = (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1); //скорость обмена
	UCSRB = (1<<RXCIE)|(1<<TXCIE)|(1<<RXEN)|(1<<TXEN); //разр. прерыв при приеме и передачи, разр приема, разр передачи.
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); //размер слова 8 разрядов
	USART_FlushRxBuf();
	USART_FlushTxBuf();
	/*	rxBuf.Buf[0]=0x55;
		rxBuf.Buf[1]=253;
		rxBuf.Buf[2]=3;
		rxBuf.Buf[3]=5;
		rxBuf.Buf[4]=16;
		rxBuf.Buf[5]=0;
		rxBuf.Buf[6]=25;
		rxBuf.Buf[7]=30;
		rxBuf.Buf[8]=35;
		rxBuf.Buf[9]=40;
		rxBuf.Buf[10]=45;
		rxBuf.Buf[11]=50;
		rxBuf.Buf[12]=55;
		rxBuf.Buf[13]=60;
		rxBuf.Buf[14]=65;
		rxBuf.Buf[15]=70;
		rxBuf.Buf[16]=75;
		rxBuf.Buf[17]=80;
		rxBuf.Buf[18]=85;
		rxBuf.Buf[19]=90;
		rxBuf.Buf[20]=95;
		rxBuf.Buf[21]=100;
		rxBuf.Count=22;
		rxBuf.Head=0;
		rxBuf.Tail=0;*/
}

//______________________________________________________________________________
//возвращает колличество символов передающего буфера
unsigned char USART_GetTxCount(void)
{
	return txBuf.Count;
}

//"очищает" передающий буфер
void USART_FlushTxBuf(void)
{
	txBuf.Tail = 0;
	txBuf.Count = 0;
	txBuf.Head = 0;
}

//помещает символ в буфер, инициирует начало передачи
void USART_PutChar(unsigned char sym)
{
	//если модуль usart свободен и это первый символ
	//пишем его прямо в регистр UDR
	if(((UCSRA & (1<<UDRE)) != 0) && (txBuf.Count == 0)) UDR = sym;
	else {
		while(txBuf.Count==SIZE_BUF) {
			
		}
		txBuf.Buf[txBuf.Tail] = sym; //помещаем в него символ
		txBuf.Count++;                   //инкрементируем счетчик символов
		txBuf.Tail++;                 //и индекс хвоста буфера
		if (txBuf.Tail == SIZE_BUF) txBuf.Tail = 0;
	}
}

//функция посылающая строку по usart`у
void USART_SendStr(char * data)
{
	unsigned char sym;
	while(*data){
		sym = *data++;
		USART_PutChar(sym);
	}
}



//обработчик прерывания по завершению передачи
ISR(USART_TXC_vect)
{
	if (txBuf.Count > 0){              //если буфер не пустой
		UDR = txBuf.Buf[txBuf.Head]; //записываем в UDR символ из буфера
		txBuf.Count--;                   //уменьшаем счетчик символов
		txBuf.Head++;                 //инкрементируем индекс головы буфера
		if (txBuf.Head == SIZE_BUF) txBuf.Head = 0;
	}
}

//______________________________________________________________________________
//возвращает колличество символов находящихся в приемном буфере
unsigned char USART_GetRxCount(void)
{
	return rxBuf.Count;
}

//"очищает" приемный буфер
void USART_FlushRxBuf(void)
{
	unsigned char saveSreg = SREG;
	cli();
	rxBuf.Tail = 0;
	rxBuf.Head = 0;
	rxBuf.Count = 0;
	SREG = saveSreg;
}

//чтение буфера
unsigned char USART_GetChar(void)
{
	unsigned char sym;
	if (rxBuf.Count > 0){                     //если приемный буфер не пустой
		sym = rxBuf.Buf[rxBuf.Head];        //прочитать из него символ
		rxBuf.Count--;                          //уменьшить счетчик символов
		rxBuf.Head++;                        //инкрементировать индекс головы буфера
		if (rxBuf.Head == SIZE_BUF) rxBuf.Head = 0;
		return sym;                         //вернуть прочитанный символ
	}
	return 0;
}


//прерывание по завершению приема
ISR(USART_RXC_vect)
{
	if (rxBuf.Count < SIZE_BUF){                //если в буфере еще есть место
		rxBuf.Buf[rxBuf.Tail] = UDR;        //считать символ из UDR в буфер
		rxBuf.Tail++;                             //увеличить индекс хвоста приемного буфера
		if (rxBuf.Tail == SIZE_BUF) rxBuf.Tail = 0;
		rxBuf.Count++;                                 //увеличить счетчик принятых символов
	}
}
