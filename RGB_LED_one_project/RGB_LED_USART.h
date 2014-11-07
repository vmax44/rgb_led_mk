#include <avr/io.h>
#include <avr/interrupt.h>


//размер буфера
#define SIZE_BUF 22
#define UART_BAUD_RATE  57600

void USART_Init(void); //инициализация usart`a
unsigned char USART_GetTxCount(void); //взять число символов передающего буфера
void USART_FlushTxBuf(void); //очистить передающий буфер
void USART_PutChar(unsigned char sym); //положить символ в буфер
void USART_SendStr(char * data); //послать строку по usart`у
unsigned char USART_GetRxCount(void); //взять число символов в приемном буфере
void USART_FlushRxBuf(void); //очистить приемный буфер
unsigned char USART_GetChar(void); //прочитать приемный буфер usart`a
