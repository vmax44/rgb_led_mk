/******************************************************************************
* File Name     : 'mrtos.c'
* Title         : Main Module of mRTOS
* Author        : Movila V.N. - Copyright (C) 2009
* Created       : 07/01/2009
* Revised       : 18/04/2009
* Version       : 1.09
* Target MCU    : Atmel AVR series
* Editor Tabs   : 4
*
* Notes:          mRTOS - Cooperative priority real time OS for Atmel AVR
*                         series microcontrolers
*                 NOTE:
*
* This code is distributed under the GNU Public License
* which can be found at http://www.gnu.org/licenses/gpl.txt
*
* Revision History:
* When          Who		        Rev     Description of change
* -----------   -----------	    ------- -----------------------
* 18-Apr-2009   Samotkov O.V.   2       Ported to AVR GCC
* 07-Jan-2009   Movila V.N      1       Created the program structure
*******************************************************************************/

#include <avr/io.h>
#include <inttypes.h>
#include <util/atomic.h>
#include "mrtos.h"

volatile struct TCB mRTOS_Tasks[mRTOS_MAX_TASKS]; // ������ �������� TCB ���� ����� ���������� (Task Control Block)
uint8_t mRTOS_CurrentTask;                // ����� ������� ������
static volatile struct ECB mRTOS_Events[mRTOS_MAX_EVENTS]; // ������ �������� ECB ���������� (Event Task Control Block)
static uint8_t mRTOS_InitTasksCounter,    // ������� ���������� ������������������ ����� � ����������
               mRTOS_Scheduler_pri,       // ���������� ������������ �����
               mRTOS_Scheduler_i,
               mRTOS_Scheduler_i_pri,
               mRTOS_FlagStart,           // ���� �������� ������� mRTOS
               mRTOS_FlagSchedulerActive; // ���� ������������ �����
static volatile uint32_t mRTOS_SystemTime; // ������� ������� ������ ������� � ��������� �����

/**
* ������� �������� �� ������ (������������ ������)
* ������� ��������:
* \param TaskContextPtr - ��������� �� ��������� ��������� ������ ��
*                         ������� ����� ������������
*/
static void mRTOS_JmpTask(struct TaskContext* TaskContextPtr) __attribute__((noinline));
static void mRTOS_JmpTask(struct TaskContext* TaskContextPtr){
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
      asm volatile(
        "movw r26, %A0"                 "\n\t"  // ��������� ����� ��������� ��������� ������ � X
        "ld   %A0, X+"                  "\n\t"  // ��������� ��. ���� ������ ����� ����� � ������
        "ld   %B0, X+"                  "\n\t"  // ��������� ��. ���� ������ ����� ����� � ������
        "pop __tmp_reg__"               "\n\t"  // ������ ����� �������� � ������� �����
        "pop __tmp_reg__"               "\n\t"
        "push %A0"                      "\n\t"  // �������� ��. ���� ������ ����� ����� � ������ � ����
        "push %B0"                      "\n\t"  // �������� ��. ���� ������ ����� ����� � ������ � ����
        "ld  __tmp_reg__, X"            "\n\t"  // ��������� ���� ��������� �������� SREG �� ��������� ��������� ������
        "out  __SREG__, __tmp_reg__"    "\n\t"  // ���������������� ������� SREG
        : "+r" ((uint16_t)TaskContextPtr)
        :
        : "r26", "r27", "r0"
      );
   }
}

