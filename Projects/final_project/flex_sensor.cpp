#include "flex_sensor.h"
#include "mbed.h"
#include <vector>
#include <iostream>

namespace Sensing {

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

		keys.push_back(SensorReading(2300,1600,1600,1800,1600,2600, "a", 0));
		keys.push_back(SensorReading(1700,2850,2700,2950,2750,2500, "b", 0));
		keys.push_back(SensorReading(2000,2050,1700,2200,2050,2800, "c", 0));
		keys.push_back(SensorReading(1800,2750,1500,1750,1750,2750, "d", 0));
		keys.push_back(SensorReading(1800,1450,1500,1800,1750,2800, "e", 0));
		keys.push_back(SensorReading(2000,1650,2850,3000,2700,2850, "f", 0));
		keys.push_back(SensorReading(2400,2600,1750,2000,1750,2450, "g", 0));
		keys.push_back(SensorReading(1900,2750,2750,1900,1850,2600, "h", 0));
		keys.push_back(SensorReading(1850,1650,1600,1950,2650,2750, "i", 0));
		keys.push_back(SensorReading(1950,1850,1650,2100,2650,2700, "j", 0));
		keys.push_back(SensorReading(2600,2700,2700,2000,1850,2450, "k", 0));
		keys.push_back(SensorReading(2900,2900,1550,1900,1750,2650, "l", 0));
		keys.push_back(SensorReading(1950,1800,1750,2000,1750,2550, "m", 0));
		keys.push_back(SensorReading(2100,1900,1900,2100,1750,2600, "n", 0));
		keys.push_back(SensorReading(2050,2000,1700,2100,2050,2800, "o", 0));
		keys.push_back(SensorReading(2300,2900,2100,2350,2100,2750, "p", 0));
		keys.push_back(SensorReading(2400,2550,1450,1700,1650,2500, "q", 0));
		keys.push_back(SensorReading(1900,2800,2100,1850,1850,2500, "r", 0));
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
		uint32_t adc_channels[6] = {1u,2u,4u,8u,16u,32u};
		for (int i = 0; i < sizeof(adc_channels); ++i){
			LPC_ADC->ADCR=adc_channels[i]; //set to run on channel x
			ADCSetup();
			StartConversion();
			//while((LPC_ADC->ADGDR&31u)==0);
			wait_ms(10);
			EndConversion();
			uint16_t raw_data = ExtractData();
			if (i==0){
				flex_reading->thumb = raw_data;
				// printf("Thumb: raw_data: %d\n", raw_data);
			} else if (i==1){
				flex_reading->finger1 = raw_data;
				// printf("Pointer: raw_data: %d\n", raw_data);
			} else if (i==2){
				flex_reading->finger2 = raw_data;
				// printf("Middle: raw_data: %d\n", raw_data);
			} else if (i==3){
				flex_reading->finger3 = raw_data;
				// printf("Ring: raw_data: %d\n", raw_data);
			} else if (i==4){
				flex_reading->finger4 = raw_data;
				// printf("Pinkie: raw_data: %d\n", raw_data);
			} else if (i==5){
				flex_reading->palm = raw_data;
				// printf("Palm: raw_data: %d\n", raw_data);
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
