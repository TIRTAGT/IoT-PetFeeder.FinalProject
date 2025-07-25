#pragma once
#include <hal/gpio_types.h>
#include "config.h"
#include "TickTimer.h"
#include <vector>

struct ServoTarget {
	int degrees;

	gpio_num_t pin;
};

class ServoManager {
	public:
		ServoManager();
		ServoManager(gpio_num_t pin);
		void setup();
		void Rotate(int degrees);
		int getCurrentPosition();

	private:
		ServoTarget target;
};

extern std::vector<ServoTarget*> pServoTargets;
extern TickTimer servoTickTimer; // 20 ms (50 Hz)

void ServoManager_loop();
static void ServoManager_WritePWM();
