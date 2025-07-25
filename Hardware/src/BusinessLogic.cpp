#include <Arduino.h>
#include "BusinessLogic.h"

BusinessLogic::BusinessLogic() {
	this->temperature = 0.0f;
	this->humidity = 0.0f;
	this->waterLevel = 0;
	this->shouldEnableWaterPump = false;
	this->shouldDispenseFood = false;
	this->businessLogicTimer = TickTimer(100000); // 0.1 second
	this->waterPumpEnableTime = 0;

	this->isWaitingForServerActuatorData = false;
	this->isWaitingForServerReportACK = false;

	#if ENABLE_WIFI == true
		this->wsInteractionTimer = TickTimer();
		#if WS_RTX_ON == true
			this->wsInteractionTimer.setTickMicros(100000); // 0.1 second
		# else
			this->wsInteractionTimer.setTickMicros(3000000); // 3 second
		#endif

		this->shouldPushOrPull = true; // Default to push mode
	#endif
	
}

void BusinessLogic::setup() {
	// Any setup code for business logic can go here
	this->businessLogicTimer.init();
}

bool BusinessLogic::should_loop_tick() {
	return this->businessLogicTimer.shouldTick();
}

void BusinessLogic::pre_sensor_read_loop() {
	if (this->isWaitingForServerActuatorData) {
		return;
	}

	#if ENABLE_WIFI == false
		return;
	#endif

	// If in push mode, do not request data
	if (this->shouldPushOrPull) return;

	if (!this->wifiNetwork->isConnected() || !this->wifiNetwork->isServerConnected()) {
		Serial.println("Not connected to WiFi or WebSocket server, skipping actuator data request.");
		return;
	}

	if (!this->wsInteractionTimer.shouldTick()) return;

	if (this->isWaitingForServerActuatorData) {
		Serial.println("Already requested server actuator data, skipping request.");
		return;
	}

	JsonDocument doc;
	doc["key"] = "/iot/get_data";
	this->wifiNetwork->sendJSON(doc);
	this->isWaitingForServerActuatorData = true;
}

void BusinessLogic::pre_actuator_loop() {
	#if ENABLE_WATER_LEVEL_SENSOR == true
		this->handleWaterPumpLogic();
	#endif

	#if ENABLE_SERVO == true
		this->handleServoLogic();
	#endif
}

void BusinessLogic::pre_hardware_report_loop() {
	#if ENABLE_WIFI == true
		this->ReportData();
	#endif
}

#if ENABLE_WIFI == true
void BusinessLogic::setWiFiNetworkInstance(WiFiNetwork* wifiNetwork) {
	this->wifiNetwork = wifiNetwork;
}
#endif

#if ENABLE_SERVO == true
void BusinessLogic::setServoManagerInstance(ServoManager* pServoManager) {
	this->pServoManager = pServoManager;
}
#endif

#if ENABLE_WATER_LEVEL_SENSOR == true
void BusinessLogic::handleWaterPumpLogic() {
	bool EnableWaterPump = this->waterPumpEnabled;

	if (this->shouldEnableWaterPump) {
		EnableWaterPump = true;
		this->shouldEnableWaterPump = false; // Reset the flag
	}

	// GUARD CLAUSE: If water level is above threshold, do not trigger on
	if (EnableWaterPump && waterLevel >= 50) {
		EnableWaterPump = false;
		Serial.println("Water overflow detected, water pump will not be enabled.");
	}

	// TIMING CLAUSE: If the water pump has been enabled for too long, disable it
	if (EnableWaterPump && this->waterPumpEnableTime != 0) {
		if (millis() - this->waterPumpEnableTime > WATER_PUMP_ENABLE_TIMEOUT) {
			EnableWaterPump = false;
			this->waterPumpEnableTime = 0;
			Serial.println("Water pump timeout reached, disabling water pump.");
		}
	}

	digitalWrite(WATER_PUMP_PIN, !EnableWaterPump);
	
	this->waterPumpEnabled = !digitalRead(WATER_PUMP_PIN);
	if (this->waterPumpEnabled && this->waterPumpEnableTime == 0) {
		this->waterPumpEnableTime = millis();
	}
}
#endif

#if ENABLE_SERVO == true
void BusinessLogic::handleServoLogic() {
	bool ServoShouldOpenFood = pServoManager->getCurrentPosition() < ((SERVO_CLOSE_ANGLE + SERVO_OPEN_ANGLE) / 2);

	if (this->shouldDispenseFood) {
		ServoShouldOpenFood = true;
		this->shouldDispenseFood = false; // Reset the flag
	}

	// TIMING CLAUSE: If servo is open for too long, close it
	if (ServoShouldOpenFood && this->servoOpenTime != 0) {
		if (millis() - this->servoOpenTime > SERVO_OPEN_TIMEOUT) {
			ServoShouldOpenFood = false;
			this->servoOpenTime = 0;
			Serial.println("Servo open timeout reached, closing servo.");
		}
	}

	if (ServoShouldOpenFood) {
		pServoManager->Rotate(SERVO_OPEN_ANGLE); // Open the food servo
		if (this->servoOpenTime == 0) {
			this->servoOpenTime = millis();
		}
	}
	else {
		pServoManager->Rotate(SERVO_CLOSE_ANGLE); // Close the food servo
	}
}
#endif

#if ENABLE_WIFI == true
void BusinessLogic::ReportData() {
	// If in pull mode, do not send data
	if (!this->shouldPushOrPull) return;

	if (!this->wifiNetwork->isConnected() || !this->wifiNetwork->isServerConnected() || !this->wifiNetwork->isLoggedIn()) return;

	if (!this->wsInteractionTimer.shouldTick()) return;

	if (this->isWaitingForServerReportACK) {
		return;
	}

	JsonDocument doc;
	doc["key"] = "/iot/post_data";
	doc["data"]["te"] = this->temperature;
	doc["data"]["hu"] = this->humidity;
	doc["data"]["wa"] = this->waterLevel;
	doc["data"]["PuEn"] = this->waterPumpEnabled ? 1 : 0;

	bool isFoodDispenserOpen = (this->pServoManager->getCurrentPosition() < ((SERVO_CLOSE_ANGLE + SERVO_OPEN_ANGLE) / 2));
	doc["data"]["DiFo"] = isFoodDispenserOpen ? 1 : 0;
	
	if (!this->wifiNetwork->sendJSON(doc)) {
		Serial.println("Failed to send data to server, will retry later.");
		return;
	}

	this->isWaitingForServerReportACK = true;
}
#endif
