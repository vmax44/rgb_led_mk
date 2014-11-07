/*
* CFile1.c
*
* Created: 17.09.2013 21:51:45
*  Author: maxim
*/
#include "main_func.h"
#include "RGB_LED_USART.h"
#include "TWI_Master.h"
#include "mrtos.h"

int plus(int a, int b) {
    return a+b;
}

inline unsigned int cs_calc(unsigned char b) {
    return b;
}

volatile Type_command USART_command;

// Receive commands by USART
void task_ReceiveLoop(void)
{
    volatile static unsigned char state,csl,csh,nl,nh;
    volatile static unsigned int cs,cs_need;
    volatile static unsigned char count_in_USART;
    volatile static unsigned char PWM_Values_index;
    volatile static unsigned char tmp;
    
    state=State_init_state;
    while(1) {
        count_in_USART=USART_GetRxCount();
        switch (state) {
            //Init state - not received start byte
            case State_init_state:      
            if(count_in_USART && (!USART_command.received))  // if previous command finished by other tasks and in USART buffer have received chars
                if(USART_Start_Byte==USART_GetChar()) {    // if received start byte
					mRTOS_SetSystemTime(0);    // system time is used for check timeout while receive
                    state=State_usart_start_byte_received;
                }
            break;
            
            //start byte received, next 2 bytes - CS
            case State_usart_start_byte_received:       
            if(count_in_USART) {
                csl=USART_GetChar();
                state=State_usart_CS1_received;
            }
            break;
            case State_usart_CS1_received:
            if(count_in_USART) {
                csh=USART_GetChar();
                cs_need=((unsigned int)csh<<8) | ((unsigned int)csl); //CS received from Master
                cs=0;                                                 //CS calculated 
                state=State_usart_CS2_received;
            }
            break;
            
            //CS received, next byte - command code
            case State_usart_CS2_received:
            if(count_in_USART) {
                USART_command.command=USART_GetChar();
                cs+=cs_calc(USART_command.command); // adding command code to CS
                state=State_usart_command_received;
            }
            break;
            
            //Command is received, the following 2 bytes - number of bytes of parameters
            case State_usart_command_received:    
            if(count_in_USART) {
                nl=USART_GetChar();    //receiving low byte
                cs+=cs_calc(nl);
                state=State_usart_num_of_bytes1_received;
            }
            break;
            case State_usart_num_of_bytes1_received:
            if(count_in_USART) {
                nh=USART_GetChar();    //receiving Hight byte
                cs+=cs_calc(nh);
                USART_command.params_count=((unsigned int)nh<<8)|((unsigned int)nl);
                if(USART_command.params_count>USART_command_max_params) {   //if received num of params more than max params, then error while USART receiving
                    state=State_usart_error_while_receiveing;
                    USART_send_error(Error_USART_too_many_params);
                } else {
                    if(USART_command.params_count) {   // if params_count > 0 go to receiveing params
                        state=State_usart_num_of_bytes2_received;
                        PWM_Values_index=0;
                    } else {             // else all data received
                        state=State_usart_all_data_received;
                    }
                }
            }
            break;
            case State_usart_num_of_bytes2_received:
            //receiving params to temp buf
            if(count_in_USART) {
                tmp=USART_GetChar();             //get byte
                cs+=cs_calc(tmp);                //and calc check summ
                USART_command.params[PWM_Values_index++]=tmp;
                if(PWM_Values_index>=USART_command.params_count) {  //if all params received
                    state=State_usart_all_data_received;
                }
            }
            break;
            
            case State_usart_all_data_received:
            if(cs!=cs_need) {                         //if CS not match
                state=State_usart_error_while_receiveing;  //send error message
                USART_send_error(Error_USART_CS);
            } else {
                state=State_init_state;
                USART_command.received=TRUE;
				USART_send_ok();
                //mRTOS_TASK_ACTIVE()  //start task task_CommanderLoop
            }
            break;
            case State_usart_error_while_receiveing:
            state=State_init_state;     //if error while receiving
            USART_FlushRxBuf();         //clean receiver buffer
            sei();
            break;
        }
        if((state!=State_init_state) || count_in_USART) {
            if((state!=State_init_state) && (mRTOS_GetSystemTime()>USART_TIMEOUT)) {
				state=State_usart_error_while_receiveing;
				USART_send_error(Error_USART_TIMEOUT);
			}
			mRTOS_DISPATCH;
			
        } else {
            mRTOS_TASK_WAIT(50);
        }
    }
}



volatile Type_TWI to_TWI;

void task_CommanderLoop(void) {
    volatile static unsigned char tmp;

	to_TWI.count=0;
	to_TWI.ready=FALSE;
	tmp=0;
    while(1) {
        if(USART_command.received && (!to_TWI.ready)) {
            switch(USART_command.command) {
                case 1:
                break;
                case 2:
                break;
                case 3:
                to_TWI.string[0]=PCA_addr<<1;
                to_TWI.string[1]=USART_command.params[0]+PCA_first_channel;
                to_TWI.string[2]=USART_command.params[1];
                to_TWI.count=3;
                to_TWI.ready=TRUE;
                break;
                case 4:
                break;
                case 5:
				USART_send_ok();
                to_TWI.string[0]=PCA_addr<<1;
                to_TWI.string[1]=0b10000011;
                for(tmp=0;tmp<16;tmp++) {
                    to_TWI.string[tmp+2]=USART_command.params[tmp];
                }
                to_TWI.count=18;
                to_TWI.ready=TRUE;
				USART_send_ok();
                break;
                case 6:
                break;
                case 10:
                USART_SendStr("1");
                USART_SendStr(VERSION);
                break;
				default:
				break;
            } 
            USART_command.received=FALSE;
        }
        mRTOS_TASK_WAIT(10);
        
    }
}


#define SEND_DATA 1
#define WAIT_TRANSMIT 2

/*void debug_command() 
{
	USART_PutChar(USART_command.command);
	//USART_SendStr("\r\n");
	USART_PutChar(USART_command.params_count);
	//USART_SendStr("\r\n");
	USART_PutChar(USART_command.received);
	//USART_SendStr("\r\n");
	unsigned char i;
	for(i=0;i<USART_command.params_count;i++) {
		USART_PutChar(USART_command.params[i]);
	}
	//USART_SendStr("\r\n");
}*/

void task_SendLoop( void ) {
    volatile static unsigned char operation;
    
    operation=SEND_DATA;
    while(1) {
        if(to_TWI.ready) {
            switch(operation) {
                case SEND_DATA:
                if(!TWI_Transceiver_Busy()) {
                    TWI_Start_Transceiver_With_Data(to_TWI.string,to_TWI.count);
                    operation=WAIT_TRANSMIT;
                }
                break;
                case WAIT_TRANSMIT:
                while(TWI_Transceiver_Busy()) {
                    mRTOS_DISPATCH;
                }
                if(TWI_statusReg.lastTransOK) {
                    USART_send_ok();
                    } else {
                    USART_send_error(Error_TWI);
                }
                operation=SEND_DATA;
                to_TWI.ready=FALSE;
                break;
            }
        }
        mRTOS_TASK_WAIT(10);
    }
}


void USART_send_error(unsigned char err) {
    USART_SendStr("0");
    USART_PutChar(err);
}

void USART_send_ok()
{
    USART_SendStr("1");
}
