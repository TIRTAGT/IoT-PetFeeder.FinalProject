import { BaseActuatorData } from "../../DeviceData";

export class FoodServo implements BaseActuatorData {
	kind: "actuator" = "actuator";
	type: "Servo" = "Servo";
	isDispensing: boolean;

	triggerDispenseFood: boolean = false;

	constructor(isDispensing?: boolean) {
		this.isDispensing = isDispensing || false;
	}
}