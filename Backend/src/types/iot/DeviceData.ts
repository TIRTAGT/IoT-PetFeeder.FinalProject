export class DeviceData {
	hwid: string;
	name: string;
	online: boolean;
	last_seen: number;

	history: TimeBasedData[];
	live: BaseDeviceData[];

	constructor(hwid: string, name?: string) {
		this.hwid = hwid;
		this.name = name || hwid;
		this.online = true;
		this.last_seen = Date.now();
		this.history = [];
		this.live = [];
	}
}

export type TimeBasedData = {
	timestamp: number;
	data: BaseDeviceData[];
}

export type BaseDeviceData = {
	kind: "actuator" | "sensor";
	type: string;
}

export type BaseActuatorData = BaseDeviceData & {
	kind: "actuator";
}

export type BaseSensorData = BaseDeviceData & {
	kind: "sensor";
}