/**
* ������� ���������� ��������� ������
* ������� ���������:
* \param Task - ��������� �� ������� ������ �������� �������
*                ���� ���������;
* \param TaskContextPtr - ��������� �� ��������� ��������� ������ ����
*                         ����� ������� ��������.
*/
static inline void mRTOS_SaveContext(void (*Task)(void), struct TaskContext* TaskContextPtr){
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
      asm volatile(
        "movw r26, %A1"                 "\n\t"  // ��������� ����� ��������� ��������� ������ � X
        "st   X+, %A0"                  "\n\t"  // ��������� ��. ���� ������ ����� ����� � ������ � ��������� ��������� ������
        "st   X+, %B0"                  "\n\t"  // ��������� ��. ���� ������ ����� ����� � ������ � ��������� ��������� ������
        "in   __tmp_reg__, __SREG__"    "\n\t"  // ��������� ������� SREG
        "st   X, __tmp_reg__"           "\n\t"  // ��������� ������� SREG � ��������� ��������� ������
        :
        : "r" ((uint16_t)Task),
          "r" ((uint16_t)TaskContextPtr)
        : "r26", "r27", "r0"
      );
   }
}

/**
* ������� �������� ������� ������ � ��������� Wait �� �����������
* �����
* ������� ���������:
* \param Delay - �������� ������� � ����� �� ������� ������ �����
*                ���������� � ��������� Wait;
* \param TaskContextPtr - ��������� �� ��������� ��������� ������ �������
*                         ����� ���������� � ��������� Wait.
*/
void mRTOS_WaitTask(uint16_t Delay, struct TaskContext* TaskContextPtr) __attribute__((noinline));
void mRTOS_WaitTask(uint16_t Delay, struct TaskContext* TaskContextPtr){
   mRTOS_Tasks[mRTOS_CurrentTask].State = WAIT; // ���������� ��������� ������� ������ � Wait
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     mRTOS_Tasks[mRTOS_CurrentTask].Delay = Delay; // ���������� ����� ��������� Wait ������� ������
     asm volatile(
       "movw r26, %A0"                 "\n\t" // ��������� ����� ��������� ��������� ������ � X
       "pop  __tmp_reg__"              "\n\t" // ��������� ��. ���� ������ �������� �� �����
       "pop  __zero_reg__"             "\n\t" // ��������� ��. ���� ������ �������� �� �����
       "st   X+, __zero_reg__"         "\n\t" // ��������� ��. ���� ������ �������� � ��������� ��������� ������
       "st   X+, __tmp_reg__"          "\n\t" // ��������� ��. ���� ������ �������� � ��������� ��������� ������
       "in   __tmp_reg__, __SREG__"    "\n\t" // ��������� ������� SREG
       "st   X, __tmp_reg__"           "\n\t" // ��������� ������� SREG � ��������� ��������� ������
       "clr  __zero_reg__"             "\n\t" // ������������ ������� r1 (������ ���� ������ 0)
       :
       : "r" ((uint16_t)TaskContextPtr)
       : "r26", "r27", "r0"
     );
   }
   mRTOS_Scheduler();                         // ������� ������� ������������ �����
}

/**
* ������� ���������� �����
* ������� ��������:
* \param TaskContextPtr - ��������� �� ��������� ��������� ������� ������
*/
void mRTOS_DispatchTask(struct TaskContext* TaskContextPtr) __attribute__((noinline));
void mRTOS_DispatchTask(struct TaskContext* TaskContextPtr){
   mRTOS_Tasks[mRTOS_CurrentTask].State = ACTIVE; // ��������� ������� ������ ���������� � Active
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     asm volatile(
       "movw r26, %A0"                 "\n\t" // ��������� ����� ��������� ��������� ������ � X
       "pop %B0"                       "\n\t" // ��������� ��. ���� ������ �������� �� �����
       "pop %A0"                       "\n\t" // ��������� ��. ���� ������ �������� �� �����
       "st  X+, %A0"                   "\n\t" // ��������� ��. ���� ������ �������� � ��������� ��������� ������
       "st  X+, %B0"                   "\n\t" // ��������� ��. ���� ������ �������� � ��������� ��������� ������
       "in   __tmp_reg__, __SREG__"    "\n\t" // ��������� ������� SREG
       "st   X, __tmp_reg__"           "\n\t" // ��������� ������� SREG � ��������� ��������� ������
       : "+r" ((uint16_t)TaskContextPtr)
       :
       : "r26", "r27", "r0"
     );
   }
   mRTOS_Scheduler();                         // ������� ������� ������������ �����
}

