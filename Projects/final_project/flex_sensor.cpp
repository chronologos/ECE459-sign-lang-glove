#include "flex_sensor.h"
#include "mbed.h"
#include <vector>
#include <iostream>

const int ADC_MAX = 3900;
const int ADC_MIN = 100;

bool DEBUG = false;
namespace Sensing {

	void averageReadings(
		SensorReading* srp1, SensorReading* srp2,
		SensorReading* srp3, SensorReading* srp4,
		SensorReading* srp5, SensorReading* avg){
			avg->thumb = (srp1->thumb + srp2->thumb + srp3->thumb + srp4->thumb + srp5->thumb) / 5;
			avg->finger1 = (srp1->finger1 + srp2->finger1 + srp3->finger1 + srp4->finger1 + srp5->finger1) / 5;
			avg->finger2 = (srp1->finger2 + srp2->finger2 + srp3->finger2 + srp4->finger2 + srp5->finger2) / 5;
			avg->finger3 = (srp1->finger3 + srp2->finger3 + srp3->finger3 + srp4->finger3 + srp5->finger3) / 5;
			avg->finger4 = (srp1->finger4 + srp2->finger4 + srp3->finger4 + srp4->finger4 + srp5->finger4) / 5;
			avg->palm = (srp1->palm + srp2->palm + srp3->palm + srp4->palm + srp5->palm) / 5;
		}

		FlexSensorReader::FlexSensorReader() {
			keys.reserve(40);
			//keys.push_back(SensorReading(1563, 2297, 1272, 631, 1206, 2007, "1", 1)); g
			//keys.push_back(SensorReading(1523, 2363, 2071, 1230, 1272, 1894, "2", 1)); k
			keys.push_back(SensorReading(1783, 1428, 2036, 516, 986, 557, "3"));
			//keys.push_back(SensorReading(1078, 2316, 2045, 1177, 2001, 2311, "4", 1)); b
			keys.push_back(SensorReading(1776, 1422, 2078, 1136, 1863, 661, "5"));
			//keys.push_back(SensorReading(1379, 2272, 2021, 1056, 1293, 1903, "6", 1)); w
			keys.push_back(SensorReading(1194, 1714, 1708, 517, 1889, 718, "7"));
			keys.push_back(SensorReading(1181, 1206, 1198, 1121, 2001, 747, "8"));
			//keys.push_back(SensorReading(1470, 1069, 2043, 1201, 2002, 2328, "9", 1)); f

			keys.push_back(SensorReading(1440, 829, 1052, 545, 850, 747, "a"));
			keys.push_back(SensorReading(1099, 1260, 2034, 1138, 1844, 793, "b"));
			keys.push_back(SensorReading(1625, 956, 1208, 624, 1365, 765, "c"));
			keys.push_back(SensorReading(1308, 1424, 1021, 513, 871, 768, "d"));
			keys.push_back(SensorReading(961, 803, 1085, 488, 921, 715, "e"));
			keys.push_back(SensorReading(1264, 981, 2118, 1228, 1622, 746, "f"));
			keys.push_back(SensorReading(1499, 1044, 1172, 569, 967, 832, "g"));
			keys.push_back(SensorReading(999, 1126, 1940, 575, 1017, 774, "h"));
			keys.push_back(SensorReading(1019, 809, 1194, 482, 1927, 723, "i"));
			keys.push_back(SensorReading(1055, 828, 1253, 485, 1924, 741, "j"));
			keys.push_back(SensorReading(1460, 1455, 1923, 653, 774, 872, "k"));
			keys.push_back(SensorReading(1858, 1379, 1031, 594, 895, 724, "l"));
			keys.push_back(SensorReading(1045, 811, 1221, 565, 1063, 950, "m"));
			keys.push_back(SensorReading(1012, 846, 1194, 662, 963, 773, "n"));
			keys.push_back(SensorReading(1452, 909, 1233, 602, 1261, 738, "o"));
			keys.push_back(SensorReading(1505, 1448, 1935, 626, 964, 780, "p"));
			keys.push_back(SensorReading(1663, 1185, 1207, 599, 968, 777, "q"));
			keys.push_back(SensorReading(1047, 1420, 1525, 496, 866, 670, "r"));
			keys.push_back(SensorReading(1110, 757, 1076, 580, 1188, 694, "s"));
			keys.push_back(SensorReading(1098, 1027, 1197, 665, 1098, 737, "t"));
			keys.push_back(SensorReading(1140, 1393, 1974, 527, 1004, 683, "u"));
			keys.push_back(SensorReading(1168, 1481, 2031, 605, 1172, 669, "v"));
			keys.push_back(SensorReading(1212, 1470, 2061, 1240, 979, 661, "w"));
			keys.push_back(SensorReading(938, 940, 1076, 612, 975, 646, "x"));
			keys.push_back(SensorReading(1714, 768, 1117, 596, 1925, 731, "y"));
			keys.push_back(SensorReading(1047, 1420, 1216, 542, 883, 655, "z"));
		}

