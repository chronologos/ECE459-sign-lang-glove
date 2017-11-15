#ifndef FLEX_SENSOR_H
#define FLEX_SENSOR_H

#include <map>
#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>

namespace Sensing {
	
//const int MAX_SCALE = 2700; // Maximum reading from ADC
//const int MIN_SCALE = 1800; // Minimum reading from ADC
//const int NUM_STATES = 3;
//const int DIV_SIZE = (MAX_SCALE-MIN_SCALE)/NUM_STATES;

typedef struct SensorReading {
	uint8_t thumb;
	uint8_t finger1;
	uint8_t finger2;
	uint8_t finger3;
	uint8_t finger4;
	uint8_t palm;
	std::string key;
	SensorReading(int t, int f1, int f2, int f3, int f4, int p, std::string k) : thumb(t), finger1(f1), finger2(f2), finger3(f3), finger4(f4), palm(p), key(k){}
	SensorReading() {}
	
	int distance_from_key (const SensorReading &o) const{
		return min_dist(thumb,o.thumb) + min_dist(finger1,o.finger1) + min_dist(finger2,o.finger2) + min_dist(finger3,o.finger3) + min_dist(finger4,o.finger4) + min_dist(palm,o.palm);
	}

//	bool operator<(const SensorReading &o) const{
//		return (81*thumb+27*finger1+9*finger2+3*finger3+finger4) < (81*o.thumb+27*o.finger1+9*o.finger2+3*o.finger3+o.finger4);
//	}
//	
	int min_dist(int in1, int in2) const{
		return std::abs(in1-in2);
	}
	
	std::string return_key () const{
		return key;
	}
} SensorReading;


/* FlexSensorReader handles all interactions with Flex Sensors and ADC */
class FlexSensorReader {
	public:
		FlexSensorReader();
	
	  // Converts sensor readings to letter/number.
		std::string Convert(const SensorReading* flex_reading) const;
	
		// Polls flex sensors for one reading.
		int Poll(struct SensorReading* flex_reading);
		void ADCSetup() const;
		
	private:
	  std::vector<SensorReading> keys;
		// each int in scale is endpoint of that state
	  // So if MAX_SCALE is 1200 and MIN_SCALE is 300 and NUM_STATES = 3, we should have
	  // {600, 900, 1200}
		std::vector<int> scale; 

		void StartConversion() const;
		void EndConversion() const;
		uint16_t ExtractData() const;
		int ReadingToState(uint16_t reading) const;

}; 

} /* namespace FlexSensor */

#endif /* FLEX_SENSOR_H */
