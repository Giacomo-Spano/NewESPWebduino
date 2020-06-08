#include "DoorSensor.h"
#include "Shield.h"

Logger DoorSensor::logger;
String DoorSensor::tag = "DoorSensor";

DoorSensor::DoorSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>DoorSensor::DoorSensor"));

	type = "doorsensor";
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<DoorSensor::DoorSensor\n"));
}

DoorSensor::~DoorSensor()
{
}

void DoorSensor::init()
{
	logger.print(tag, "\n\t >>init DoorSensor pin=" + String(pin));
	pinMode(pin, INPUT);
	mode = MODE_NORMAL;
	logger.print(tag, F("\n\t <<init DoorSensor"));
}

void DoorSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["mode"] = mode;
	//json.printTo(Serial);
}

bool DoorSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		logger.print(tag, "\nDoorSensor::checkStatusChange\n");
		lastCheckStatus = currMillis;
				
		if (mode.equals(MODE_NORMAL)) {
			if (digitalRead(pin) == LOW) {
				logger.print(tag, "\nOPENn pin=" + String(pin));
				setStatus(STATUS_DOOROPEN);
				ret = Sensor::checkStatusChange();
			}
			else {
				logger.print(tag, "\nCLOSED pin=" + String(pin));
				setStatus(STATUS_DOORCLOSED);
				ret = Sensor::checkStatusChange();
			}
		}
		else if (mode.equals(MODE_TEST)) {
			ret = Sensor::checkStatusChange();
		}
	}
	return ret;
}

bool DoorSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>DoorSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("testmode")) {
		if (payload.equals("on")) {
			mode = MODE_TEST;
		}
		else if (payload.equals("off")) {
			mode = MODE_NORMAL;
		}
	}
	else if (command.equals("settestdoorstatus")) {
		if (mode.equals(MODE_TEST)) {
			if (payload.equals("open")) {
				setStatus(STATUS_DOOROPEN);
			}
			else if (payload.equals("closed")) {
				setStatus(STATUS_DOORCLOSED);
			}
		}
	}
	logger.print(tag, F("\n\t <<DoorSensor::sendCommand"));
	return false;
}
