import { BaseActuatorData } from "../../DeviceData";

export class WaterPump implements BaseActuatorData {
	kind: "actuator" = "actuator";
	type: "WaterPump" = "WaterPump";
	powered_on: boolean;

	/** Whether the water pump should be enabled by remote trigger */
	triggerEnableWaterPump: boolean = false;

	constructor(powered_on?: boolean) {
		this.powered_on = powered_on || false;
	}
}