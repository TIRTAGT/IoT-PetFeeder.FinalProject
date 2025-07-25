#include <TickTimer.h>
#include <Arduino.h>

TickTimer::TickTimer() {
	this->lastMicros = 0;
}

TickTimer::TickTimer(ulong tick_micros) {
	this->targetTickMicros = tick_micros;
}

void TickTimer::init() {
	this->lastMicros = micros();
}

void TickTimer::setTickMicros(ulong tick_micros) {
	this->targetTickMicros = tick_micros;
}

ulong TickTimer::getTickMicros() {
	return this->targetTickMicros;
}

bool TickTimer::shouldTick() {
	ulong currentMicros = micros();

	if (this->lastMicros == 0) {
		this->lastMicros = currentMicros;
		return true; // First tick
	}

	if (currentMicros - this->lastMicros >= this->targetTickMicros) {
		this->lastMicros = currentMicros;
		return true;
	}
	
	return false;
}