/**
*  ������� ������� ������ (������� �� ���������)
*/
static void mRTOS_Idle(void){
   mRTOS_SetTaskNStatus(0, ACTIVE);     // ������� ������� ��������� ��������� 0-� ������ - Active
   while(1){                            // ���� ������ ������ Idle
      mRTOS_DISPATCH;                   // ������� ����������� �����
   }
}

/**
* ���������� ���������� �� ������������ ������� T0
* (������ ���������� ������� mRTOS)
*/
ISR(TIMER0_OVF_vect){
  uint8_t i;
    TCNT0 = mRTOS_SYSTEM_TIMER_RELOAD_VALUE; // ������������� �������� ������� ��� ����������� ��������� ������� ���������� ����
    mRTOS_SystemTime++;                    // ��������� �������� ���������� �������
    for(i=0; i < mRTOS_InitTasksCounter; i++) // ���� ������������ ������������������ �����
       if(mRTOS_Tasks[i].Delay)            // ���� �������� ������� �������� ������ �� ����, ��
         --mRTOS_Tasks[i].Delay;           // ��������� �������� ������� �������� ������
}

/**
* ������� ������������� mRTOS
*/
void mRTOS_Init(void){
  uint8_t i;
   for(i=0; i < mRTOS_MAX_TASKS; i++){     // ���� ������������� ������� �������� �����
      mRTOS_Tasks[i].Priority = 0;      // �������� ��������� ������
      mRTOS_Tasks[i].CurrentPriority = 0; // �������� ������� ��������� ������
      mRTOS_Tasks[i].State = NOINIT;    // ���������� ��������� ������ - NoInit
      mRTOS_Tasks[i].Delay = 0;         // �������� ���� ��������
   }
   for(i=0; i < mRTOS_MAX_EVENTS; i++){ // ���� ������������� ������� �������� �������
      mRTOS_Events[i].TaskNumber = 0;   // �������� ����� ����������� �� �������� ������
      mRTOS_Events[i].FlagControlEvent = 0; // �������� ���� ���������� �������
      mRTOS_Events[i].FlagEvent = 0;    // �������� ���� �������
   }
   mRTOS_InitTasksCounter = 0;          // �������� ������� ���������� ������������������ ����� � ����������
   mRTOS_FlagStart = 0;                 // �������� ���� �������� ������� mRTOS
   mRTOS_CurrentTask = 0;               // ���������� ����� ������� ������ - 0
   mRTOS_SystemTime = 0;                // �������� ������� ���������� �������
   mRTOS_CreateTask(mRTOS_Idle, 5, ACTIVE); // ������� ������� �������� ������� ������ Idle � ����������� 5 � ���������� Active
   TCNT0 = mRTOS_SYSTEM_TIMER_RELOAD_VALUE; // ������������� ���������� ������� (T0)
   TCCR0 = mRTOS_SYSTEM_TIMER_PRESCALER_VALUE;  // ������ ���������� �������
}

/**
* ������� �������� ������
* ������� ���������:
* \param Task - ��������� �� ������� ������
* \param Priority - ��������� ������ (1...255)
* \param State - ��������� ������ �� ������ ��������
*   ����������:
* \return   1 - ������ ������� �������
* \return   0 - ������, ������ �� �������
*/
uint8_t mRTOS_CreateTask(void (*Task)(void), uint8_t Priority, enum TaskState State){
   if((mRTOS_InitTasksCounter >= mRTOS_MAX_TASKS) // ���� ������� ���������� ������������������ ����� ������ ���������
     ||(Priority == 0))                  // ��� ��������� �� ������, ��
     return 0;                           // ����� � ����� ������
   mRTOS_SaveContext(Task, &mRTOS_Tasks[mRTOS_InitTasksCounter].Context); // ������� ������� ���������� ��������� ������
   mRTOS_Tasks[mRTOS_InitTasksCounter].Priority = Priority; // ���������� ��������� ������
   mRTOS_Tasks[mRTOS_InitTasksCounter].CurrentPriority = Priority; // ���������� ������� ��������� ������
   mRTOS_Tasks[mRTOS_InitTasksCounter++].State = State; // ���������� ��������� ������ � ��������� �������� ������������������ �����
   return 1;                             // ����� � ����� ��������� ����������
}

