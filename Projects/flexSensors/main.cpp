#include "mbed.h"                    
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <stdint.h>
#include "seven_segment.h"
#include "flex_sensor.h"

#define FLEX_ENABLE

AnalogIn adcin1(p15), adcin2(p16), adcin3(p17), adcin4(p18), adcin5(p19), adcin6(p20);
InterruptIn toggleButton(p14);
DigitalOut led1(LED1), led2(LED2), led3(LED3);
DigitalOut ssEn(p13);
DigitalOut red(p5), orange(p6), yellow(p7), green(p8), blue(p9), purple(p10), gray(p11), white(p12);
Serial pc(USBTX,USBRX); //tx, rx
Serial bt(p28, p27);
Sensing::FlexSensorReader flex_reader;
Sensing::SensorReading sensor_reading();

DisplayState ssOut;
int max_scale = 2500;
int min_scale = 1250;
int displaying = 1; // TODO(iantay) shows 1 to 6 // TODO(iantay) remove
int divSize = (max_scale-min_scale)/4;

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

void config() // TODO(iantay) remove
{	
	pc.baud(115200);
	// Use global displaying var to choose adc channel
	if (displaying == 1){
		LPC_ADC->ADCR=1u; //set to run on channel 0
	} else if (displaying == 2) {
		LPC_ADC->ADCR=2u;
	} else if (displaying == 3) {
		LPC_ADC->ADCR=4u;
	} else if (displaying == 4) {
		LPC_ADC->ADCR=8u;
	} else if (displaying == 5) {
		LPC_ADC->ADCR=16u;
	} else if (displaying == 6) {
		LPC_ADC->ADCR=32u;
	}
	LPC_ADC->ADCR|=0x00000900; //set clock divisor to have divide by x+1 (from 100MHz)
	LPC_ADC->ADCR|=0x00200000; //set to power the ADC
	LPC_SC->PCONP |= (1<<12);
	LPC_PINCON->PINSEL1|=0x01<<14;
	ssEn=1;
}

void startConversion() {
	LPC_ADC->ADCR|=0x01000000; //set to startConversion
}

void endConversion() {
	LPC_ADC->ADCR&=0xfeffffff; //set to end conversion
}

uint16_t extractData() {
	return (LPC_ADC->ADGDR>>4)&0x00000fff;
}


void display(uint16_t input) {
	if (input>(max_scale-divSize)){
		set('3', &ssOut);
	}
	if (input>(max_scale-divSize*2)){
		set('2', &ssOut);
	}
	if (input>(min_scale+divSize)){
		set('1', &ssOut);
	}
	else{
		set('0', &ssOut);
	}
	update(&ssOut);
	led1 = (displaying & 1u);
	led2 = (displaying & 2u);
	led3 = (displaying & 4u);
}

void toggleHandler(){ // TODO(iantay) remove
	if (displaying > 5){
		displaying = 1;
	} else {
		displaying += 1;
	}
	config(); // refresh the adc channel
}

int old(void){ // TODO(iantay) remove
	config();
	toggleButton.rise(&toggleHandler);
	while(1) {
		wait_ms(1);
		startConversion();
		while((LPC_ADC->ADGDR&31u)==0);
		wait_ms(1);
		endConversion();
		display(extractData());
		pc.printf("%d\n", extractData());
	}
}
// Main function
int main(void) {
	pc.baud(115200);
	bt.baud(9600);
	led1 = 1;
	pc.printf("Hello world\n");
	
	#ifdef FLEX_ENABLE
	ssEn=1;
	Sensing::FlexSensorReader flexReader;
	flexReader.ADCSetup();
	Sensing::SensorReading sensorReading;
	while(1) {
		wait_ms(100);
		flexReader.Poll(&sensorReading);
		string res = flexReader.Convert(&sensorReading);
		pc.printf("%s\n", res.c_str());
		if (res.size() != 0){
			//set(res.c_str()[0], &ssOut);
			//update(&ssOut);
			wait_ms(100);
			pc.printf("%c",res.c_str()[0]);
			bt.printf("%c",res.c_str()[0]);
			
			
		}
	}

	#endif
	return 0;
}
