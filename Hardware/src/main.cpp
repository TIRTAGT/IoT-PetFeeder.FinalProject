#include <Arduino.h>
#include "TickTimer.h"
#include "InputRelated.h"
#include "config.h"
#include "BusinessLogic.h"

BusinessLogic businessLogic;

TickTimer HardwareReportingTimer(1000000); // 1 second
void Serial_StatusReport();

#if ENABLE_WIFI == true
	#include "WiFiNetwork.h"

	WiFiNetwork wifiNetwork(WIFI_SSID, WIFI_PASSWORD);
	void OnWebSocketMessage(const JsonDocument& doc);
#endif

#if ENABLE_LCD_OUTPUT == true
	#include <Wire.h>
	#include <LiquidCrystal_I2C.h>

	bool isLCDInitialized = false;
	void LCD_StatusReport();
#endif

#pragma region Actuator Related Variables and Functions

#if ENABLE_SERVO == true
	#include <ServoManager.h>

	ServoManager* pServoManager;
#endif

#pragma endregion

#pragma region Sensor Related Stuff

#if ENABLE_DHT == true
	Input::DHT11Sensor dht1(DHT_PIN);
#endif

#if ENABLE_WATER_LEVEL_SENSOR == true
	Input::WaterLevel waterLevel1(WATER_LEVEL_SENSOR_PIN);
#endif

void ReadSensors() {
	#if ENABLE_DHT == true
		businessLogic.temperature = dht1.readTemperature();
		businessLogic.humidity = dht1.readHumidity();
	#endif

	#if ENABLE_WATER_LEVEL_SENSOR == true
		businessLogic.waterLevel = waterLevel1.readPercent();
	#endif
}

#pragma endregion

#if ENABLE_LCD_OUTPUT == true
	LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
#endif

void setup() {
	Serial.begin(115200);

	pinMode(2, OUTPUT); // D2 is LED_BUILTIN on ESP32

	#if ENABLE_WATER_PUMP == true
		pinMode(WATER_PUMP_PIN, OUTPUT);
		digitalWrite(WATER_PUMP_PIN, HIGH); // Ensure the pump is off at startup
	#endif

	#if ENABLE_LCD_OUTPUT == true
		if (lcd.begin(LCD_COLUMNS_SIZE, LCD_ROWS_SIZE, LCD_5x8DOTS) == 1) // columns, rows, characters size
			isLCDInitialized = true;

		if (isLCDInitialized) {
			lcd.clear();
			char firstLine[17];
			snprintf(
				firstLine,
				sizeof(firstLine),
				"Space  :  %d KB",
				ESP.getFreeHeap() / 1024
			);

			lcd.setCursor(0, 0);
			lcd.print(firstLine);

			char secondLine[17];

			for (size_t i = 0; i < 12; i++)
			{
				snprintf(
					secondLine,
					sizeof(secondLine),
					"CPU    : %d MHz",
					ESP.getCpuFreqMHz()
				);
				lcd.setCursor(0, 1);
				lcd.print(secondLine);
				delay(100);
			}

			lcd.clear();
		}
	#endif

	#if ENABLE_DHT == true
		dht1.setup();
	#endif

	#if ENABLE_WATER_LEVEL_SENSOR == true
		waterLevel1.setup();
	#endif

	#if ENABLE_SERVO == true
		pServoManager = new ServoManager(SERVO1_PIN);
		pServoManager->setup();
		pServoManager->Rotate(SERVO_CLOSE_ANGLE);
	#endif

	#if ENABLE_WIFI == true
		wifiNetwork.setOnMessageCallback(OnWebSocketMessage);
		wifiNetwork.setup();
		businessLogic.setWiFiNetworkInstance(&wifiNetwork);
		businessLogic.setServoManagerInstance(pServoManager);
	#endif

	Serial.println("Setup complete.");
}

void loop() {
	#if ENABLE_WIFI == true
		wifiNetwork.loop();
	#endif

	bool ShouldBusinessLogicTick = businessLogic.should_loop_tick();

	if (ShouldBusinessLogicTick) {
		businessLogic.pre_sensor_read_loop();
	}

	ReadSensors();

	if (ShouldBusinessLogicTick) {
		businessLogic.pre_actuator_loop();
	}

	#if ENABLE_SERVO == true
		ServoManager_loop();
	#endif

	if (ShouldBusinessLogicTick) {
		businessLogic.pre_hardware_report_loop();
	}

	if (HardwareReportingTimer.shouldTick()) {
		Serial_StatusReport();

		#if ENABLE_LCD_OUTPUT == true
			LCD_StatusReport();
		#endif
	}
}