/**
*  ������� ������������ �����
*/
void mRTOS_Scheduler(void){
  uint16_t temp;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
      asm volatile(
        "pop  __tmp_reg__"              "\n\t"  // �������� ������� ��������� �����
        "pop  __tmp_reg__"              "\n\t"
        :
        :
        : "r0"
      );
    }
    mRTOS_Scheduler_pri = 0;
    mRTOS_FlagSchedulerActive = 0;
    if(!mRTOS_FlagStart){               // ���� ������ ���� � ����������� (��� ������� mRTOS), ��
      mRTOS_FlagStart = 1;              // ������� ���� �������� ������� mRTOS
      mRTOS_JmpTask(&mRTOS_Tasks[mRTOS_CurrentTask].Context); // �������� ������� �������� ���������� ������ ������
    }
    if(--mRTOS_Tasks[mRTOS_CurrentTask].CurrentPriority == 0) // ��������� �������� ���������� ������� ������ � ���� �� ����� ���� ���� �� ��� ����� ������, �� ������������ ��������� ���� �����
      for(mRTOS_Scheduler_i=0; mRTOS_Scheduler_i < mRTOS_InitTasksCounter; mRTOS_Scheduler_i++) // ���� �������������� ���������� ���� �����
         mRTOS_Tasks[mRTOS_Scheduler_i].CurrentPriority = mRTOS_Tasks[mRTOS_Scheduler_i].Priority; // ������������ ��������� ������
    if(mRTOS_Tasks[mRTOS_CurrentTask].State == ACTIVE){ // ���� ��������� ������� ������ Active, ��
      mRTOS_Tasks[mRTOS_CurrentTask].State = SUSPEND; // ��������� ������� ������ � ��������� Suspend
      mRTOS_Scheduler_i_pri = mRTOS_CurrentTask; // ��������� ����� ������� ������
      mRTOS_FlagSchedulerActive = 1;       // ������� ���� ����������
    }
    for(mRTOS_Scheduler_i=0; mRTOS_Scheduler_i < mRTOS_InitTasksCounter; mRTOS_Scheduler_i++){ // ���� ������������ �����
       ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
         temp = mRTOS_Tasks[mRTOS_Scheduler_i].Delay; // ��������� �������� �������� ������� ������
       }
       if((temp == 0) && (mRTOS_Tasks[mRTOS_Scheduler_i].State == WAIT)){ // ���� ��������� ������� ������ Wait � �������� �������, ��
         mRTOS_Scheduler_pri = mRTOS_Tasks[mRTOS_Scheduler_i].CurrentPriority; // ��������� ������� ��������� ���� ������
         mRTOS_CurrentTask = mRTOS_Scheduler_i; // ��������� ����� ������� ������
         break;                         // ����� �� ����� (�� ���� �������� ���������� ���� ������ �� �������� �� ����������)
       }
       if((mRTOS_Tasks[mRTOS_Scheduler_i].CurrentPriority >= mRTOS_Scheduler_pri) && (mRTOS_Tasks[mRTOS_Scheduler_i].State == ACTIVE)){ // ���� ��������� ������� ������ Active � � ��������� ����, ��
         mRTOS_Scheduler_pri = mRTOS_Tasks[mRTOS_Scheduler_i].CurrentPriority; // ��������� ��������� ���� ������ (����� ������ � �������� ������� �����������)
         mRTOS_CurrentTask = mRTOS_Scheduler_i; // ��������� ����� ������� ������
       }
    }
    if(mRTOS_FlagSchedulerActive)          // ���� ���� ���������� ������, ��
      mRTOS_Tasks[mRTOS_Scheduler_i_pri].State = ACTIVE; // ���������� ��������� ������� ������ Active
    mRTOS_JmpTask(&mRTOS_Tasks[mRTOS_CurrentTask].Context); // ������� ������� �������� ���������� ������� ������
}

