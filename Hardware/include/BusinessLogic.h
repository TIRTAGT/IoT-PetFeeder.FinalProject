#pragma once
#include "TickTimer.h"
#include "config.h"
#include "ServoManager.h"

#if ENABLE_WIFI == true
	#include "WiFiNetwork.h"
#endif

class BusinessLogic {
	public:
		BusinessLogic();
		void setup();
		bool should_loop_tick();
		void pre_sensor_read_loop();
		void pre_actuator_loop();
		void pre_hardware_report_loop();

		float temperature; // \*C
		float humidity; // %
		int waterLevel; // 0 - 100
		bool waterPumpEnabled;

		bool shouldEnableWaterPump;
		bool shouldDispenseFood;

		#if ENABLE_WIFI == true
			bool isWaitingForServerActuatorData;
			bool isWaitingForServerReportACK;

			// True if the WebSocket should push, false if it should pull
			bool shouldPushOrPull;
			void setWiFiNetworkInstance(WiFiNetwork* wifiNetwork);
		#endif

		#if ENABLE_SERVO == true
			void setServoManagerInstance(ServoManager* pServoManager);
		#endif

	private:
		void handleWaterPumpLogic();
		void handleServoLogic();

		ulong waterPumpEnableTime;
		ulong servoOpenTime;

		TickTimer businessLogicTimer;
		#if ENABLE_WIFI == true
			WiFiNetwork* wifiNetwork;
			void ReportData();
			TickTimer wsInteractionTimer;
		#endif

		#if ENABLE_SERVO == true
			ServoManager* pServoManager;
		#endif
};