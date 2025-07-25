import { BaseSensorData } from "../../DeviceData";

export class WaterLevel implements BaseSensorData {
	kind: "sensor" = "sensor";
	type: "WaterLevel" = "WaterLevel";

	/** Water level in percentage */
	waterLevel: number = 0;
}