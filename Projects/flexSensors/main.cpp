#include "mbed.h"                    
//#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <stdint.h>
#include "seven_segment.h"
//#include <lpc17xx.h>

//#include <nrk_error.h>
//#include <nrk_timer.h>
//#include <nrk_stack_check.h>
//#include <nrk_stats.h>


// Static stack for Task1
//NRK_STK Stack1[NRK_APP_STACKSIZE];
//// Task1 declaration
//nrk_task_type TaskOne;

//void Task1(void);
//// Function creates taskset
//void nrk_create_taskset();

// You do not need to modify this function
//struct __FILE { int handle; };

AnalogIn adcin(p15);
DigitalOut ssEn(p20);
//DigitalOut ssOut(p5,p6,p7,p8,p9,p10,p11,p12);
DigitalOut red(p5), orange(p6), yellow(p7), green(p8), blue(p9), purple(p10), gray(p11), white(p12);
Serial pc(USBTX,USBRX); //tx, rx

DisplayState ssOut;
int max = 2500;
int min = 1250;
int divSize = (max-min)/4;

void update(DisplayState* state) {
    red = state->g;
    orange = state->f;
    yellow = state->a;
    green = state->b;
    blue = state->e;
    purple = state->d;
    gray = state->c;
    white = state->dp;
}

void config()
{	
	pc.baud(115200);
	LPC_ADC->ADCR|=0x00000001; //set to run on channel 0
	LPC_ADC->ADCR|=0x00000900; //set clock divisor to have divide by x+1 (from 100MHz)
	LPC_ADC->ADCR|=0x00200000; //set to power the ADC
	LPC_SC->PCONP |= (1<<12);
	LPC_PINCON->PINSEL1|=0x01<<14;
	ssEn=1;
}

void startConversion()
{
	LPC_ADC->ADCR|=0x01000000; //set to startConversion
}

void endConversion()
{
	LPC_ADC->ADCR&=0xfeffffff; //set to end conversion
}

uint16_t extractData()
{
	return (LPC_ADC->ADGDR>>4)&0x00000fff;
}


void display(uint16_t input)
{
	if (input>(max-divSize)){
		set('3', &ssOut);
		update(&ssOut);
		return;
	}
	if (input>(max-divSize*2)){
		set('2', &ssOut);
		update(&ssOut);
		return;
	}
	if (input>(min+divSize)){
		set('1', &ssOut);
		update(&ssOut);
		return;
	}
	else{
		set('0', &ssOut);
		update(&ssOut);
		return;
	}
}


// Main function
int main(void)
{
	uint16_t output =0;
	config();
	
	while(1)
	{
		pc.printf("Hello world\n");
		
		startConversion();
		while((LPC_ADC->ADGDR&31u)==0);
		wait_ms(1);
		endConversion();
		display(extractData());
		pc.printf("%d\n", extractData());
		//LPC_ADC->ADGDR &= 0x7fffffff;

	}
	return 0;
}

// Function creates taskset
//void nrk_create_taskset()
//{
//	
//    nrk_task_set_entry_function( &TaskOne, Task1);
//    nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
//    TaskOne.prio = 2;
//    TaskOne.FirstActivation = TRUE;
//    TaskOne.Type = BASIC_TASK;
//    TaskOne.SchType = PREEMPTIVE;
//    TaskOne.period.secs = 0;
//    TaskOne.period.nano_secs = 60*NANOS_PER_MS;
//    TaskOne.cpu_reserve.secs = 0;
//    TaskOne.cpu_reserve.nano_secs = 0;
//    TaskOne.offset.secs = 0;
//    TaskOne.offset.nano_secs= 0;
//    nrk_activate_task (&TaskOne);
//}
