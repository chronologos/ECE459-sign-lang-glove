#include "flex_sensor.h"
#include "mbed.h"
#include <vector>
#include <iostream>

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
		/* 
		 *
		 */
		keys.reserve(40);
		keys.push_back(SensorReading(1700,2600,1450,1800,1675,2450, "1", 1));
		keys.push_back(SensorReading(1800,2750,2625,1750,1750,2350, "2", 1));
		keys.push_back(SensorReading(2825,2750,2600,1750,1700,2700, "3", 1));
		keys.push_back(SensorReading(1700,2725,2650,3000,2700,2700, "4", 1));
	  keys.push_back(SensorReading(2750,2725,2550,2900,2700,2800, "5", 1));
		keys.push_back(SensorReading(1550,2750,2600,2750,1600,2300, "6", 1));
		keys.push_back(SensorReading(1750,2750,2500,1700,2650,2500, "7", 1));
		keys.push_back(SensorReading(1850,2700,1400,2900,2800,2750, "8", 1));
		keys.push_back(SensorReading(2000,1550,2700,2900,2750,2800, "9", 1));

		keys.push_back(SensorReading(1751, 1140, 1024, 1327, 1246, 1959, "a", 0));
		keys.push_back(SensorReading(1350, 2252, 2170, 2543, 2373, 2025, "b", 0));	
		keys.push_back(SensorReading(1421, 1502, 1140, 1537, 1687, 2111, "c", 0));
		keys.push_back(SensorReading(1448, 2378, 1048, 1338, 1287, 1902, "d", 0));
		keys.push_back(SensorReading(1307, 1044, 1029, 1315, 1323, 2127, "e", 0));
		keys.push_back(SensorReading(1454, 1239, 2375, 3490, 2426, 2281, "f", 0));
		keys.push_back(SensorReading(1896, 2060, 1206, 2014, 1369, 1755, "g", 0));
		keys.push_back(SensorReading(1268, 2145, 2114, 1525, 1481, 2207, "h", 0));
		keys.push_back(SensorReading(1329, 1112, 1059, 1391, 2313, 2093, "i", 0));
		keys.push_back(SensorReading(1339, 1221, 1176, 1483, 2132, 1967, "j", 0));
		keys.push_back(SensorReading(1932, 2385, 2284, 1496, 1376, 1896, "k", 0));
		keys.push_back(SensorReading(2086, 2235, 1100, 1533, 1478, 2029, "l", 0));
		keys.push_back(SensorReading(1489, 1252, 1280, 1459, 1311, 1878, "m", 0));
		keys.push_back(SensorReading(1536, 1272, 1275, 1539, 1292, 2096, "n", 0));
		keys.push_back(SensorReading(1508, 1382, 1197, 1640, 1501, 2326, "o", 0));
		keys.push_back(SensorReading(1944, 2307, 1494, 2922, 1305, 1884, "p", 0));
		keys.push_back(SensorReading(1994, 1984, 1161, 3953, 1380, 2010, "q", 0));
		keys.push_back(SensorReading(2831, 2257, 1683, 1356, 1477, 2034, "r", 0));
		keys.push_back(SensorReading(1950,1450,1475,1800,1750,2650, "s", 0));
		keys.push_back(SensorReading(2450,1800,1500,1950,1700,2500, "t", 0));
		keys.push_back(SensorReading(1950,2750,2600,1875,1750,2500, "u", 0));
		keys.push_back(SensorReading(2000,2900,2750,1900,1800,2500, "v", 0));
		keys.push_back(SensorReading(2000,2900,2750,3000,1825,2400, "w", 0));
		keys.push_back(SensorReading(1650,1850,1550,2025,1650,2500, "x", 0));
		keys.push_back(SensorReading(2900,1700,1650,2000,2500,2750, "y", 0));
		keys.push_back(SensorReading(2000,2700,1600,2000,1700,2700, "z", 0));
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
		// printf("%d",min_score);
		return best_key;
	}

	int FlexSensorReader::Poll(struct SensorReading* flex_reading){
		uint32_t adc_channels[6] = {16u,8u,4u, 2u, 1u, 32u};
		for (int i = 0; i < sizeof(adc_channels); ++i){
			LPC_ADC->ADCR=adc_channels[i]; //set to run on channel x
			ADCSetup();
			StartConversion();
			//while((LPC_ADC->ADGDR&31u)==0);
			wait_ms(20);
			EndConversion();
			uint16_t raw_data = ExtractData();
			// pinky p15(1u), ring p16(2u), middle p17(4u), 
			// pointer p18(8u), thumb p19(16u). palm p20(32u)
			if (i==0){
				flex_reading->thumb = raw_data;
				//printf("thumb: raw_data: %d\n", raw_data);
			} else if (i==1){
				flex_reading->finger1 = raw_data;
				//printf("finger1: raw_data: %d\n", raw_data);
			} else if (i==2){
				flex_reading->finger2 = raw_data;
				//printf("finger2: raw_data: %d\n", raw_data);
			} else if (i==3){
				flex_reading->finger3 = raw_data;
				//printf("finger3: raw_data: %d\n", raw_data);
			} else if (i==4){
				flex_reading->finger4 = raw_data;
				//printf("finger4: raw_data: %d\n", raw_data);
			} else if (i==5){
				flex_reading->palm = raw_data;
				//printf("palm: raw_data: %d\n\n", raw_data);
			} else {
				return 1;
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
