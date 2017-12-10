#ifndef FLEX_SENSOR_H
#define FLEX_SENSOR_H

#include "mbed.h"
#include <map>
#include <math.h>
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
		int thumb;
		int finger1;
		int finger2;
		int finger3;
		int finger4;
		int palm;
		std::string key;

		SensorReading(int t, int f1, int f2, int f3, int f4, int p,
			std::string k) :
			thumb(t), finger1(f1), finger2(f2), 
			finger3(f3), finger4(f4), palm(p),
			key(k) {}

			SensorReading() {}

			void printify() const {
				printf("keys.push_back(SensorReading(%d, %d, %d, %d, %d, %d, \n", thumb, finger1, finger2, finger3, finger4, palm);
			}

			int distance_from_key (const SensorReading &o) const{
				int dist = sqrt(double(min_dist(thumb,o.thumb) + min_dist(finger1,o.finger1) +
				min_dist(finger2,o.finger2) + min_dist(finger3,o.finger3) +
				min_dist(finger4,o.finger4) + 3*min_dist(palm,o.palm)));
				//TODO(iantay) THIS IS A WORKAROUND FOR OUR PALM SENSOR's LOW RANGE
				return dist;
			}

			int min_dist(int in1, int in2) const{
				int dist =  std::abs(in1-in2)*std::abs(in1-in2);
				return dist;
			}

		} SensorReading;

		void averageReadings(
			SensorReading* srp1, SensorReading* srp2,
			SensorReading* srp3, SensorReading* srp4,
			SensorReading* srp5, SensorReading* avg);

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
				void StartConversion() const;
				void EndConversion() const;
				uint16_t ExtractData() const;
				int ReadingToState(uint16_t reading) const;

			};

		} /* namespace FlexSensor */

		#endif /* FLEX_SENSOR_H */
