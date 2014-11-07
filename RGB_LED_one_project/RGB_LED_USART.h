#include <avr/io.h>
#include <avr/interrupt.h>


//������ ������
#define SIZE_BUF 22
#define UART_BAUD_RATE  57600

void USART_Init(void); //������������� usart`a
unsigned char USART_GetTxCount(void); //����� ����� �������� ����������� ������
void USART_FlushTxBuf(void); //�������� ���������� �����
void USART_PutChar(unsigned char sym); //�������� ������ � �����
void USART_SendStr(char * data); //������� ������ �� usart`�
unsigned char USART_GetRxCount(void); //����� ����� �������� � �������� ������
void USART_FlushRxBuf(void); //�������� �������� �����
unsigned char USART_GetChar(void); //��������� �������� ����� usart`a
