import { RouteHandler } from "./types/route";
import hello from "./routes/hello";
import login from "./routes/login";
import iot_get_data from "./routes/iot/get_data";
import iot_post_state_data from "./routes/iot/post_state_data";
import client_get_data from "./routes/client/get_data";
import client_food_control from "./routes/client/food_control";
import client_pump_control from "./routes/client/pump_control";

const RouteMap = new Map<string, RouteHandler>();

RouteMap.set("/hello", hello);
RouteMap.set("/login", login);
RouteMap.set("/iot/get_data", iot_get_data);
RouteMap.set("/iot/post_data", iot_post_state_data);
RouteMap.set("/client/get_data", client_get_data);
RouteMap.set("/client/food_control", client_food_control);
RouteMap.set("/client/pump_control", client_pump_control);

export default RouteMap;