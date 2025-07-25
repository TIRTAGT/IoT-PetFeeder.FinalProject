import { AppData } from "./types/AppData"

export class Config {
	public data: {
		"config": {},
		"app_data": AppData
	};

	constructor() {
		this.data = {
			"config": {},
			"app_data": {
				version: "1.0.0",
				devices: new Map()
			}
		};
	}
}