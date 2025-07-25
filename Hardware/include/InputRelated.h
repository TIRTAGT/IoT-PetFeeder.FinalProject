#pragma once
#include "config.h"
#include "TickTimer.h"
#include <hal/gpio_types.h>
#include <sys/types.h>
#include <sys/config.h>

#if ENABLE_DHT == true
	#include <Adafruit_Sensor.h>
	#include <DHT.h>
	#include <DHT_U.h>
#endif

class Input {
	public:
		#if ENABLE_DHT == true
			class DHT11Sensor {
				public:
					DHT11Sensor(gpio_num_t pin);
					void setup();
					float readTemperature();
					float readHumidity();

				private:
					gpio_num_t pin;
					DHT_Unified* dht;
					float temperature;
					float humidity;
					TickTimer dhtTimer;
					void readSensor();
			};
		#endif

		#if ENABLE_WATER_LEVEL_SENSOR == true
			class WaterLevel {
				public:
					WaterLevel(gpio_num_t pin);
					void setup();
					ushort readPercent();

				private:
					gpio_num_t pin;
					uint16_t rawValue = 0;
					TickTimer waterLevelSensorTimer;
					void readSensor();
			};
		#endif
};