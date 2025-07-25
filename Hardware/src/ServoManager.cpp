#include <Arduino.h>
#include "ServoManager.h"
#include "TickTimer.h"

TickTimer servoTickTimer(20000); // 20 ms (50 Hz)

#define MANUAL_TICK_GEAR_RATIO 20

#include <vector>
std::vector<ServoTarget*> pServoTargets;
ulong LastTickMicros = 0;

ServoManager::ServoManager() {}

ServoManager::ServoManager(gpio_num_t pin) {
	this->target = {};
	this->target.degrees = 0;
	this->target.pin = pin;

	// Add the ServoTarget to the vector
	pServoTargets.push_back(&this->target);

	Serial.print("ServoManager initialized with pin: ");
	Serial.print(pin);
	Serial.print(", size of pServoTargets: ");
	Serial.println(pServoTargets.size());
}

void ServoManager::setup() {
	pinMode(this->target.pin, OUTPUT);
}

void ServoManager::Rotate(int degrees) {
	this->target.degrees = degrees;
}

int ServoManager::getCurrentPosition() {
	return this->target.degrees;
}

void ServoManager_loop() {
	// If 20 ms have passed
	if (!servoTickTimer.shouldTick())
		return;

	ServoManager_WritePWM();
}

static void ServoManager_WritePWM() {
	// DOCS: Range is 470-2572 microseconds

	// For each ServoTargetDegrees
	int ArrayCount = pServoTargets.size();

	if (ArrayCount == 0) {
		Serial.println("No ServoTargets to process.");
		return;
	}

	for (int i = 0; i < ArrayCount; i++) {
		ServoTarget* Target = pServoTargets[i];

		float TargetuS = 470.0f + (Target->degrees * 11.5f);

		digitalWrite(Target->pin, HIGH);
		delayMicroseconds((uint32_t)TargetuS);
		digitalWrite(Target->pin, LOW);

		#if SERVO_SERIAL_DEBUG == true
			Serial.print("Servo on pin ");
			Serial.print(Target->pin);
			Serial.print(" set to ");
			Serial.print(Target->degrees);
			Serial.print(" degrees (");
			Serial.print(TargetuS);
			Serial.println(" microseconds)");
		#endif
	}
}