#if ENABLE_WIFI == true
void OnWebSocketMessage(const JsonDocument& doc) {
	// Handle incoming WebSocket messages

	if (doc["endpoint"] == "/iot/get_data") {
		// Foreach
		businessLogic.shouldEnableWaterPump = doc["data"]["shouldEnableWaterPump"].as<bool>();
		businessLogic.shouldDispenseFood = doc["data"]["shouldDispenseFood"].as<bool>();
		businessLogic.isWaitingForServerActuatorData = false;
		businessLogic.shouldPushOrPull = true; // Switch to push mode after receiving data
	}
	else if (doc["endpoint"] == "/iot/post_data") {
		businessLogic.isWaitingForServerReportACK = false;
		businessLogic.shouldPushOrPull = false; // Switch to pull mode after receiving data
	}
}
#endif

void Serial_StatusReport() {
	Serial.print("Temperature: ");
	Serial.print(businessLogic.temperature);
	Serial.println(" °C");

	Serial.print("Humidity: ");
	Serial.print(businessLogic.humidity);
	Serial.println(" %");

	Serial.print("Water Level: ");
	Serial.print(businessLogic.waterLevel);
	Serial.println(" %");

	#if ENABLE_WIFI == true
		Serial.print("WiFi Status: ");
		Serial.println(wifiNetwork.isConnected() ? "Connected" : "Disconnected");

		Serial.print("WebSocket Status: ");
		#if WS_RTX_ON == true
			Serial.println(wifiNetwork.isServerConnected() ? "Connected (RTX)" : "Disconnected");
		#else
			Serial.println(wifiNetwork.isServerConnected() ? "Connected (Normal)" : "Disconnected");
		#endif
	#endif

	Serial.print("Water Pump: ");
	Serial.println(businessLogic.waterPumpEnabled ? "ON" : "OFF");
}

#if ENABLE_LCD_OUTPUT == true

void LCD_StatusReport() {
	if (!isLCDInitialized)
		return;

	int TemperatureStrlen = snprintf(nullptr, 0, LCD_TEMP_FORMAT, businessLogic.temperature);
	char TemperatureStr[TemperatureStrlen + 1];
	snprintf(TemperatureStr, sizeof(TemperatureStr), LCD_TEMP_FORMAT, businessLogic.temperature);

	int HumidityStrlen = snprintf(nullptr, 0, LCD_HUMIDITY_FORMAT, int(businessLogic.humidity));
	char HumidityStr[HumidityStrlen + 1];
	snprintf(HumidityStr, sizeof(HumidityStr), LCD_HUMIDITY_FORMAT, int(businessLogic.humidity));

	int WaterLevelStrlen = snprintf(nullptr, 0, LCD_WATER_LEVEL_FORMAT, businessLogic.waterLevel);
	char WaterLevelStr[WaterLevelStrlen + 1];
	snprintf(WaterLevelStr, sizeof(WaterLevelStr), LCD_WATER_LEVEL_FORMAT, businessLogic.waterLevel);

	char firstLine[17];

	int MiddleBorder = (LCD_COLUMNS_SIZE - TemperatureStrlen - HumidityStrlen);

	// Put temperature on first line
	snprintf(firstLine, sizeof(firstLine), "%s", TemperatureStr);

	// Fill the middle with spaces
	for (int i = 0; i < MiddleBorder; i++) {
		firstLine[TemperatureStrlen + i] = ' ';
	}

	// Put humidity after temperature on first line
	strncpy(firstLine + TemperatureStrlen + MiddleBorder, HumidityStr, HumidityStrlen);

	lcd.setCursor(0, 0);
	lcd.print(firstLine);

	char secondLine[17];

	// Put water level
	snprintf(secondLine, sizeof(secondLine), "%s", WaterLevelStr);

	// Fill the rest of the line with spaces
	for (int i = WaterLevelStrlen; i < LCD_COLUMNS_SIZE; i++) {
		secondLine[i] = ' ';
	}
	secondLine[LCD_COLUMNS_SIZE] = '\0'; // Ensure null termination

	lcd.setCursor(0, 1);
	lcd.print(secondLine);
}

#endif