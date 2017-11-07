#include "flex_sensor.h"
#include "mbed.h"                    

namespace Sensing {
	
	FlexSensorReader::FlexSensorReader() {
		LUT[SensorReading(1,2,0,0,0)] = "1";
		LUT[SensorReading(1,2,2,0,0)] = "2";
		LUT[SensorReading(2,2,2,0,0)] = "3";
		LUT[SensorReading(0,2,2,2,2)] = "4";
		LUT[SensorReading(2,2,2,2,2)] = "5";
		LUT[SensorReading(1,2,2,2,0)] = "6";
		LUT[SensorReading(1,2,2,0,2)] = "7";
		LUT[SensorReading(1,2,0,2,2)] = "8";
		LUT[SensorReading(1,0,2,2,2)] = "9";
		
		for (int i = 0; i < NUM_STATES; ++i) {
			scale.push_back(MIN_SCALE+(i+1)*DIV_SIZE);
		}
	}
			
	std::string FlexSensorReader::Convert(const SensorReading* flex_reading) const{
		printf("convert\n");
		std::map<SensorReading, std::string>::const_iterator found = LUT.find(*flex_reading);
		if (found == LUT.end()){
			printf("key [%d,%d,%d,%d,%d] : value not found\n",flex_reading->thumb,flex_reading->finger1,flex_reading->finger2,flex_reading->finger3,flex_reading->finger4);
			return "-";
		} else {
			printf("key [%d,%d,%d,%d,%d] : value %s\n",flex_reading->thumb,flex_reading->finger1,flex_reading->finger2,flex_reading->finger3,flex_reading->finger4,found->second.c_str());
				return found->second;
		}
		printf("convert done\n");

	}
	
	int FlexSensorReader::Poll(struct SensorReading* flex_reading){
		//uint32_t adc_channels[6] = {1u,2u,4u,8u,16u,32u};
		uint32_t adc_channels[5] = {1u,2u,4u,8u,16u};
		for (int i = 0; i < sizeof(adc_channels); ++i){
			LPC_ADC->ADCR=adc_channels[i]; //set to run on channel x
			ADCSetup();
			wait_ms(10);
			StartConversion();
			//while((LPC_ADC->ADGDR&31u)==0);
			wait_ms(10);
			EndConversion();
			uint16_t raw_data = ExtractData();
			int state = ReadingToState(raw_data);
			if (i==0){
				flex_reading->thumb = state;
				printf("Thumb: raw_data: %d, state: %d\n", raw_data,state);
			} else if (i==1){
				flex_reading->finger1 = state;
				printf("Pointer: raw_data: %d, state: %d\n", raw_data,state);
			} else if (i==2){
				flex_reading->finger2 = state;
				printf("Middle: raw_data: %d, state: %d\n", raw_data,state);
			} else if (i==3){
				flex_reading->finger3 = state;
				printf("Ring: raw_data: %d, state: %d\n", raw_data,state);
			} else if (i==4){
				flex_reading->finger4 = state;
				printf("Pinkie: raw_data: %d, state: %d\n", raw_data,state);
//			} else if (i==5){
//				flex_reading->palm = state;
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
	
	int FlexSensorReader::ReadingToState(uint16_t reading) const {
		for (int i = 0; i < NUM_STATES; ++i) {
			if (reading < scale[i]){
				return i;
			} 
		}
		return (NUM_STATES-1);
	}


} /* namespace Sensing */
