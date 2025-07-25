import { RouteHandler } from "../types/route";

const handler: RouteHandler = (client, db, session, data) => {
	if ("name" in data) {
		if (typeof data.name !== "string") {
			return {
				status: "error",
				code: 400,
				error_message: "Invalid name format. Expected a string."
			};
		}

		return {
			status: "success",
			code: 200,
			data: {
				message: `Hello, ${data.name}!`
			}
		}
	}

	return {
		status: "success",
		code: 200,
		data: {
			message: "Hello, world!"
		}
	}
}

export default handler;