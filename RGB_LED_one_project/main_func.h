/*
 * IncFile1.h
 *
 * Created: 17.09.2013 21:52:15
 *  Author: maxim
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#define VERSION "0001"

#define USART_command_max_params 16

typedef struct
{
    unsigned char command;
    unsigned char received;
    unsigned char params[USART_command_max_params];
    unsigned int params_count;
} Type_command;

#define PCA_addr 0b1110000
#define PCA_first_channel 1
typedef struct
{
	unsigned char string[18];
	unsigned char ready;
	unsigned char count;
} Type_TWI;

void Make_Command();
void Send_to_TWI();
void test();
#endif /* INCFILE1_H_ */