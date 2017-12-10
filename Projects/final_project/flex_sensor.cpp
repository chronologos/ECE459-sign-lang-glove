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
			keys.push_back(SensorReading(991, 1255, 1083, 487, 786, 621, "1"));
			//keys.push_back(SensorReading(1523, 2363, 2071, 1230, 1272, 1894, "2", 1)); k
			keys.push_back(SensorReading(1783, 1428, 2036, 516, 986, 557, "3"));
			//keys.push_back(SensorReading(1078, 2316, 2045, 1177, 2001, 2311, "4", 1)); b
			keys.push_back(SensorReading(1627, 1338, 1927, 1045, 1813, 769, "5"));
			//keys.push_back(SensorReading(1379, 2272, 2021, 1056, 1293, 1903, "6", 1)); w
			keys.push_back(SensorReading(1194, 1714, 1708, 517, 1889, 718, "7"));
			keys.push_back(SensorReading(1181, 1206, 1198, 1121, 2001, 747, "8"));
			//keys.push_back(SensorReading(1470, 1069, 2043, 1201, 2002, 2328, "9", 1)); f

			keys.push_back(SensorReading(1398, 860, 1167, 576, 852, 791, "a"));
			keys.push_back(SensorReading(1014, 1248, 2038, 1140, 1892, 838, "b"));
			keys.push_back(SensorReading(1643, 907, 1148, 608, 1291, 768, "c"));
			keys.push_back(SensorReading(1442, 1320, 1084, 526, 892, 716, "d"));
			keys.push_back(SensorReading(903, 814, 1056, 514, 835, 676, "e"));
			keys.push_back(SensorReading(1198, 950, 2129, 1197, 1843, 784, "f"));
			keys.push_back(SensorReading(1408, 1060, 1196, 541, 1922, 739, "g"));
			keys.push_back(SensorReading(984, 1067, 1890, 579, 756, 671, "h"));
			keys.push_back(SensorReading(1006, 846, 1257, 520, 1786, 725, "i"));
			keys.push_back(SensorReading(1138, 893, 1228, 541, 1427, 719, "j"));
			keys.push_back(SensorReading(1320, 1334, 1867, 555, 656, 736, "k"));
			keys.push_back(SensorReading(1863, 1368, 1077, 553, 755, 747, "l"));
			keys.push_back(SensorReading(974, 828, 1293, 555, 919, 810, "m"));
			keys.push_back(SensorReading(910, 820, 1175, 580, 807, 776, "n"));
			keys.push_back(SensorReading(1448, 881, 1194, 594, 1236, 688, "o"));
			keys.push_back(SensorReading(1421, 1421, 1661, 628, 1976, 645, "p"));
			keys.push_back(SensorReading(1660, 1107, 1145, 581, 878, 689, "q"));
			keys.push_back(SensorReading(1072, 1073, 1499, 502, 913, 559, "r"));
			keys.push_back(SensorReading(981, 853, 1185, 530, 790, 724, "s"));
			keys.push_back(SensorReading(974, 876, 1162, 599, 760, 722, "t"));
			keys.push_back(SensorReading(1082, 1309, 2040, 521, 821, 607, "u"));
			keys.push_back(SensorReading(1117, 1444, 2073, 575, 941, 599, "v"));
			keys.push_back(SensorReading(1126, 1455, 2073, 1238, 854, 504, "w"));
			keys.push_back(SensorReading(984, 1036, 1111, 592, 881, 560, "x"));
			keys.push_back(SensorReading(1854, 775, 1181, 612, 1827, 709, "y"));
			keys.push_back(SensorReading(1151, 1427, 1249, 538, 915, 730, "z"));
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
				wait_ms(8);
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
