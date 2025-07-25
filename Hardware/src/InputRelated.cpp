#include "InputRelated.h"
#include "config.h"
#include <Arduino.h>
#include "TickTimer.h"

#if ENABLE_DHT == true

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

Input::DHT11Sensor::DHT11Sensor(gpio_num_t pin) {
	this->pin = pin;
	this->dht = new DHT_Unified(pin, DHT11);
	this->temperature = 0.0f;
	this->humidity = 0.0f;
	this->dhtTimer = TickTimer(5000000); // 5 seconds
}

void Input::DHT11Sensor::setup() {
	this->dht->begin();
}

float Input::DHT11Sensor::readTemperature() {
	this->readSensor();
	return this->temperature;
}

float Input::DHT11Sensor::readHumidity() {
	this->readSensor();
	return this->humidity;
}

void Input::DHT11Sensor::readSensor() {
	if (!this->dhtTimer.shouldTick())
		return;

	sensors_event_t event;
	this->dht->temperature().getEvent(&event);

	if (!isnan(event.temperature)) {
		this->temperature = event.temperature;
	}

	this->dht->humidity().getEvent(&event);
	if (!isnan(event.relative_humidity)) {
		this->humidity = event.relative_humidity;
	}
}

#endif

#if ENABLE_WATER_LEVEL_SENSOR == true

Input::WaterLevel::WaterLevel(gpio_num_t pin) {
	this->pin = pin;
	this->waterLevelSensorTimer = TickTimer(1000000); // 1 second
	this->waterLevelSensorTimer.init();
}

void Input::WaterLevel::setup() {
	pinMode(this->pin, INPUT);
}

ushort Input::WaterLevel::readPercent() {
	this->readSensor();

	return round((this->rawValue / 4095.0f) * 100.0f);
}

void Input::WaterLevel::readSensor() {
	if (!this->waterLevelSensorTimer.shouldTick())
		return;

	this->rawValue = analogRead(this->pin); // 0 - 4095

	if (this->rawValue < 0 || this->rawValue > 4095) {
		Serial.println("Error: Water level sensor reading out of range.");
		return;
	}
}

#endif