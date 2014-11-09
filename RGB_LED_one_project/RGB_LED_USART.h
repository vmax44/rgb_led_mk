#include <avr/io.h>
#include <avr/interrupt.h>

#define UART_BAUD_RATE  250000
#define CHANNELS_COUNT 16
#define DMX_ADDRESS 13

typedef struct {
	unsigned char Channels[CHANNELS_COUNT];
} buffer;

void USART_Init(void); //инициализация usart`a
unsigned char dmx_received();
buffer get_buf(void);