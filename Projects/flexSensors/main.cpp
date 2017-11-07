#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <stdint.h>
#include "flex_sensor.h"
#include "mbed.h"                    
#include "seven_segment.h"

//#define FLEX_ENABLE
#define IMU_ENABLE

AnalogIn adcin1(p15), adcin2(p16), adcin3(p17), adcin4(p18), adcin5(p19), adcin6(p20);
InterruptIn toggleButton(p14);
DigitalOut led1(LED1), led2(LED2), led3(LED3);
DigitalOut ssEn(p13);
DigitalOut red(p5), orange(p6), yellow(p7), green(p8), blue(p9), purple(p10), gray(p11), white(p12);
Serial pc(USBTX,USBRX);
Serial bt(p28, p27);
Sensing::FlexSensorReader flex_reader;
Sensing::SensorReading sensor_reading();

DisplayState ssOut;
int max_scale = 2500;
int min_scale = 1250;
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

int main(void) {
	pc.baud(115200);
	bt.baud(9600);
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
			set(res.c_str()[0], &ssOut);
			update(&ssOut);
			wait_ms(100);
			bt.printf("%c",res.c_str()[0]);
		}
	}
	#endif //FLEX_ENABLE
	#ifdef IMU_ENABLE
	#endif
	return 0;
}
