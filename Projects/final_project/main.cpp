#include "mbed.h"                    
#include <stdio.h>
#include <stdint.h>
#include "flex_sensor.h"
#include "LSM9DS0/LSM9DS0.h" 
#include "consensus.h"
#include "imu_fsm.h"

#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW

const float Q_X_AXIS_THRESHOLD = -0.8;
const float Y_AXIS_MVMT_THRESHOLD = 0.4;
// const float G_Y_AXIS_THRESHOLD = 0.8;
const float J_GX_THRESHOLD = 150.0;
const int TIMEOUT_MS = 100;
AnalogIn adcin1(p15), adcin2(p16), adcin3(p17), adcin4(p18), adcin5(p19), adcin6(p20);
DigitalOut led1(LED1), led2(LED2), led3(LED3);
Serial pc(USBTX,USBRX); //tx, rx
Serial bt(p28, p27);
Sensing::FlexSensorReader flex_reader;
Sensing::SensorReading sensor_reading();
LSM9DS0 dof(p9, p10,LSM9DS0_G, LSM9DS0_XM);
Timer timer;

// Main function
int main(void) {
	ImuFsm::state_t cur_state = ImuFsm::STATE_DEFAULT;
	ImuFsm::instance_data_t data;
	pc.baud(115200);
	bt.baud(9600);
	
	// Initialize IMU
	uint16_t status = dof.begin();
  pc.printf("LSM9DS0 WHO_AM_I's returned: 0x");
  pc.printf("%x\n",status);
  pc.printf("Should be 0x49D4\n\n");

	Sensing::FlexSensorReader flexReader;
	flexReader.ADCSetup();
	Sensing::SensorReading sensorReading;
	while(1) {
		wait_ms(100);
		flexReader.Poll(&sensorReading);
		string candidate = flexReader.Convert(&sensorReading);
		string result = "";
		Consensus::consensus_queue.push_back(candidate);
		if (Consensus::hasConsensus()){
			result = Consensus::getConsensus()[0];
			data.event = ImuFsm::GESTURE_IN;
			data.key = result;
			cur_state = ImuFsm::run_state(cur_state, &data);
			if (data.event == ImuFsm::KEY_OUT){
				pc.printf("result (consensus): %s\n", result.c_str());
				} 
			else if (cur_state == ImuFsm::STATE_WAIT_AY_Q 
					  || cur_state == ImuFsm::STATE_WAIT_AY_G
					  || cur_state == ImuFsm::STATE_WAIT_AX
			      || cur_state == ImuFsm::STATE_WAIT_GZ) {
				timer.start();
				while (1){
					dof.readAccel();
					dof.readGyro();
					if (timer.read_ms() > TIMEOUT_MS){
						data.event = ImuFsm::TIMEOUT_IN;
						timer.stop();
						timer.reset();
						cur_state = ImuFsm::run_state(cur_state, &data);
						break;
					}
					else if(dof.calcAccel(dof.ay)-dof.abias[1]>Y_AXIS_MVMT_THRESHOLD){
						data.event = ImuFsm::MOTION_IN;
						data.key = 'y';
						cur_state = ImuFsm::run_state(cur_state, &data);
						pc.printf("y axis motion detected.\n");
						pc.printf("result (consensus + imu): %s\n", data.key.c_str());
						break;
					}
					else if(dof.calcGyro(dof.gx) - dof.gbias[0]>J_GX_THRESHOLD){ //TODO(iantay) change to gz
						data.event = ImuFsm::MOTION_IN;
						data.key = 'z';
						cur_state = ImuFsm::run_state(cur_state, &data);
						pc.printf("j motion detected.\n");
						pc.printf("result (consensus + imu): %s\n", data.key.c_str());
						break;
					}
					else if(dof.calcGyro(dof.ax) - dof.abias[0]<Q_X_AXIS_THRESHOLD){
						data.event = ImuFsm::MOTION_IN;
						data.key = 'x';
						cur_state = ImuFsm::run_state(cur_state, &data);
						pc.printf("q orientation detected.\n");
						pc.printf("result (consensus + imu): %s\n", data.key.c_str());
						break;
					}
		//		pc.printf("%2f",dof.calcGyro(dof.gx) - dof.gbias[0]);
		//    pc.printf(", ");
		//    pc.printf("%2f",dof.calcGyro(dof.gy) - dof.gbias[1]);
		//    pc.printf(", ");
		//    pc.printf("%2f\n",dof.calcGyro(dof.gz) - dof.gbias[2]);
		//		pc.printf("%2f",dof.calcAccel(dof.ax) - dof.abias[0]);
		//    pc.printf(", ");
		//    pc.printf("%2f",dof.calcAccel(dof.ay) - dof.abias[1]);
		//    pc.printf(", ");
		//    pc.printf("%2f\n",dof.calcAccel(dof.az) - dof.abias[2]);
				}
			}

		}	
	}
	return 0;
}

