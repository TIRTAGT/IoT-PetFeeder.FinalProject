#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PicoWebsocket.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include "config.h"

class WiFiNetwork {
	public:
		WiFiNetwork(const char* ssid, const char* password);
		void setup();
		void loop();
		bool isConnected();
		bool isServerConnected();
		bool isLoggedIn();
		bool sendJSON(JsonDocument& doc);
		bool sendJSON(JsonDocument& doc, bool bypass_login_check);
		void setOnMessageCallback(void (*callback)(const JsonDocument& doc));
		
	private:
		void tick();
		void handleWebSocket();

		void (*messageCallback)(const JsonDocument& doc);

		char* writeBuffer;
		size_t writeBufferSize = 0;

		const char* ssid;
		const char* password;
		WiFiClient wifiClient;
		PicoWebsocket::Client ws;

		bool hasLoggedIn;
		bool hasSentLoginRequest;

		#if ENABLE_LCD_OUTPUT == true
			LiquidCrystal_I2C lcd;
		#endif
};