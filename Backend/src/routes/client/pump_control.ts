import { WaterPump } from "../../types/iot";
import { RouteHandler } from "../../types/route";

const handler: RouteHandler = (client, db, session, data) =>{
	if (typeof session.auth_data === "undefined") {
		return {
			status: "error",
			code: 401,
			error_message: "You must be authenticated to post data"
		};
	}

	// If client isn't iot device, return error
	if (session.auth_data.kind !== "client") {
		return {
			status: "error",
			code: 403,
			error_message: "This endpoint is only accessible by client devices"
		};
	}

	if (typeof data !== "object") {
		return {
			status: "error",
			code: 400,
			error_message: "You should send \"iot_hwid\" in the data",
		};
	}

	if (typeof data.iot_hwid !== "string") {
		return {
			status: "error",
			code: 400,
			error_message: "Invalid iot_hwid format, expected a string",
		};
	}

	let a = db.devices.get(data.iot_hwid);

	// Check if this device exist on the db
	if (!a) {
		return {
			status: "error",
			code: 404,
			error_message: "This IoT device data is not found"
		};
	}

	if (typeof data.enable !== "boolean") {
		return {
			status: "error",
			code: 400,
			error_message: "Invalid iot_hwid format, expected a string",
		};
	}

	const servo = a.live.find((item) => item.type === "WaterPump") as WaterPump | undefined;
	if (!servo) {
		return {
			status: "error",
			code: 404,
			error_message: "Servo not found in the device data",
		};
	}

	servo.triggerEnableWaterPump = data.enable;

	db.devices.set(data.iot_hwid, a);
}

export default handler;