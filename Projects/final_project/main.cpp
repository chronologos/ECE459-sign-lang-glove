#include "mbed.h"                    
#include <stdio.h>
#include <stdint.h>
#include "flex_sensor.h"
#include "LSM9DS0/LSM9DS0.h" 
#include "consensus.h"

#define FLEX_ENABLE
#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW

const float Q_X_AXIS_THRESHOLD = -0.8;
const float Y_AXIS_MVMT_THRESHOLD = 0.4;
const float G_Y_AXIS_THRESHOLD = 0.8;
const float J_GX_THRESHOLD = 150.0;
AnalogIn adcin1(p15), adcin2(p16), adcin3(p17), adcin4(p18), adcin5(p19), adcin6(p20);
DigitalOut led1(LED1), led2(LED2), led3(LED3);
Serial pc(USBTX,USBRX); //tx, rx
Serial bt(p28, p27);
Sensing::FlexSensorReader flex_reader;
Sensing::SensorReading sensor_reading();
LSM9DS0 dof(p9, p10,LSM9DS0_G, LSM9DS0_XM);

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
	Sensing::FlexSensorReader flexReader;
	flexReader.ADCSetup();
	Sensing::SensorReading sensorReading;
	while(1) {
		dof.readAccel();
    dof.readGyro();
//		pc.printf("%2f",dof.calcGyro(dof.gx) - dof.gbias[0]);
//    pc.printf(", ");
//    pc.printf("%2f",dof.calcGyro(dof.gy) - dof.gbias[1]);
//    pc.printf(", ");
//    pc.printf("%2f\n",dof.calcGyro(dof.gz) - dof.gbias[2]);
		pc.printf("%2f",dof.calcAccel(dof.ax) - dof.abias[0]);
    pc.printf(", ");
    pc.printf("%2f",dof.calcAccel(dof.ay) - dof.abias[1]);
    pc.printf(", ");
    pc.printf("%2f\n",dof.calcAccel(dof.az) - dof.abias[2]);
		if(dof.calcAccel(dof.ay)-dof.abias[1]>G_Y_AXIS_THRESHOLD){
			pc.printf("g orientation detected.\n");
		}
		else if(dof.calcAccel(dof.ay)-dof.abias[1]>Y_AXIS_MVMT_THRESHOLD){
			pc.printf("y axis motion detected.\n");
		}
		else if(dof.calcGyro(dof.gx) - dof.gbias[0]>J_GX_THRESHOLD){
			pc.printf("j motion detected.\n");
		}
		else if(dof.calcGyro(dof.ax) - dof.abias[0]<Q_X_AXIS_THRESHOLD){
			pc.printf("q orientation detected.\n");
		}
		wait_ms(100);
		flexReader.Poll(&sensorReading);
		string candidate = flexReader.Convert(&sensorReading);
		string result = "";
		Consensus::consensus_queue.push_back(candidate);
		if (Consensus::hasConsensus()){
			result = Consensus::getConsensus();
			pc.printf("%s\n", result.c_str());
		}
	}

	#endif
	return 0;
}
