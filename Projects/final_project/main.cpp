#include "mbed.h"                    
#include <stdio.h>
#include <stdint.h>
#include "flex_sensor.h"
#include "LSM9DS0/LSM9DS0.h" 
#include "consensus.h"
#include "imu_fsm.h"

#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW

const float X_AXIS_THRESHOLD = 0.8;
const float NX_AXIS_THRESHOLD = -0.8;
const float Y_AXIS_THRESHOLD = 0.8;
const float NY_AXIS_THRESHOLD = -0.8;
const float Z_AXIS_THRESHOLD = 0.80;
const float NZ_AXIS_THRESHOLD = -0.80;
// const float G_Y_AXIS_THRESHOLD = 0.8;
const float J_GZ_THRESHOLD = 150.0;
const int TIMEOUT_MS = 350;
AnalogIn adcin1(p15), adcin2(p16), adcin3(p17), adcin4(p18), adcin5(p19), adcin6(p20);
DigitalOut led1(LED1), led2(LED2), led3(LED3);
Serial pc(USBTX,USBRX); //tx, rx
Serial bt(p28, p27);
LSM9DS0 dof(p9, p10,LSM9DS0_G, LSM9DS0_XM);
Timer timer;
string prev_key = "";

typedef enum ModeT {
	NORMAL,
	TEST,
	IMUTEST
} ModeT;

ModeT mode = NORMAL;

