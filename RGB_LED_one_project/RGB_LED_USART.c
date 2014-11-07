//***************************************************************************
//
//  Author(s)...: Pashgan    http://ChipEnable.Ru
//
//  Target(s)...: ATMega8535
//
//  Compiler....: WINAVR
//
//  Description.: USART/UART. ���������� ��������� �����
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

//���������� �����
volatile Buff txBuf;

//�������� �����
volatile Buff rxBuf;

//������������� usart`a
void USART_Init(void)
{
	UBRRH = 0;
	UBRRL = (((F_CPU / (UART_BAUD_RATE * 16UL))) - 1); //�������� ������
	UCSRB = (1<<RXCIE)|(1<<TXCIE)|(1<<RXEN)|(1<<TXEN); //����. ������ ��� ������ � ��������, ���� ������, ���� ��������.
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); //������ ����� 8 ��������
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
//���������� ����������� �������� ����������� ������
unsigned char USART_GetTxCount(void)
{
	return txBuf.Count;
}

//"�������" ���������� �����
void USART_FlushTxBuf(void)
{
	txBuf.Tail = 0;
	txBuf.Count = 0;
	txBuf.Head = 0;
}

//�������� ������ � �����, ���������� ������ ��������
void USART_PutChar(unsigned char sym)
{
	//���� ������ usart �������� � ��� ������ ������
	//����� ��� ����� � ������� UDR
	if(((UCSRA & (1<<UDRE)) != 0) && (txBuf.Count == 0)) UDR = sym;
	else {
		while(txBuf.Count==SIZE_BUF) {
			
		}
		txBuf.Buf[txBuf.Tail] = sym; //�������� � ���� ������
		txBuf.Count++;                   //�������������� ������� ��������
		txBuf.Tail++;                 //� ������ ������ ������
		if (txBuf.Tail == SIZE_BUF) txBuf.Tail = 0;
	}
}

//������� ���������� ������ �� usart`�
void USART_SendStr(char * data)
{
	unsigned char sym;
	while(*data){
		sym = *data++;
		USART_PutChar(sym);
	}
}



//���������� ���������� �� ���������� ��������
ISR(USART_TXC_vect)
{
	if (txBuf.Count > 0){              //���� ����� �� ������
		UDR = txBuf.Buf[txBuf.Head]; //���������� � UDR ������ �� ������
		txBuf.Count--;                   //��������� ������� ��������
		txBuf.Head++;                 //�������������� ������ ������ ������
		if (txBuf.Head == SIZE_BUF) txBuf.Head = 0;
	}
}

//______________________________________________________________________________
//���������� ����������� �������� ����������� � �������� ������
unsigned char USART_GetRxCount(void)
{
	return rxBuf.Count;
}

//"�������" �������� �����
void USART_FlushRxBuf(void)
{
	unsigned char saveSreg = SREG;
	cli();
	rxBuf.Tail = 0;
	rxBuf.Head = 0;
	rxBuf.Count = 0;
	SREG = saveSreg;
}

//������ ������
unsigned char USART_GetChar(void)
{
	unsigned char sym;
	if (rxBuf.Count > 0){                     //���� �������� ����� �� ������
		sym = rxBuf.Buf[rxBuf.Head];        //��������� �� ���� ������
		rxBuf.Count--;                          //��������� ������� ��������
		rxBuf.Head++;                        //���������������� ������ ������ ������
		if (rxBuf.Head == SIZE_BUF) rxBuf.Head = 0;
		return sym;                         //������� ����������� ������
	}
	return 0;
}


//���������� �� ���������� ������
ISR(USART_RXC_vect)
{
	if (rxBuf.Count < SIZE_BUF){                //���� � ������ ��� ���� �����
		rxBuf.Buf[rxBuf.Tail] = UDR;        //������� ������ �� UDR � �����
		rxBuf.Tail++;                             //��������� ������ ������ ��������� ������
		if (rxBuf.Tail == SIZE_BUF) rxBuf.Tail = 0;
		rxBuf.Count++;                                 //��������� ������� �������� ��������
	}
}