/**
* ������� ������������� ������� (����������� ������� �� ������� �������,
* ���������� ������� � ����� ����� �������)
* ������� ��������:
* \param EventNumber - ����� ������� ������������ �� ������� �������
* ����������:
* \return 1 - ������� ������� ����������������
* \return 0 - ������, ������� �� ����������������
*/
uint8_t mRTOS_InitEvent(uint8_t EventNumber){
   if(EventNumber >= mRTOS_MAX_EVENTS)  // ���� ����� ������� �� ������, ��
     return 0;                          // ����� � ����� ������
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     mRTOS_Events[EventNumber].TaskNumber = mRTOS_CurrentTask; // ��������� ����� ������� ������ � ��������� �������
     mRTOS_Events[EventNumber].FlagControlEvent = 1; // ������� ���� ���������� �������
     mRTOS_Events[EventNumber].FlagEvent = 0; // �������� ���� �������
   }
   return 1;                            // ����� � ����� ��������� ����������
}

/**
* ������� ������� �������
* ������� ��������:
* \param EventNumber - ����� �������
* ����������:
* \return 1 - ������� ������� �����������
* \return 0 - ������, ������� �� �����������
*/
uint8_t mRTOS_DisableEvent(uint8_t EventNumber){
   if(EventNumber >= mRTOS_MAX_EVENTS)     // ���� ����� ������� �� ������, ��
     return 0;                          // ����� � ����� ������
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     mRTOS_Events[EventNumber].FlagControlEvent = 0; // �������� ���� ���������� ������� (��������� �������)
   }
   return 1;                            // ����� � ����� ��������� ����������
}

/**
* ������� ���������� �������
* ������� ��������:
* \param EventNumber - ����� �������
* ����������:
* \return 1 - ������� ������� �����������
* \return 0 - ������, ������� �� �����������
*/
uint8_t mRTOS_EnableEvent(uint8_t EventNumber){
   if(EventNumber >= mRTOS_MAX_EVENTS)  // ���� ����� ������� �� ������, ��
     return 0;                          // ����� � ����� ������
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     mRTOS_Events[EventNumber].FlagControlEvent = 1; // ������� ���� ���������� ������� (��������� �������)
   }
   return 1;                            // ����� � ����� ��������� ����������
}

/**
* ������� ��������� ����� ������� ���� ��� ���������
* ������� ��������:
* \param  EventNumber - ����� �������
* ����������:
* \return 1 - ������� ������� �����������
* \return 0 - ������, ������� �� �����������
*/
uint8_t mRTOS_SetEvent(uint8_t EventNumber){
   if(EventNumber >= mRTOS_MAX_EVENTS)  // ���� ����� ������� �� ������, ��
     return 0;                          // ����� � ����� ������
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     if(mRTOS_Events[EventNumber].FlagControlEvent) // ���� ���� ���������� ������� ������, ��
       mRTOS_Events[EventNumber].FlagEvent++; // ��������� ����� �������
   }
   return 1;                            // ����� � ����� ��������� ����������
}

/**
* ������� ��������� �������� ����� ������� ���� ��� ���������
* ������� ���������:
* \param EventNumber - ����� �������
* \param FlagEventValue - ��������������� �������� ����� �������
* ����������:
* \return 1 - ���� ������� ������� ����������
* \return 0 - ������, ���� ������� �� ����������
*/
uint8_t mRTOS_SetEventValue(uint8_t EventNumber, uint8_t FlagEventValue){
   if(EventNumber >= mRTOS_MAX_EVENTS)  // ���� ����� ������� �� ������, ��
     return 0;                          // ����� � ����� ������
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     if(mRTOS_Events[EventNumber].FlagControlEvent) // ���� ���� ���������� ������� ������, ��
       mRTOS_Events[EventNumber].FlagEvent = FlagEventValue; // ���������������� �������� ����� �������
   }
   return 1;                            // ����� � ����� ��������� ����������
}

