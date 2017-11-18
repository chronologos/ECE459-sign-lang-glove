#include "mbed.h"                    
#include <stdio.h>
#include <stdint.h>
#include "flex_sensor.h"
#include "LSM9DS0/LSM9DS0.h"  
#define FLEX_ENABLE
#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW
AnalogIn adcin1(p15), adcin2(p16), adcin3(p17), adcin4(p18), adcin5(p19), adcin6(p20);
InterruptIn toggleButton(p14);
DigitalOut led1(LED1), led2(LED2), led3(LED3);
DigitalOut ssEn(p13);
DigitalOut red(p5), orange(p6), yellow(p7), green(p8), blue(p9), purple(p10), gray(p11), white(p12);
Serial pc(USBTX,USBRX); //tx, rx
Serial bt(p28, p27);
Sensing::FlexSensorReader flex_reader;
Sensing::SensorReading sensor_reading();
std::vector<string> consensus_queue;
const int CONSENSUS_N = 10;
LSM9DS0 dof(p9, p10,LSM9DS0_G, LSM9DS0_XM);
/* Check if last CONSENSUS_N readings constitute a consensus on what character was 
 * read. IMPT: Clears vector if no consensus. 
 */ 
 
void emptyQueue(){
	while (consensus_queue.size() > 0){
		consensus_queue.pop_back();
	}
}

bool hasConsensus(){
	if (consensus_queue.size() < CONSENSUS_N){
		return false;
	}
	string candidate = consensus_queue[0];
	for (int i =1; i<consensus_queue.size(); ++i){
		if (consensus_queue[i] != candidate){
			emptyQueue();
			return false;
		}
	}
	return true;
}

/* Return consensus value and clears vector, only safe to call if hasConsensus! */
std::string getConsensus(){
		string consensus = consensus_queue[0];
		emptyQueue();
		return consensus;
}

// Main function
int main(void) {
	pc.baud(115200);
	bt.baud(9600);
	led1 = 1;
	pc.printf("Hello world\n");
	uint16_t status = dof.begin();
	// Make sure communication is working
  pc.printf("LSM9DS0 WHO_AM_I's returned: 0x");
  pc.printf("%x\n",status);
  pc.printf("Should be 0x49D4\n");
  pc.printf("\n");
	#ifdef FLEX_ENABLE
	ssEn=1;
	Sensing::FlexSensorReader flexReader;
	flexReader.ADCSetup();
	Sensing::SensorReading sensorReading;
	while(1) {
		wait_ms(1000);
		flexReader.Poll(&sensorReading);
		string candidate = flexReader.Convert(&sensorReading);
		string result = "";
		consensus_queue.push_back(candidate);
		if (hasConsensus()){
			result = getConsensus();
			pc.printf("%s\n", result.c_str());
		}
	}

	#endif
	return 0;
}
