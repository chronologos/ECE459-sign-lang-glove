#include "flex_sensor.h"
#include "mbed.h"
#include <vector>
#include <iostream>

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
		/* 
		 *
		 */
		keys.reserve(40);
		//keys.push_back(SensorReading(1563, 2297, 1272, 631, 1206, 2007, "1", 1)); d
		//keys.push_back(SensorReading(1523, 2363, 2071, 1230, 1272, 1894, "2", 1)); k
		keys.push_back(SensorReading(1832, 2299, 2025, 564, 1682, 2135, "3", 1));
		//keys.push_back(SensorReading(1078, 2316, 2045, 1177, 2001, 2311, "4", 1)); b
	  keys.push_back(SensorReading(1683, 1980, 1922, 1004, 1757, 2208, "5", 1));
		//keys.push_back(SensorReading(1379, 2272, 2021, 1056, 1293, 1903, "6", 1)); w
		keys.push_back(SensorReading(1322, 2585, 1950, 550, 1942, 2058, "7", 1));
		keys.push_back(SensorReading(1318, 3454, 1150, 1027, 1964, 2222, "8", 1));
		keys.push_back(SensorReading(1470, 1069, 2043, 1201, 2002, 2328, "9", 1));

		keys.push_back(SensorReading(1281, 1256, 1128, 559, 954, 2048, "a", 0));
		keys.push_back(SensorReading(1829, 1856, 1974, 1152, 1985, 2126, "b", 0));	
		keys.push_back(SensorReading(1477, 1475, 1151, 617, 2026, 2176, "c", 0));
		keys.push_back(SensorReading(1886, 1914, 1106, 546, 1927, 1509, "d", 0));
		keys.push_back(SensorReading(1105, 1106, 1099, 516, 1651, 2068, "e", 0));
		keys.push_back(SensorReading(1387, 1382, 2074, 1240, 2012, 2238, "f", 0));
		keys.push_back(SensorReading(1820, 1829, 1254, 587, 1085, 1779, "g", 0));
		keys.push_back(SensorReading(1451, 1897, 1944, 595, 1659, 1720, "h", 0));
		keys.push_back(SensorReading(734, 723, 1143, 533, 2030, 1989, "i", 0));
		keys.push_back(SensorReading(1362, 1024, 1205, 602, 1962, 2029, "j", 0));
		keys.push_back(SensorReading(1553, 2270, 2033, 581, 1119, 1841, "k", 0));
		keys.push_back(SensorReading(1819, 2261, 1230, 632, 1141, 2079, "l", 0));
		keys.push_back(SensorReading(1094, 1125, 1371, 642, 1131, 1874, "m", 0));
		keys.push_back(SensorReading(1271, 1167, 1387, 631, 1093, 2133, "n", 0));
		keys.push_back(SensorReading(1485, 1237, 1288, 638, 1379, 2088, "o", 0));
		keys.push_back(SensorReading(1729, 2596, 1626, 610, 1093, 1889, "p", 0));
		keys.push_back(SensorReading(1709, 1773, 1241, 632, 1130, 2050, "q", 0));
		keys.push_back(SensorReading(1342, 2301, 1868, 521, 1057, 2006, "r", 0));
		keys.push_back(SensorReading(1343, 2751, 1310, 610, 1184, 2147, "s", 0));
		keys.push_back(SensorReading(1394, 2569, 1297, 673, 1157, 2156, "t", 0));
		keys.push_back(SensorReading(1376, 2463, 2071, 577, 1139, 2125, "u", 0));
		keys.push_back(SensorReading(1388, 2331, 2056, 595, 1197, 2013, "v", 0));
		keys.push_back(SensorReading(1374, 2321, 2090, 1188, 1199, 1740, "w", 0));
		keys.push_back(SensorReading(995, 1953, 1216, 629, 2096, 1650, "x", 0));
		keys.push_back(SensorReading(1782, 1005, 1235, 609, 2192, 2063, "y", 0));
		keys.push_back(SensorReading(1298, 2306, 1268, 582, 1144, 1708, "z", 0));
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
