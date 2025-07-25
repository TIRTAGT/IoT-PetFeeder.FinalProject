#include "WiFiNetwork.h"
#include "TickTimer.h"
#include <lwip/sockets.h>

TickTimer logicTimer(3000000); // 3 seconds initial delay
TickTimer DisconnectedAnimationTimer(200000); // 200 ms
bool isWiFiBeginCalled = false;
bool isWiFiConnectedLastStatus = false;

WiFiNetwork::WiFiNetwork(const char* ssid, const char* password) : ws(wifiClient) {
	this->ssid = ssid;
	this->password = password;
	this->wifiClient = WiFiClient();
	this->messageCallback = nullptr;
	this->writeBuffer = nullptr;
	this->hasLoggedIn = false;
	this->hasSentLoginRequest = false;
}

void WiFiNetwork::setup() {
	if (isWiFiBeginCalled) {
		return;
	}

	isWiFiBeginCalled = true;
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(false);
	WiFi.setAutoReconnect(true);
	WiFi.begin(ssid, password);
	WiFi.setSleep(WIFI_PS_NONE); // Disable WiFi power save mode
}

void WiFiNetwork::loop() {
	if (WiFi.status() == WL_CONNECTED) {
		if (!isWiFiConnectedLastStatus) {
			#if ENABLE_LCD_OUTPUT == true
				lcd.clear();
			#endif

			digitalWrite(2, LOW); // Turn off the LED_BUILTIN
			Serial.println();
			Serial.println("Connected to WiFi");
			Serial.print("IP Address: ");
			Serial.println(WiFi.localIP());
		}
		isWiFiConnectedLastStatus = true;

		if (logicTimer.shouldTick()) {
			tick();
		}

		return;
	}

	if (!DisconnectedAnimationTimer.shouldTick())
		return;

	#if ENABLE_LCD_OUTPUT == true
		this->lcd.clear();
		this->lcd.setCursor(0, 0);
	#endif

	int WIFI_CONNECT_DOT_ANIM = 0;

	// Construct the WiFi connection text with dots before printing
	char wifiConnectText[LCD_COLUMNS_SIZE + 1];

	// Generate repeated dots string
	char dots[WIFI_CONNECT_DOT_MAX + 1];
	for (int i = 0; i < WIFI_CONNECT_DOT_ANIM; ++i) {
		dots[i] = '.';
	}
	// Fill the rest with spaces
	for (int i = WIFI_CONNECT_DOT_ANIM; i < WIFI_CONNECT_DOT_MAX; ++i) {
		dots[i] = ' ';
	}
	dots[WIFI_CONNECT_DOT_MAX + 1] = '\0';

	snprintf(
		wifiConnectText, 
		sizeof(wifiConnectText),
		"%s%s",
		WIFI_CONNECT_TEXT,
		dots);

	#if ENABLE_LCD_OUTPUT == true
		this->lcd.setCursor(0, 0);
		this->lcd.print(wifiConnectText);
	#endif

	Serial.print("\r");
	Serial.print(wifiConnectText);
	Serial.flush();

	digitalWrite(2, !digitalRead(2)); // D2 is LED_BUILTIN on ESP32
	delay(200);

	WIFI_CONNECT_DOT_ANIM++;

	if (WIFI_CONNECT_DOT_ANIM > WIFI_CONNECT_DOT_MAX) {
		WIFI_CONNECT_DOT_ANIM = 0;
	}
}

void WiFiNetwork::tick() {
	if (!this->ws.connected()) {
		logicTimer.setTickMicros(5000000); // 5 seconds
		if (this->hasLoggedIn) {
			Serial.println("WebSocket disconnected, attempting to reconnect...");
			this->hasLoggedIn = false;
			this->hasSentLoginRequest = false;
			this->ws.stop();
			this->ws.~Client(); // Explicitly call the destructor to clean up the old instance
			new (&this->ws) PicoWebsocket::Client(wifiClient);
			return;
		}
		
		Serial.println("Connecting to WebSocket server...");

		if (!ws.connect(WS_SERVER_ADDRESS, WS_SERVER_PORT)) {
			Serial.println("Failed to connect WebSocket.");
			return;
		}
		
		#if WS_RTX_ON == true
			this->wifiClient.setNoDelay(true); // Disable Nagle's algorithm for low latency
			int keepAlive = 1;
			this->wifiClient.setSocketOption(SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive)); // Enable TCP keepalive
		#endif
		return;
	}

	logicTimer.setTickMicros(200000); // Reset to 200 ms
	handleWebSocket();
}

