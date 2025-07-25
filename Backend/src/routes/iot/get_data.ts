import { FoodServo, WaterPump } from "../../types/iot";
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
	if (session.auth_data.kind !== "iot") {
		return {
			status: "error",
			code: 403,
			error_message: "This endpoint is only accessible by IoT devices"
		};
	}

	// If iot_hwid is not present in session, return error
	if (!session.auth_data.iot_hwid) {
		return {
			status: "error",
			code: 403,
			error_message: "IoT device HWID is not set in session"
		};
	}

	// If data is empty object
	if (Object.keys(data).length !== 0) {
		return {
			status: "error",
			code: 400,
			error_message: "You should not send any data to this endpoint"
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

	const ResultData = {
		shouldEnableWaterPump: false,
		shouldDispenseFood: false
	};

	let WaterPump = a.live.find((d) => d.type === "WaterPump") as WaterPump | undefined;
	if (WaterPump) {
		ResultData.shouldEnableWaterPump = WaterPump.triggerEnableWaterPump;
		WaterPump.triggerEnableWaterPump = false; // Reset the trigger after getting the data
	}

	let Servo = a.live.find((d) => d.type === "Servo") as FoodServo | undefined;
	if (Servo) {
		ResultData.shouldDispenseFood = Servo.triggerDispenseFood;
		Servo.triggerDispenseFood = false; // Reset the trigger after getting the data
	}

	return {
		status: "success",
		code: 200,
		data: ResultData
	};
}

export default handler;