/**
* ������� ������ ����� ������� � ����������� ������� �����
* �������
* ������� ���������:
* \param EventNumber - ����� �������
* ����������:
* \return �������� ����� �������
* \return 0 - ��� ������ ������ �������
*/
uint8_t mRTOS_GetEvent(uint8_t EventNumber){
   uint8_t temp=0;
   if(EventNumber >= mRTOS_MAX_EVENTS)  // ���� ����� ������� �� ������, ��
     return temp;                       // ����� � ��������� 0
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     if(mRTOS_Events[EventNumber].FlagControlEvent){ // ���� ������� ���������, ��
       temp = mRTOS_Events[EventNumber].FlagEvent; // ��������� �������� ����� �������
       mRTOS_Events[EventNumber].FlagEvent = 0; // �������� ���� �������
     }
   }
   return temp;                         // ����� � ��������� ����� �������
}

/**
* ������� ������ ����� ������� ��� ������ ����� �������
* ������� ��������:
* \param EventNumber - ����� �������
* ����������:
* \return �������� ����� �������
* \return 0 - ��� ������ ������ �������
*/
uint8_t mRTOS_PopEvent(uint8_t EventNumber){
  uint8_t temp=0;
   if(EventNumber >= mRTOS_MAX_EVENTS)  // ���� ����� ������� �� ������, ��
     return temp;                       // ����� � ��������� 0
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     if(mRTOS_Events[EventNumber].FlagControlEvent) // ���� ������� ���������, ��
       temp = mRTOS_Events[EventNumber].FlagEvent; // ��������� �������� ����� �������
   }
   return temp;                         // ����� � ��������� ����� �������
}

/**
* ������� ��������� ��������� ������� ������
* ������� ��������:
* \param Status - ��������������� ��������� ������� ������
*/
void mRTOS_SetTaskStatus(enum TaskState Status){
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     mRTOS_Tasks[mRTOS_CurrentTask].State = Status; // ���������� ��������� ������� ������
   }
}

/**
* ������� ��������� ��������� ������ � �������� �������
* ������� ���������:
* \param TaskNumber - ����� ������
* \param Status - ��������������� ���������
* ����������:
* \return 1 - ��������� ������ ������� �����������
* \return 0 - ������, ��������� ������ �� �����������
*/
uint8_t mRTOS_SetTaskNStatus(uint8_t TaskNumber, enum TaskState Status){
   if(TaskNumber >= mRTOS_MAX_TASKS)    // ���� ����� ������ ��������, ��
     return 0;                          // ����� � ����� ������
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     if(mRTOS_Tasks[TaskNumber].State == STOP) // ���� �������� ������ �����������, ��
       mRTOS_Tasks[TaskNumber].State = Status; // ���������� ��������� ���� ������
   }
   return 1;                            // ����� � ����� ��������� ����������
}

/**
* ������� ��������� �������� ���������� ������� � �����
* ������� ��������:
* \param Time - ��������������� �������� ���������� ������� � �����
*/
void mRTOS_SetSystemTime(uint32_t Time){
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     mRTOS_SystemTime = Time;           // ��������� �������� ���������� �������
   }
}

/**
* ������� ������ �������� �������� ���������� ������� � �����
* ����������:
* \return ������� �������� ���������� ������� � �����
*/
uint32_t mRTOS_GetSystemTime(void){
  uint32_t temp;
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
     temp = mRTOS_SystemTime;           // ���������� �������� ���������� �������
   }
   return temp;
}
