/******************************************************************************
* File Name     : 'mrtos.h'
* Title         : Main Module header of mRTOS
* Author        : Movila V.N. - Copyright (C) 2009
* Created       : 07/01/2009
* Revised       : 18/04/2009
* Version       : 1.09
* Target MCU    : Atmel AVR
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

#ifndef mRTOS_H_INCLUDED
#define mRTOS_H_INCLUDED

enum TaskState{ NOINIT, ACTIVE, SUSPEND, WAIT, SEMAPHORE, STOP }; // ��������� (������) ������

                                                // --- ��������� ��������� ������ ---
struct TaskContext{
       uint16_t TaskAddress;                    // ������ ����� ����� � ������
       uint8_t  TaskCpuState;                   // ��������� �������� SREG ������
};
                                                // --- ��������� ����� �������� ������ (Task Control Block) ---
struct TCB{
           uint8_t Priority,                    // ��������� ������
                   CurrentPriority;             // ������� ��������� ������
struct TaskContext Context;                     // ������� �������� ������
    enum TaskState State;                       // ������� ��������� ������
           uint16_t Delay;                      // ����� �������� � ����� � ��������� ������ Wait
};
                                                // --- ��������� ����� �������� ������� (Event Control Block) ---
struct ECB{
           uint8_t TaskNumber,                  // ����� ������ ������������ �� ��������
                   FlagControlEvent,            // ���� ���������� - ������� ��������� (�������� �������)
                   FlagEvent;                   // ���� ���������� - ������� ���������
};

                                                // --- ������� mRTOS ---

#define mRTOS_APPLICATION_TASKS 3               // ���������� ���������������� ����� � ����������
#define mRTOS_MAX_EVENTS        0               // ���������� ������� � ����������
#define mRTOS_MAX_TASKS    (mRTOS_APPLICATION_TASKS + 1) // ����� ���������� ����� � ���������� (������ Idle �������� ������)

                                                // ��� ������ RTOS ������������ ������ T0 - ������ ��������� �������� - ��������� ���
                                                // Ktcnt0 = 256 - T / (Tclk * Pscl), ��� Tclk = 1 / Fxtal, Pscl - �������� ������������ T0
                                                // T = 1 ����.  ��� Xtal = 8 ���, Tclk = 0.125 �����, Pscl = 64
#define mRTOS_SYSTEM_TIMER_PRESCALER_VALUE 3    // Ktcnt0 = 256 - 1000 / (0.125 * 64) = 131; T = (256 - Ktcnt0) * (Tclk * Pscl) = (256 - 131) * (0.125 * 64) = 1.000 ����.
#define mRTOS_SYSTEM_TIMER_RELOAD_VALUE    131  // �������� ������������ ���������� ������� ��� ����������� ��������� ��������� ���������� ����

#define mRTOS_DISPATCH  mRTOS_DispatchTask(&mRTOS_Tasks[mRTOS_CurrentTask].Context) // ����� ������� ���������� �����
#define mRTOS_TASK_WAIT(d)  mRTOS_WaitTask(d, &mRTOS_Tasks[mRTOS_CurrentTask].Context) // ����� ������� �������� ������� ������ � ��������� Wait �� ����� d ����� (d = 0 .. 65535)
#define mRTOS_TASK_ACTIVE(n)  mRTOS_SetTaskNStatus(n, ACTIVE) // ����� ������� �������� ������ ��� ������� n � ��������� Active
#define mRTOS_TASK_STOP  {mRTOS_SetTaskStatus(STOP); mRTOS_DISPATCH;} // ����� ������� �������� ������� ������ � ��������� Stop � ����������� ������� ���������� �����

                                                // --- ������� mRTOS ---

void mRTOS_Init(void);                          // ������� ������������� OS
uint8_t mRTOS_CreateTask(void (*Task)(void), uint8_t Priority, enum TaskState State); // ������� �������� ������
void mRTOS_WaitTask(uint16_t Delay, struct TaskContext* TaskContextPtr); // ������� �������� ������ � ���������� Wait �� ����� Delay �����
void mRTOS_DispatchTask(struct TaskContext* TaskContextPtr); // ������� ������ ���������� �����
void mRTOS_Scheduler(void);                     // ������� ������������ �����
void mRTOS_SetTaskStatus(enum TaskState Status); // ������� �������� ������� ������ � ��������� Status
uint8_t mRTOS_SetTaskNStatus(uint8_t TaskNumber, enum TaskState Status); // ������� �������� ������ ��� ������� TaskNumber � ��������� Status

                                                // -- ������� ������ � ��������� --

uint8_t mRTOS_InitEvent(uint8_t EventNumber);   // ������� ������������� ������� ��� ������� EventNumber � ������� ������
uint8_t mRTOS_DisableEvent(uint8_t EventNumber); // ������� ������� ������� ��� ������� EventNumber
uint8_t mRTOS_EnableEvent(uint8_t EventNumber); // ������� ���������� ������� ��� ������� EventNumber
uint8_t mRTOS_SetEvent(uint8_t EventNumber);    // ������� ��������� ������� ��� ������� EventNumber
uint8_t mRTOS_SetEventValue(uint8_t EventNumber, uint8_t FlagEventValue); // ������� ��������� �������� ����� ������� FlagEventValue ��� ������� EventNumber
uint8_t mRTOS_GetEvent(uint8_t EventNumber);    // ������� ������ ��������� ������� ��� ������� EventNumber � ����������� ������� �������
uint8_t mRTOS_PopEvent(uint8_t EventNumber);    // ������� ������ ��������� ������� ��� ������� EventNumber ��� ������ �������

                                                // -- ������� ������ � ��������� �������� --

void mRTOS_SetSystemTime(uint32_t Time);        // ������� ��������� ���������� ������� � �����
uint32_t mRTOS_GetSystemTime(void);             // ������� ������ �������� �������� ���������� ������� � �����

extern volatile struct TCB mRTOS_Tasks[mRTOS_MAX_TASKS]; // ������ �������� TCB ���� ����� ���������� (Task Control Block)
extern uint8_t mRTOS_CurrentTask;            // ����� ������� ������

#endif
