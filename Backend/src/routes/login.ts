import { RouteHandler } from "../types/route";
import * as IoT_Types from "../types/iot";

const handler: RouteHandler = (client, db, session, data) =>{
	if (typeof session.auth_data !== "undefined") {
		return {
			status: "error",
			code: 400,
			error_message: "You are already logged in"
		};
	}

	// If data doesn't contain kind
	if (!("kind" in data)) {
		return {
			status: "error",
			code: 400,
			error_message: "Missing authentication kind"
		};
	}

	// Kind must be a string of either "client" | "iot"
	if (data.kind !== "client" && data.kind !== "iot") {
		return {
			status: "error",
			code: 400,
			error_message: "Invalid authentication kind, expected \"client\" or \"iot\""
		};
	}

	// If kind is iot, ensure iot_hwid is present
	if (data.kind === "iot" && !("iot_hwid" in data)) {
		return {
			status: "error",
			code: 400,
			error_message: "Missing iot_hwid for iot authentication"
		};
	}

	// If iot_hwid is present, ensure it is a string
	if ("iot_hwid" in data && typeof data.iot_hwid !== "string") {
		return {
			status: "error",
			code: 400,
			error_message: "Invalid iot_hwid format, expected a string"
		};
	}

	session.auth_data = {
		kind: data.kind
	};

	if ("iot_hwid" in data) {
		session.auth_data.iot_hwid = data.iot_hwid;
		
		// Create a database for the IoT device if it doesn't exist
		if (!db.devices.has(data.iot_hwid)) {
			db.devices.set(data.iot_hwid, new IoT_Types.DeviceData(data.iot_hwid));
		}

		return {
			status: "success",
			code: 200,
			data: {
				message: `Connected as IoT device with HWID: ${data.iot_hwid}`
			}
		}
	}

	return {
		status: "success",
		code: 200,
		data: {
			message: "Connected as client"
		}
	};
}

export default handler;