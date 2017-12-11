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
			
			keys.push_back(SensorReading(1682, 773, 963, 481, 610, 751, "a"));
			keys.push_back(SensorReading(1057, 1319, 2056, 1170, 1645, 810, "b"));
			keys.push_back(SensorReading(1495, 855, 1151, 581, 1245, 731, "c"));
			keys.push_back(SensorReading(1101, 1349, 1100, 505, 776, 668, "d"));
			keys.push_back(SensorReading(947, 791, 1067, 493, 783, 701, "e"));
			keys.push_back(SensorReading(1282, 1051, 2089, 1229, 1961, 745, "f"));
			keys.push_back(SensorReading(1380, 1049, 1199, 538, 831, 739, "g"));
			keys.push_back(SensorReading(1090, 1137, 1884, 545, 822, 731, "h"));
			keys.push_back(SensorReading(1061, 834, 1129, 497, 1527, 786, "i"));
			keys.push_back(SensorReading(1075, 825, 1180, 489, 1675, 715, "j"));
			keys.push_back(SensorReading(1392, 1330, 1975, 567, 811, 777, "k"));
			keys.push_back(SensorReading(1910, 1399, 1059, 558, 797, 685, "l"));
			keys.push_back(SensorReading(996, 826, 1185, 1068, 1062, 905, "m"));
			keys.push_back(SensorReading(1027, 836, 875, 589, 824, 697, "n"));
			keys.push_back(SensorReading(1425, 926, 1203, 576, 1006, 689, "o"));
			keys.push_back(SensorReading(1502, 1317, 1930, 606, 832, 735, "p"));
			keys.push_back(SensorReading(1642, 1049, 1106, 552, 812, 745, "q"));
			keys.push_back(SensorReading(1029, 1006, 1137, 475, 747, 614, "r"));
			keys.push_back(SensorReading(994, 955, 1114, 566, 939, 714, "s"));
			keys.push_back(SensorReading(997, 872, 1170, 538, 812, 718, "t"));
			keys.push_back(SensorReading(1029, 1272, 1983, 495, 665, 611, "u"));
			keys.push_back(SensorReading(1057, 1428, 2130, 518, 706, 666, "v"));
			keys.push_back(SensorReading(1137, 1421, 2115, 1264, 788, 610, "w"));
			keys.push_back(SensorReading(939, 1071, 1025, 574, 859, 640, "x"));
			keys.push_back(SensorReading(1792, 776, 1146, 572, 1914, 767, "y"));
			keys.push_back(SensorReading(911, 1335, 1129, 515, 797, 646, "z"));
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
