import * as http from "http";
import * as FileSystem from "fs";
import { WebSocketServer, WebSocket } from "ws";
import { Config } from "./config";
import { ClientSession } from "./types/client_session";
import Routes from "./routes";
import { RouteHandler, RouteHandlerReturnType } from "./types/route";
import { CurrentLogTimestamp } from "./utility";

class PetFeederBackend {
	private PORT: number | undefined = undefined;
	private HTTPServer: http.Server;
	private WebSocketServer: WebSocketServer;
	private pingInterval: NodeJS.Timeout | null = null;

	private db: Config = new Config();

	/** A session mapping, key is the IP+port of the client */
	private WebSocketClientSessions: Map<string, ClientSession> = new Map();

	constructor() {
		// If process.env.PORT is set, try parsing it as a number
		let EnvPort = parseInt(process.env.PORT || '', 10);
		this.PORT = isNaN(EnvPort) ? 8080 : EnvPort;

		this.HTTPServer = http.createServer();
		this.HTTPServer.on("listening", () => {
			console.log(`[${CurrentLogTimestamp()}] HTTP Server started on *:${this.PORT}`);
		});
		this.HTTPServer.on("request", (req, res) => {
			// If WebSocket request, let the WebSocket server handel it
			if (req.headers.upgrade) {
				return;
			}

			// Read a file in the public directory
			let url = req.url || "/";

			// Split any query string from the URL
			url = url.split("?")[0];

			// Guard there must no double dot (..) in the URL to avoid directory traversal attacks
			if (url.includes("..")) {
				res.writeHead(400, { "Content-Type": "text/plain" });
				res.end("400 Bad Request");
				return;
			}

			const CurrentDirectory = process.cwd();
			let filePath = `${CurrentDirectory}/build/public`;
			try {
				filePath = FileSystem.realpathSync(`${filePath}/${url === "/" ? "/index.html" : url}`);
			}
			catch (error: any) {
				// If error is just ENOENT, it means the file does not exist
				if (error.code === "ENOENT") {
					res.writeHead(404, { "Content-Type": "text/plain" });
					res.end("404 Not Found");
					return;
				}
				console.error("Error resolving file path:", error);
				res.writeHead(500, { "Content-Type": "text/plain" });
				res.end("500 Internal Server Error");
				return;
			}

			// If filePath is outside of the current directory, return 403 Forbidden
			if (!filePath.startsWith(CurrentDirectory)) {
				res.writeHead(403, { "Content-Type": "text/plain" });
				res.end("403 Forbidden");
				return;
			}
			
			// Check if the file exists
			if (!FileSystem.existsSync(filePath)) {
				res.writeHead(404, { "Content-Type": "text/plain" });
				res.end("404 Not Found");
				return;
			}

			// Read the file and send it
			FileSystem.readFile(filePath, (err, data) => {
				if (err) {
					console.error("Error reading file:", err);
					res.writeHead(500, { "Content-Type": "text/plain" });
					res.end("500 Internal Server Error");
					return;
				}

				// Set the content type based on the file extension
				let contentType = "text/html";
				if (filePath.endsWith(".js")) {
					contentType = "application/javascript";
				} else if (filePath.endsWith(".css")) {
					contentType = "text/css";
				} else if (filePath.endsWith(".png")) {
					contentType = "image/png";
				} else if (filePath.endsWith(".jpg") || filePath.endsWith(".jpeg")) {
					contentType = "image/jpeg";
				} else if (filePath.endsWith(".svg")) {
					contentType = "image/svg+xml";
				}

				res.writeHead(200, { "Content-Type": contentType });
				res.end(data);
			});
		});
		this.WebSocketServer = new WebSocketServer({ server: this.HTTPServer});

		this.WebSocketServer.on("listening", () => {
			console.log(`[${CurrentLogTimestamp()}] WebSocket server is listening on port *:${this.PORT}`);
		});

		this.WebSocketServer.on("connection", (ws, request) => {
			const ClientAddr = request.socket.remoteAddress;
			
			if (typeof ClientAddr !== "string") {
				console.error("Failed to get remote address from request");
				ws.close(1003, "Invalid remote address");
				return;
			}

			const ClientPort = request.socket.remotePort;
			if (typeof ClientPort !== "number") {
				console.error("Failed to get remote port from request");
				ws.close(1003, "Invalid remote port");
				return;
			}

			const SessionKey = `${ClientAddr}:${ClientPort}`;

			if (this.WebSocketClientSessions.has(SessionKey)) {
				console.error("Client session already exists:", SessionKey);
				ws.close(1003, "Client session already exists");
				return;
			}

			this.WebSocketClientSessions.set(SessionKey, {
				socket: ws,
				public_ip: ClientAddr,
				port: ClientPort
			});

			console.log(`[${CurrentLogTimestamp()}] [${ClientAddr}:${ClientPort}] New client connected`);

			ws.on("error", (error) => {
				console.error("WebSocket error:", error);

				if (this.WebSocketClientSessions.has(SessionKey))
					this.WebSocketClientSessions.delete(SessionKey);
			});

			ws.on("close", () => {
				console.log(`[${CurrentLogTimestamp()}] [${ClientAddr}:${ClientPort}] Client disconnected`);

				if (this.WebSocketClientSessions.has(SessionKey)) 
					this.WebSocketClientSessions.delete(SessionKey);
			});

			ws.on("message", (message, isBinary) => {
				if (!(message instanceof Buffer)) {
					console.error("Received non-buffer message:", message);
					ws.close(1003, "Invalid message type, client must send binary encoded-string");
					return;
				}

				// Convert the buffer to string
				const messageString = message.toString("utf8");
				if (!messageString) {
					console.error("Received empty message");
					ws.close(1003, "Empty message received");
					return;
				}

				// JSON parse the message
				let data: any;
				try {
					data = JSON.parse(messageString);
				}
				catch (error) {
					ws.close(1003, "Invalid JSON format");
					return;
				}

				// Ensure the data has key and value properties
				if (typeof data !== "object") {
					console.error("Received data is not an object:", data);
					ws.close(1003, "Data must be an valid JSON-encoded object");
					return;
				}

				if (!data.hasOwnProperty("key") || typeof data["key"] !== "string") {
					console.error("Received data does not have a valid key:", data);
					ws.close(1003, "Data must contain a valid key (string) property");
					return;
				}

				const key = data["key"];
				let route_data: any = {};

				if (data.hasOwnProperty("data") && typeof data["data"] === "object") {
					// If data has a "data" property, use it
					route_data = data["data"];
				}

				const session = this.WebSocketClientSessions.get(SessionKey);
				if (!session) {
					console.error("No session found for client:", SessionKey);
					ws.close(1003, "Session not found");
					return;
				}

				this.onClientMessage(ws, session, key, route_data);
			});
		});

		this.WebSocketServer.on("error", (error) => {
			console.error("WebSocket server error:", error);
		});

		this.WebSocketServer.on("wsClientError", (error) => {
			console.error("WebSocket client error:", error);
		});

		this.WebSocketServer.on("close", () => {
			console.log("WebSocket server closed");
			if (this.pingInterval) {
				clearInterval(this.pingInterval);
			}
		});

		this.HTTPServer.listen(this.PORT, "0.0.0.0");

		this.pingInterval = setInterval(() => {
			this.PingClients();
		}, 3000); // Ping every 30 seconds
	}

