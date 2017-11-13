#include "flex_sensor.h"
#include "mbed.h"                    
#include <vector>
#include <iostream>

namespace Sensing {
	
	FlexSensorReader::FlexSensorReader() {
		keys.push_back(SensorReading(2000,2800,1400,1800,1600, "1"));
		keys.push_back(SensorReading(2000,3000,2700,1700,1800, "2"));
		keys.push_back(SensorReading(3000,2900,2650,1800,1800, "3"));
		keys.push_back(SensorReading(1900,3000,2700,3000,2800, "4"));
	  keys.push_back(SensorReading(3000,2900,2600,2800,2650, "5"));
		keys.push_back(SensorReading(2000,3000,2700,2900,1700, "6"));
		keys.push_back(SensorReading(2100,2900,2600,1700,2700, "7"));
		keys.push_back(SensorReading(2300,2900,1400,2900,2800, "8"));
		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "9"));
		
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "a"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "b"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "c"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "d"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "e"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "f"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "g"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "h"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "i"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "j"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "k"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "l"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "m"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "n"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "o"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "p"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "q"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "r"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "s"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "t"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "u"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "v"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "w"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "x"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "y"));
//		keys.push_back(SensorReading(2300,1700,2700,2900,2700, "z"));
		
		
//		for (int i = 0; i < NUM_STATES; ++i) {
//			scale.push_back(MIN_SCALE+(i+1)*DIV_SIZE);
//		}
	}
		
	std::string FlexSensorReader::Convert(const SensorReading* flex_reading) const{
		printf("convert\n");
//		std::map<SensorReading, std::string>::const_iterator found = LUT.find(*flex_reading);
//		if (found == LUT.end()){
//			printf("key [%d,%d,%d,%d,%d] : value not found\n",flex_reading->thumb,flex_reading->finger1,flex_reading->finger2,flex_reading->finger3,flex_reading->finger4);
//			return "-";
//		} else {
//			printf("key [%d,%d,%d,%d,%d] : value %s\n",flex_reading->thumb,flex_reading->finger1,flex_reading->finger2,flex_reading->finger3,flex_reading->finger4,found->second.c_str());
//				return found->second;
//		}
		std::string best_key = "-";
		uint16_t min_score = 60000;
		int temp_score =  0;
		for (int i =0; i < keys.size(); ++i){
			temp_score = keys[i].distance_from_key(*flex_reading);
			if (temp_score < min_score){
				min_score = temp_score;
				best_key = keys[i].key;
			}
			printf("Number:%d, distance: %d, min_distance %d\n",i+1,temp_score,min_score);	
		}
		printf("convert done\n");
		return best_key;
	}
	
	int FlexSensorReader::Poll(struct SensorReading* flex_reading){
		uint32_t adc_channels[6] = {1u,2u,4u,8u,16u,32u};
		// uint32_t adc_channels[5] = {1u,2u,4u,8u,16u};
		for (int i = 0; i < sizeof(adc_channels); ++i){
			LPC_ADC->ADCR=adc_channels[i]; //set to run on channel x
			ADCSetup(i);
			wait_ms(40);
			StartConversion();
			//while((LPC_ADC->ADGDR&31u)==0);
			EndConversion();
			uint16_t raw_data = ExtractData();
			if (i==0){
				flex_reading->thumb = raw_data;
				printf("Thumb: raw_data: %d, channel: %d\n", raw_data, adc_channels[i]);
			} else if (i==1){
				flex_reading->finger1 = raw_data;
				printf("Pointer: raw_data: %d, channel: %d\n", raw_data, adc_channels[i]);
			} else if (i==2){
				flex_reading->finger2 = raw_data;
				printf("Middle: raw_data: %d, channel: %d\n", raw_data, adc_channels[i]);
			} else if (i==3){
				flex_reading->finger3 = raw_data;
				printf("Ring: raw_data: %d, channel: %d\n", raw_data, adc_channels[i]);
			} else if (i==4){
				flex_reading->finger4 = raw_data;
				printf("Pinkie: raw_data: %d, channel: %d\n", raw_data, adc_channels[i]);
			}	else if (i==5){
				flex_reading->palm = raw_data;
				printf("Palm: raw_data: %d, channel: %d\n", raw_data, adc_channels[i]);
			} else {
				return 1;
			}
		}
		return 0;
	}
	
	void FlexSensorReader::ADCSetup(int i) const {	
		LPC_ADC->ADCR|=0x00000900; //set clock divisor to have divide by x+1 (from 100MHz)
		LPC_ADC->ADCR|=0x00200000; //set to power the ADC
		LPC_SC->PCONP |= (1<<12);
		if (i < 3){
			LPC_PINCON->PINSEL1|=0x01<<(14+2*i);
		} else {
			LPC_PINCON->PINSEL3|=0x01<<(20+2*i);
		}
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
	
//	int FlexSensorReader::ReadingToState(uint16_t reading) const {
//		for (int i = 0; i < NUM_STATES; ++i) {
//			if (reading < scale[i]){
//				return i;
//			} 
//		}
//		return (NUM_STATES-1);
//	}


} /* namespace Sensing */
