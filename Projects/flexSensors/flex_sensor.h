#ifndef FLEX_SENSOR_H
#define FLEX_SENSOR_H

#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace Sensing {
	
const int MAX_SCALE = 2700; // Maximum reading from ADC
const int MIN_SCALE = 1800; // Minimum reading from ADC
const int NUM_STATES = 3;
const int DIV_SIZE = (MAX_SCALE-MIN_SCALE)/NUM_STATES;

typedef struct SensorReading {
	int thumb;
	int finger1;
	int finger2;
	int finger3;
	int finger4;
//	int palm;
	SensorReading(int t, int f1, int f2, int f3, int f4) : thumb(t), finger1(f1), finger2(f2), finger3(f3), finger4(f4){}
	SensorReading() {}
	
	bool operator==(const SensorReading &o) const{
		return thumb == o.thumb && finger1 == o.finger1 && finger2 == o.finger2 && finger3 == o.finger3 && finger4 == o.finger4;
	}

	bool operator<(const SensorReading &o) const{
		return (81*thumb+27*finger1+9*finger2+3*finger3+finger4) < (81*o.thumb+27*o.finger1+9*o.finger2+3*o.finger3+o.finger4);
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
		std::map<SensorReading, std::string> LUT;
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
