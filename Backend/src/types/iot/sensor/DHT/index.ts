import { BaseSensorData } from "../../DeviceData";

export class DHT implements BaseSensorData {
	kind: "sensor" = "sensor";
	type: "DHT" = "DHT";

	/** Temperature in Celsius */
	temperature: number = 0.00;

	/** Relative humidity in percentage */
	humidity: number = 0;
}