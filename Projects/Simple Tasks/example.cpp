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
// Task1 declaration
nrk_task_type TaskOne;
void Task1(void);
// Function creates taskset
void nrk_create_taskset();

// You do not need to modify this function
struct __FILE { int handle; };

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
	while(1)
	{
		nrk_led_toggle(ORANGE_LED);
		nrk_spin_wait_us(1000);
		nrk_led_toggle(ORANGE_LED);
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
    TaskOne.period.nano_secs = 100*NANOS_PER_MS;
    TaskOne.cpu_reserve.secs = 0;
    TaskOne.cpu_reserve.nano_secs = 0;
    TaskOne.offset.secs = 0;
    TaskOne.offset.nano_secs= 0;
    nrk_activate_task (&TaskOne);
	
}
