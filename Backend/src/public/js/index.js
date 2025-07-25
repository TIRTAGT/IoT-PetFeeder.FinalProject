document.addEventListener("DOMContentLoaded", () => {
	const ESP_HWID = "petfeeder-esp32dev-dimas";

	const ConnectionStatusElement = document.getElementById("connection_status");
	const TemperatureText = document.getElementById("temperature");
	const HumidityText = document.getElementById("humidity");
	const WaterLevelText = document.getElementById("waterLevel");

	const DebugConsoleInput = document.getElementById("command_input");
	const DebugConsoleSubmitButton = document.getElementById("send_command");
	const DebugConsoleOutput = document.getElementById("console_output");

	if (!window.location.hostname) {
		ConnectionStatusElement.innerText = "Server Auto-Detect Failed";
		return;
	}

	const ESP_EmulatorMode = window.location.search.includes("login=esp");
	const MANUAL_DEBUG_MODE = window.location.search.includes("debug");

	if (MANUAL_DEBUG_MODE) {
		document.getElementById("debug_console").style.display = "block";
		DebugConsoleOutput.innerText = "> Debug mode enabled. Commands will be sent directly to the server.\n";
	}

	const ws = new WebSocket(`ws://${window.location.hostname}:8080/ws`);

	ConnectionStatusElement.innerText = "Connecting...";
	ConnectionStatusElement.classList.remove("text_red");
	ConnectionStatusElement.classList.add("text_yellow");

	function SendData(json_object, bypass_json_encode = false) {
		if (ws.readyState !== WebSocket.OPEN) {
			console.warn("WebSocket is not open. Cannot send data.");
			return;
		}

		try {
			if (typeof json_object !== "string" && !bypass_json_encode) {
				json_object = JSON.stringify(json_object);
			}

			ws.send(json_object);
		}
		catch (error) {
			console.error("Failed to send data:", error);

			DebugConsoleOutput.innerText += `! Error sending data: ${error.message}\n`;
		}
	}

	ws.onopen = () => {
		ConnectionStatusElement.innerText = "Connected, logging in...";

		if (ESP_EmulatorMode) {
			SendData({ "key": "/login", "data" : {"kind": "iot", "iot_hwid": ESP_HWID } });
			return;
		}
		SendData({ "key": "/login", "data" : {"kind": "client" }});
	};

	document.getElementById("enable_servo").addEventListener("click", () => {
		let a = { "key": "/client/food_control", "data" : {"iot_hwid": ESP_HWID, "enable": true } };
		SendData(a);
	});

	document.getElementById("enable_pump").addEventListener("click", () => {
		let a = { "key": "/client/pump_control", "data" : {"iot_hwid": ESP_HWID, "enable": true } };
		SendData(a);
	});

	DebugConsoleInput.addEventListener("keydown", (event) => {
		if (event.key !== "Enter") {
			return;
		}

		DebugConsoleSubmitButton.click();
		event.preventDefault();
	});

	DebugConsoleSubmitButton.addEventListener("click", () => {
		let command = DebugConsoleInput.value.trim();
		if (!command) return;

		DebugConsoleOutput.innerText += `> ${command}\n`;
		DebugConsoleInput.value = '';

		SendData(command);
	});

	ws.onmessage = (event) => {
		let text = event.data.trim();
		if (!text) return;

		let data = JSON.parse(text);

		// Ensure has "code" as number
		if (typeof data.code !== "number") {
			throw new Error("Invalid response format: \"code\" is not a number");
		}

		// Ensure status is either "success" or "error"
		if (data.status !== "success" && data.status !== "error") {
			throw new Error("Invalid response format: \"status\" is not \"success\" or \"error\"");
		}

		// Ensure has "endpoint" as string
		if (typeof data.endpoint !== "string") {
			throw new Error("Invalid response format: \"endpoint\" is not a string");
		}

		if (MANUAL_DEBUG_MODE) {
			DebugConsoleOutput.innerText += `< ${text}\n`;
		}

		// If data.endpoint is "/login"
		if (data.endpoint === "/login") {
			if (data.status === "error" && data.error_message !== "You are already logged in") {
				ConnectionStatusElement.innerText = "Login failed: " + data.error_message;
				ConnectionStatusElement.classList.remove("text_yellow");
				ConnectionStatusElement.classList.add("text_red");
				console.error("Login error:", data);
				return;
			}

			ConnectionStatusElement.innerText = "Connected (RTX)";
			ConnectionStatusElement.classList.remove("text_yellow");
			ConnectionStatusElement.classList.add("text_green");
			return;
		}
		
		onResponse(data);
	};

	ws.onclose = (ev) => {
		console.log(`Connection closed, code: ${ev.code}, reason: ${ev.reason}`);
		ConnectionStatusElement.innerText = "Disconnected";
		ConnectionStatusElement.classList.remove("text_green");
		ConnectionStatusElement.classList.remove("text_yellow");
		ConnectionStatusElement.classList.add("text_red");

		DebugConsoleOutput.innerText += `> Connection closed, code: ${ev.code}, reason: ${ev.reason}\n`;
	};

	ws.onerror = (error) => {
		console.log(`Error: ${error.message}`);

		DebugConsoleOutput.innerText += `> Error: ${error.message}\n`;
		ConnectionStatusElement.innerText = "Error: " + error.message;
		ConnectionStatusElement.classList.remove("text_green");
		ConnectionStatusElement.classList.remove("text_yellow");
		ConnectionStatusElement.classList.add("text_red");
	};

	function onResponse(response) {
		if (response.status === "error") {
			DebugConsoleOutput.innerText += `! Error: ${response.error_message}\n`;
			return;
		}

		if (response.endpoint === "/client/get_data" && response.data.live) {
			for (let key in response.data.live) {
				let detail = response.data.live[key];

				// If type "DHT"
				if (detail.type === "DHT") {
					TemperatureText.innerText = detail.temperature;
					HumidityText.innerText = detail.humidity;
					continue;
				}

				// If type "WaterLevel"
				if (detail.type === "WaterLevel") {
					WaterLevelText.innerText = detail.waterLevel;
					continue;
				}

				if (detail.type === "WaterPump") {
					let statusText = detail.powered_on ? "On" : "Off";
					document.getElementById("waterPumpStatus").innerText = statusText;
					continue;
				}

				if (detail.type === "Servo") {
					let statusText = detail.isDispensing ? "Open" : "Closed";
					document.getElementById("foodServoStatus").innerText = statusText;
					continue;
				}
			}

			return;
		}
		console.log(response.data);
	}

	function PeriodicRefresh() {
		if (ws.readyState !== WebSocket.OPEN) return;

		if (MANUAL_DEBUG_MODE) { return; }

		if (ESP_EmulatorMode) {
			SendData(JSON.stringify({ 
				"key": "/iot/post_data",
				"data": {
					"te": 26.5,
					"hu": 70,
					"wa": 0,
					"PuEn": false,
					"shPuEn": false,
					"DiFo": false,
					"shDiFo": false
				}
			}));
			return;
		}

		SendData(JSON.stringify({ "key": "/client/get_data", "data" : {"iot_hwid": ESP_HWID } }));
	}

	setInterval(() => { PeriodicRefresh(); }, 200);
});