	private PingClients() {
		this.WebSocketServer.clients.forEach((client: WebSocket) => {
			if (client.readyState === WebSocket.OPEN) {
				client.ping();
			}
		});
	}

	private onClientMessage(ws: WebSocket, session: ClientSession, key: string, data: any) {
		let response: RouteHandlerReturnType;

		console.log(`[${CurrentLogTimestamp()}] [${session.public_ip}:${session.port}] REQUEST: ${key}`);

		// Get the route function
		const routeFunction = Routes.get(key);

		if (routeFunction) {
			if (typeof routeFunction === "function") {
				try {
					response = this.ProcessRequest(key, routeFunction, ws, session, data);
				}
				catch (error) {
					console.error("Error processing route:", error);

					response = {
						status: "error",
						code: 500,
						error_message: "An unexpected error occurred while processing the request"
					};
				}
			}
			else {
				response = {
					status: "error",
					code: 500,
					error_message: `Route "${key}" is not a valid function`
				};
			}
		}
		else {
			response = {
				status: "error",
				code: 404,
				error_message: `Route "${key}" not found`
			};
		}

		if (typeof response === "undefined") {
			response = {
				status: "error",
				code: 500,
				error_message: "Route handler did not return a valid response"
			};
		}

		let EncodedResponse = JSON.stringify({ ...response, endpoint: key });
		ws.send(EncodedResponse);
		ws.send("\r"); // The used IoT library waits a while for "\r", this avoids the unnecessary delay

		console.log(`[${CurrentLogTimestamp()}] [${session.public_ip}:${session.port}] RESPONSE: ${EncodedResponse}`);
	}

	private ProcessRequest(handler_name: string, handler: RouteHandler, client: WebSocket, session: ClientSession, data: object): Exclude<RouteHandlerReturnType, undefined> {
		let RouteResponse = handler(client, this.db.data.app_data, session, data);

		// If no response, return a 204 No Content response
		if (RouteResponse === undefined) {
			return {
				status: "success",
				code: 204,
				data: {}
			};
		}

		if (typeof RouteResponse !== "object") {
			return {
				status: "error",
				code: 500,
				error_message: "Route endpoint did not return a valid object"
			};
		}

		if (RouteResponse.status === "success") {
			if (RouteResponse.code < 200 || RouteResponse.code >= 300) {
				console.warn(`Route "${handler_name}" returned a success status with a non-2xx code: ${RouteResponse.code}`);
			}
		}
		else if (RouteResponse.status === "error") {
			if (RouteResponse.code < 400 || RouteResponse.code >= 600) {
				console.warn(`Route "${handler_name}" returned an error status with a non-4xx/5xx code: ${RouteResponse.code}`);
			}
		}

		return RouteResponse;
	}
}

const petFeederBackend = new PetFeederBackend();