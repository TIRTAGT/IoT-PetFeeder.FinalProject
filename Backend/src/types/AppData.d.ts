import { DeviceData } from "./iot/DeviceData";

export type AppData = {
	version: string;

	devices: Map<string, DeviceData>;
};