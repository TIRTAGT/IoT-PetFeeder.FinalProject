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
			error_message: "You should send \"iot_hwid\" in the data"
		};
	}

	if (typeof data.iot_hwid !== "string") {
		return {
			status: "error",
			code: 400,
			error_message: "Invalid iot_hwid format, expected a string"
		};
	}

	let a = db.devices.get(data.iot_hwid);

	console.log(db.devices)

	// Check if this device exist on the db
	if (!a) {
		return {
			status: "error",
			code: 404,
			error_message: "This IoT device data is not found"
		};
	}

	return {
		status: "success",
		code: 200,
		data: a
	};
}

export default handler;