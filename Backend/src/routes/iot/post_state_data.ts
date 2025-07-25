import * as IoT_Types from "../../types/iot";
import { RouteHandler } from "../../types/route";

const handler: RouteHandler = (client, db, session, data) => {
	if (typeof session.auth_data === "undefined") {
		return {
			status: "error",
			code: 401,
			error_message: "You must be authenticated to post data"
		};
	}

	// If iot_hwid is not present in session, return error
	if (!session.auth_data.iot_hwid) {
		return {
			status: "error",
			code: 403,
			error_message: "IoT device HWID is not set in session",
		};
	}

	if (typeof data !== "object") {
		return {
			status: "error",
			code: 400,
			error_message: "Invalid data format. Expected a valid JSON object"
		};
	}

	let a = db.devices.get(session.auth_data.iot_hwid);

	// Check if this device exist on the db
	if (!a) {
		return {
			status: "error",
			code: 404,
			error_message: "This IoT device data is not found"
		};
	}

	if (typeof data.te !== "undefined" && typeof data.hu !== "undefined") {
		let b = new IoT_Types.DHT();
		b.temperature = data.te;
		b.humidity = data.hu;

		a.live = pushOrMergeDeviceData(a.live, b);
	}

	if (typeof data.wa !== "undefined") {
		let b = new IoT_Types.WaterLevel();
		b.waterLevel = data.wa;

		a.live = pushOrMergeDeviceData(a.live, b);
	}

	if (typeof data.PuEn !== "undefined") {
		let b = new IoT_Types.WaterPump();
		b.powered_on = (data.PuEn > 0) ? true : false;

		a.live = pushOrMergeDeviceData(a.live, b);
	}

	if (typeof data.DiFo !== "undefined") {
		let b = new IoT_Types.FoodServo();
		b.isDispensing = (data.DiFo > 0) ? true : false;

		a.live = pushOrMergeDeviceData(a.live, b);
	}

	db.devices.set(session.auth_data.iot_hwid, a);

	return {
		code: 200,
		status: "success",
		data: {
			message: "Data received successfully"
		}
	};
}

export default handler;

function pushOrMergeDeviceData<T extends IoT_Types.BaseDeviceData>(collection: IoT_Types.BaseDeviceData[], newData: T): IoT_Types.BaseDeviceData[] {
	// Find whether the exact same pair of kind and type already exists
	const existingIndex = collection.findIndex(item => item.kind === newData.kind && item.type === newData.type);
	
	if (existingIndex === -1) {
		// If not found, push the new data
		collection.push(newData);
		return collection;
	}

	// If found, merge the existing data with the new data
	const existingData = collection[existingIndex];

	// For each key in newData, if it exists in the existing data, merge it
	for (const key in newData) {
		// If the key starts with "trigger", skip it
		if (key.startsWith("trigger")) {
			// If not already present, set default value to false
			if (!(key in existingData)) {
				(existingData as any)[key] = false;
			}
			continue;
		}

		(existingData as any)[key] = (newData as any)[key];
	}

	collection[existingIndex] = existingData;
	return collection;
}