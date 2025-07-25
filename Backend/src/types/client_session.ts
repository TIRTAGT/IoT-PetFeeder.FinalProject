import { WebSocket } from "ws";

export type ClientSession = {
	socket: WebSocket;
	public_ip: string;
	port: number;

	auth_data?: {
		kind: "client" | "iot";
		
		iot_hwid?: string;
	}
};