void print_key_out(ImuFsm::instance_data_t *data){
	prev_key = data->key;
	pc.printf("result: %s\n", data->key.c_str());
}

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
	// Outer loop: poll and look for consensus.
	if (mode == NORMAL){
		while (1){
			wait_ms(50);
			int res = flexReader.Poll(&sensorReading);
			while (res != 0) {
				printf("Polling error!\n");
				res = flexReader.Poll(&sensorReading);
			}
			string candidate = flexReader.Convert(&sensorReading);
			string result = "";
			Consensus::consensus_queue.push_back(candidate);
			// Update FSM state and check for outputs if any
			if (Consensus::hasConsensusOrClear()){
				// Inputs to FSM go in through ImuFsm::instance_data_t data;
				// FSM actuates/outputs on transition arrows (i.e. it is a Mealy machine)
				// Outputs go out thru ImuFsm::instance_data_t data as well.
				result = Consensus::getConsensusAndClear();
				data.event = ImuFsm::GESTURE_IN;
				data.key = result;
				cur_state = ImuFsm::run_state(cur_state, &data);
				sensorReading.printify();
				//cur_state = ImuFsm::STATE_WAIT_AX ; TODO(iantay) remove these debug stmts
				//data.event = ImuFsm::GESTURE_IN;
				
				if (data.event == ImuFsm::KEY_OUT){
					print_key_out(&data);
				}
				else if (cur_state == ImuFsm::STATE_WAIT_MNSXT) {
					timer.start();
					while (1){
						dof.readAccel();
						dof.readGyro();
						// We exit the IMU polling loop if 1) timeout OR 2) particular motion detected.
						if (timer.read_ms() > TIMEOUT_MS){
							data.event = ImuFsm::TIMEOUT_IN;
							timer.stop();
							timer.reset();
							cur_state = ImuFsm::run_state(cur_state, &data);
							printf("timeout\n");
							break;
						}
						else if(dof.calcAccel(dof.ay)-dof.abias[1]>Y_AXIS_THRESHOLD/2){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'y';
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("half y axis motion detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
						  timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcAccel(dof.ay)-dof.abias[1]>NY_AXIS_THRESHOLD/2){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'Y';
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("half -y axis motion detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
						  timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcAccel(dof.az)-dof.abias[2]<NZ_AXIS_THRESHOLD/2){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'z';
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("half -z axis motion detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
						  timer.stop();
							timer.reset();
							break;
						}
					}
				}
				// We turn on IMU if we are in a state that requires IMU data.
				else if (cur_state == ImuFsm::STATE_WAIT_IJ
							|| cur_state == ImuFsm::STATE_WAIT_GDZQ
			       	|| cur_state == ImuFsm::STATE_WAIT_5
				      || cur_state == ImuFsm::STATE_WAIT_A) {
					timer.start();
					while (1){
						dof.readAccel();
						dof.readGyro();
						// We exit the IMU polling loop if 1) timeout OR 2) particular motion detected.
						if (timer.read_ms() > TIMEOUT_MS){
							data.event = ImuFsm::TIMEOUT_IN;
							timer.stop();
							timer.reset();
							cur_state = ImuFsm::run_state(cur_state, &data);
							printf("timeout\n");
							break;
						}
						else if(dof.calcAccel(dof.ay)-dof.abias[1]>Y_AXIS_THRESHOLD){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'y';
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("y axis motion detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
						  timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcAccel(dof.ay)-dof.abias[1]<NY_AXIS_THRESHOLD){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'Y';
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("-y axis motion detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
						  timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcGyro(dof.gx) - dof.gbias[2]>J_GZ_THRESHOLD){ //TODO(iantay) change to gz
							data.event = ImuFsm::MOTION_IN;
							data.key = 'c';
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("j motion detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
						  timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcAccel(dof.ax) - dof.abias[0]<NX_AXIS_THRESHOLD){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'X'; // TODO(iantay) change to enum
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("-x accel detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
							timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcAccel(dof.ax) - dof.abias[0]>X_AXIS_THRESHOLD){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'x'; // TODO(iantay) change to enum
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("x accel detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
							timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcAccel(dof.az) - dof.abias[2]>Z_AXIS_THRESHOLD){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'z'; // TODO(iantay) change to enum
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("z accel detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
							timer.stop();
							timer.reset();
							break;
						}
						else if(dof.calcAccel(dof.az) - dof.abias[2]<NZ_AXIS_THRESHOLD){
							data.event = ImuFsm::MOTION_IN;
							data.key = 'Z'; // TODO(iantay) change to enum
							cur_state = ImuFsm::run_state(cur_state, &data);
							pc.printf("-z accel detected.\n");
							if (data.event == ImuFsm::KEY_OUT){
								print_key_out(&data);
							}
							timer.stop();
							timer.reset();
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
	} else if (mode == TEST) {
			Sensing::SensorReading sensorReading1;
			Sensing::SensorReading sensorReading2;
			Sensing::SensorReading sensorReading3;
			Sensing::SensorReading sensorReading4;
			Sensing::SensorReading sensorReading5;
			Sensing::SensorReading sensorReadingAverage;
			while(1){
				wait_ms(200);
				flexReader.Poll(&sensorReading1);
				wait_ms(200);
				flexReader.Poll(&sensorReading2);
				wait_ms(200);
				flexReader.Poll(&sensorReading3);
				wait_ms(200);
				flexReader.Poll(&sensorReading4);
				wait_ms(200);
				flexReader.Poll(&sensorReading5);
				Sensing::averageReadings(&sensorReading1, &sensorReading2,
					&sensorReading3, &sensorReading4, &sensorReading5, &sensorReadingAverage);
				sensorReadingAverage.printify();
			}
		} else if (mode == IMUTEST){
			while(1){
				wait_ms(1000);
				dof.readAccel();
				dof.readGyro();
				pc.printf("%2f",dof.calcGyro(dof.gx) - dof.gbias[0]);
				pc.printf(", ");
				pc.printf("%2f",dof.calcGyro(dof.gy) - dof.gbias[1]);
				pc.printf(", ");
				pc.printf("%2f\n",dof.calcGyro(dof.gz) - dof.gbias[2]);
				pc.printf("%2f",dof.calcAccel(dof.ax) - dof.abias[0]);
				pc.printf(", ");
				pc.printf("%2f",dof.calcAccel(dof.ay) - dof.abias[1]);
				pc.printf(", ");
				pc.printf("%2f\n",dof.calcAccel(dof.az) - dof.abias[2]);
			}
		}
	return 0;
}

