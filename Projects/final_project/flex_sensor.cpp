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
			keys.push_back(SensorReading(1783, 1428, 2036, 516, 986, 557, "3", 1));
			//keys.push_back(SensorReading(1078, 2316, 2045, 1177, 2001, 2311, "4", 1)); b
			keys.push_back(SensorReading(1776, 1422, 2078, 1136, 1863, 661, "5", 1));
			//keys.push_back(SensorReading(1379, 2272, 2021, 1056, 1293, 1903, "6", 1)); w
			keys.push_back(SensorReading(1194, 1714, 1708, 517, 1889, 718, "7", 1));
			keys.push_back(SensorReading(1181, 1206, 1198, 1121, 2001, 747, "8", 1));
			//keys.push_back(SensorReading(1470, 1069, 2043, 1201, 2002, 2328, "9", 1)); f

			keys.push_back(SensorReading(1437, 817, 1127, 574, 973, 662, "a", 0));
			keys.push_back(SensorReading(1137, 1305, 2049, 1187, 1977, 744, "b", 0));
			keys.push_back(SensorReading(1241, 920, 1229, 587, 1330, 717, "c", 0));
			keys.push_back(SensorReading(1177, 1413, 1145, 519, 917, 642, "d", 0));
			keys.push_back(SensorReading(1128, 847, 1183, 547, 1048, 692, "e", 0));
			keys.push_back(SensorReading(1312, 950, 2067, 1201, 1961, 702, "f", 0));
			keys.push_back(SensorReading(1519, 1101, 1282, 601, 1102, 732, "g", 0));
			keys.push_back(SensorReading(1077, 1132, 1973, 570, 1103, 679, "h", 0));
			keys.push_back(SensorReading(1096, 922, 1012, 508, 1951, 702, "i", 0));
			keys.push_back(SensorReading(1191, 867, 1174, 744, 1790, 628, "j", 0));
			keys.push_back(SensorReading(1391, 1345, 1985, 611, 939, 783, "k", 0));
			keys.push_back(SensorReading(1881, 1373, 1124, 604, 1034, 702, "l", 0));
			keys.push_back(SensorReading(996, 850, 1324, 592, 1059, 725, "m", 0));
			keys.push_back(SensorReading(1105, 893, 1360, 614, 1026, 692, "n", 0));
			keys.push_back(SensorReading(1391, 858, 1298, 615, 1224, 685, "o", 0));
			keys.push_back(SensorReading(1695, 1139, 1389, 615, 1022, 704, "p", 0));
			keys.push_back(SensorReading(1669, 1063, 1257, 577, 1005, 670, "q", 0));
			keys.push_back(SensorReading(1178, 1655, 1685, 525, 1023, 679, "r", 0));
			keys.push_back(SensorReading(1084, 841, 1266, 551, 1030, 682, "s", 0));
			keys.push_back(SensorReading(1211, 1018, 1134, 565, 1068, 723, "t", 0));
			keys.push_back(SensorReading(1107, 1381, 1989, 539, 1061, 603, "u", 0));
			keys.push_back(SensorReading(1112, 1430, 2121, 579, 1101, 651, "v", 0));
			keys.push_back(SensorReading(1187, 1451, 2145, 1281, 1133, 626, "w", 0));
			keys.push_back(SensorReading(873, 905, 1189, 611, 1093, 726, "x", 0));
			keys.push_back(SensorReading(1762, 841, 1255, 520, 1893, 670, "y", 0));
			keys.push_back(SensorReading(1162, 1426, 1279, 557, 1063, 630, "z", 0));
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
