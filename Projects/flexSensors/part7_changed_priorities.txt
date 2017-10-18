#include "mbed.h"                    
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <nrk_stats.h>


// Static stack for Task1
NRK_STK Stack1[NRK_APP_STACKSIZE];
// Static stack for Task1
NRK_STK Stack2[NRK_APP_STACKSIZE];
// Static stack for Task1
NRK_STK Stack3[NRK_APP_STACKSIZE];
// Static stack for Task1
NRK_STK Stack4[NRK_APP_STACKSIZE];
// Task1 declaration
nrk_task_type TaskOne;
nrk_task_type TaskTwo;

void Task1(void);
// Function creates taskset
void nrk_create_taskset();

// You do not need to modify this function
struct __FILE { int handle; };

DigitalOut led1(p26);
DigitalOut led2(p27);
DigitalOut led3(p28);
DigitalOut led4(p29);
// Main function
int main(void)
{
	nrk_setup_ports();	
	nrk_init();
	nrk_create_taskset();
	nrk_start();
	return 0;
}

// Task1 body
void Task1()
{
	DigitalOut m(p27);
	while(1)
	{
		m=!m;
		double x=100.22;
		for(int n=0;n<10000;n++){
		x=x*.001;
		}
		x=x+x+5;
		nrk_wait_until_next_period();
	}
}

// Task1 body
void Task2()
{	
	DigitalOut m(p28);
	while(1)
	{
		m=!m;
		double x=100.22;
		double y=100.22;
		x=x*y;
		nrk_wait_until_next_period();
	}
}

// Function creates taskset
void nrk_create_taskset()
{
	
    nrk_task_set_entry_function( &TaskOne, Task1);
    nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
    TaskOne.prio = 2;
    TaskOne.FirstActivation = TRUE;
    TaskOne.Type = BASIC_TASK;
    TaskOne.SchType = PREEMPTIVE;
    TaskOne.period.secs = 0;
    TaskOne.period.nano_secs = 60*NANOS_PER_MS;
    TaskOne.cpu_reserve.secs = 0;
    TaskOne.cpu_reserve.nano_secs = 0;
    TaskOne.offset.secs = 0;
    TaskOne.offset.nano_secs= 0;
    nrk_activate_task (&TaskOne);
	
		nrk_task_set_entry_function( &TaskTwo, Task2);
    nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
    TaskTwo.prio = 3;
    TaskTwo.FirstActivation = TRUE;
    TaskTwo.Type = BASIC_TASK;
    TaskTwo.SchType = PREEMPTIVE;
    TaskTwo.period.secs = 0;
    TaskTwo.period.nano_secs = 30*NANOS_PER_MS;
    TaskTwo.cpu_reserve.secs = 0;
    TaskTwo.cpu_reserve.nano_secs = 0;
    TaskTwo.offset.secs = 0;
    TaskTwo.offset.nano_secs= 0;
    nrk_activate_task (&TaskTwo);

	
}