void WiFiNetwork::handleWebSocket() {
	while (this->ws.available()) {
		String message = this->ws.readStringUntil('\r');

		if (message.length() == 0) {
			break;
		}

		digitalWrite(2, LOW); // LED_BUILTIN on ESP32

		// Parse JSON
		JsonDocument doc;
		DeserializationError error = deserializeJson(doc, message);
		if (error) {
			Serial.print("Failed to parse JSON: ");
			Serial.println(error.c_str());
			this->ws.stop();
			return;
		}

		if (doc["status"].isNull() || doc["code"].isNull() || doc["endpoint"].isNull()) {
			Serial.println("Received message without status, code, or endpoint fields.");
			this->ws.stop();
			return;
		}

		const char* status = doc["status"].as<const char*>();
		int code = doc["code"].as<int>();
		const char* endpoint = doc["endpoint"].as<const char*>();

		if (strcmp(status, "error") == 0) {
			Serial.print("Error (code ");
			Serial.print(code);
			Serial.print("): ");
			if (!doc["error_message"].isNull()) {
				Serial.print(doc["error_message"].as<String>());
			}
			Serial.println();

			this->ws.stop();
			return;
		}

		// If status isn't "success", something is wrong on the server side
		if (strcmp(status, "success") != 0) {
			Serial.print("Unexpected status received: ");
			Serial.println(status);
			this->ws.stop();
			return;
		}

		// If code is not on the 2xx range, treat it as an error
		if (code < 200 || code >= 300) {
			Serial.print("Unsupported code received: ");
			Serial.println(code);
			this->ws.stop();
			return;
		}
		
		// Check if doc["data"] exists
		if (!doc["data"].is<JsonObject>()) {
			Serial.println("No data field in the message.");
			return;
		}

		// If this is a login reply, set hasLoggedIn to true
		if (strcmp(endpoint, "/login") == 0) {
			this->hasLoggedIn = true;
			Serial.println("Logged in successfully.");
			return;
		}

		// If this isn't a login reply but we haven't logged in yet
		if (!this->hasLoggedIn) {
			Serial.println("Received message before logging in");
			this->ws.stop();
			return;
		}

		// If a message callback is set, call it with the received JSON document
		if (this->messageCallback == nullptr) {
			Serial.println("No message callback set, ignoring message.");
			return;
		}

		this->messageCallback(doc);
	}

	// If we haven't logged in yet, send the login message
	if (!this->hasLoggedIn && !this->hasSentLoginRequest) {
		JsonDocument doc;
		doc["key"] = "/login";
		doc["data"]["kind"] = "iot";
		doc["data"]["iot_hwid"] = WS_SERVER_HW_ID;
		
		this->sendJSON(doc, true); // Bypass login check to send login request
		this->hasSentLoginRequest = true;
	}

	// Copy the writeBuffer so we can empty it immediately
	char* bufferToSend = this->writeBuffer;
	size_t bufferToSendSize = this->writeBufferSize;
	if (bufferToSend == nullptr) {
		return;
	}

	this->writeBuffer = nullptr; // Clear the writeBuffer to prevent re-sending
	this->writeBufferSize = 0;

	size_t bytesWritten = this->ws.write((const uint8_t*)bufferToSend, bufferToSendSize);
	if (bytesWritten > 0) {
		// Serial.println("Sent JSON data to WebSocket server.");
	}
	else {
		Serial.println("Failed to send JSON data to WebSocket server.");
	}

	delete[] bufferToSend;
}

bool WiFiNetwork::isConnected() {
	return WiFi.status() == WL_CONNECTED;
}

bool WiFiNetwork::isServerConnected() {
	return ws.connected();
}

bool WiFiNetwork::isLoggedIn() {
	return this->hasLoggedIn;
}

bool WiFiNetwork::sendJSON(JsonDocument& doc) {
	return this->sendJSON(doc, false);
}

bool WiFiNetwork::sendJSON(JsonDocument& doc, bool bypass_login_check) {
	if (!this->isServerConnected()) {
		Serial.println("WebSocket is not connected, cannot send JSON.");
		return false;
	}

	// If we aren't logged in yet, we cannot send JSON
	if (!this->hasLoggedIn && !bypass_login_check) {
		return false;
	}

	// If writeBuffer is not yet clear, wait
	if (this->writeBuffer != nullptr) {
		Serial.println("Write buffer is not empty, unable to send new JSON yet.");
		return false;
	}

	String jsonString;
	serializeJson(doc, jsonString);

	this->writeBuffer = new char[jsonString.length() + 1];
	strcpy(this->writeBuffer, jsonString.c_str());
	this->writeBufferSize = jsonString.length();

	digitalWrite(2, HIGH); // LED_BUILTIN on ESP32
	return true;
}

void WiFiNetwork::setOnMessageCallback(void (*callback)(const JsonDocument& doc)) {
	this->messageCallback = callback;
}