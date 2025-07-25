import WebSocket from "ws";
import { ClientSession } from "./client_session";
import { AppData } from "./AppData";

export type RouteHandler = (
	client: WebSocket,
	db: AppData,
	session: ClientSession,
	data: { [K: string]: any }
) => RouteHandlerReturnType;

export type BaseReturn = {
	/** Regular HTTP status code */
	code: number;

	endpoint?: never; // This is automatically handled by the route handler
}

type SuccessReturn = BaseReturn & {
	status: "success";

	data: object;
}

type ErrorReturn = BaseReturn & {
	status: "error";

	error_message: string;
}

export type RouteHandlerReturnType = SuccessReturn | ErrorReturn | undefined;