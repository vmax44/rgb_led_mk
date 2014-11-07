/*
 * IncFile1.h
 *
 * Created: 17.09.2013 21:52:15
 *  Author: maxim
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#define VERSION "0001"

int plus(int a, int b);
inline unsigned int cs_calc(unsigned char b);

#define USART_Start_Byte 0x55
#define State_init_state 0
#define State_usart_start_byte_received 1
#define State_usart_CS1_received 2
#define State_usart_CS2_received 3
#define State_usart_command_received 4
#define State_usart_num_of_bytes1_received 5
#define State_usart_num_of_bytes2_received 6
#define State_usart_all_data_received 7  //  -> send "OK" message -> set state depending on Command
#define State_usart_error_while_receiveing 8  // -> send "Error" message -> set State_init_state
#define USART_command_max_params 16
#define USART_TIMEOUT 500

typedef struct
{
    unsigned char command;
    unsigned char received;
    unsigned char params[USART_command_max_params];
    unsigned int params_count;
} Type_command;
void task_ReceiveLoop(void);

#define PCA_addr 0b1110000
#define PCA_first_channel 1
typedef struct
{
	unsigned char string[18];
	unsigned char ready;
	unsigned char count;
} Type_TWI;
void task_CommanderLoop(void);

void task_SendLoop(void);


#define Error_USART_CS 0x43  // "C"
#define Error_TWI 0x54		 // "T"
#define Error_USART_too_many_params 0x55  // "U"
#define Error_USART_TIMEOUT 0x4f		  // "O"
void USART_send_error(unsigned char err);

void USART_send_ok();
void debug_command();
#endif /* INCFILE1_H_ */