		std::string FlexSensorReader::Convert(const SensorReading* flex_reading) const{
			std::string best_key = "-";
			int min_score = 60000;
			int temp_score =  0;
			for (int i=0; i<keys.size() ; i++){
				temp_score = keys[i].distance_from_key(*flex_reading);
				if (temp_score < min_score){
					min_score = temp_score;
					best_key = keys[i].key;
				}
			}
			if (DEBUG) printf("%d",min_score);
			return best_key;
		}

		int FlexSensorReader::Poll(struct SensorReading* flex_reading){
			uint32_t adc_channels[6] = {16u,8u,4u, 2u, 1u, 32u};
			for (int i = 0; i < 6; i++){
				LPC_ADC->ADCR=adc_channels[i]; //set to run on channel x
				ADCSetup();
				StartConversion();
				//while((LPC_ADC->ADGDR&31u)==0);
				wait_ms(10);
				EndConversion();
				uint16_t raw_data = ExtractData();
				if (raw_data > ADC_MAX || raw_data < ADC_MIN){
					printf("Polling error! raw data %d for finger %d \n", raw_data, i);
					return 1;
				}
				// pinky p15(1u), ring p16(2u), middle p17(4u),
				// pointer p18(8u), thumb p19(16u). palm p20(32u)
				if (i==0){
					flex_reading->thumb = raw_data;
					if (DEBUG) printf("thumb: raw_data: %d\n", raw_data);
				} else if (i==1){
					flex_reading->finger1 = raw_data;
					if (DEBUG) printf("finger1: raw_data: %d\n", raw_data);
				} else if (i==2){
					flex_reading->finger2 = raw_data;
					if (DEBUG) printf("finger2: raw_data: %d\n", raw_data);
				} else if (i==3){
					flex_reading->finger3 = raw_data;
					if (DEBUG) printf("finger3: raw_data: %d\n", raw_data);
				} else if (i==4){
					flex_reading->finger4 = raw_data;
					if (DEBUG) printf("finger4: raw_data: %d\n", raw_data);
				} else if (i==5){
					flex_reading->palm = raw_data;
					if (DEBUG) printf("palm: raw_data: %d\n\n", raw_data);
				} else {
					return 0;
				}
			}
			return 0;
		}

		void FlexSensorReader::ADCSetup() const {
			LPC_ADC->ADCR|=0x00000900; //set clock divisor to have divide by x+1 (from 100MHz)
			LPC_ADC->ADCR|=0x00200000; //set to power the ADC
			LPC_SC->PCONP |= (1<<12);
			LPC_PINCON->PINSEL1|=0x01<<14;
		}

		void FlexSensorReader::StartConversion() const {
			LPC_ADC->ADCR|=0x01000000; //set to startConversion
		}

		void FlexSensorReader::EndConversion() const {
			LPC_ADC->ADCR&=0xfeffffff; //set to end conversion
		}

		uint16_t FlexSensorReader::ExtractData() const {
			return (LPC_ADC->ADGDR>>4)&0x00000fff;
		}


	} /* namespace Sensing */
