#include "DoorSensor.h"
#include "Shield.h"

Logger DoorSensor::logger;
String DoorSensor::tag = "DoorSensor";

String  DoorSensor::STATUS_DOOROPEN = "dooropen";
String  DoorSensor::STATUS_DOORCLOSED = "doorclosed";

String  DoorSensor::MODE_NORMAL = "normal";
String  DoorSensor::MODE_TEST = "test";
String  DoorSensor::MODE_TESTOPEN = "testopen";


DoorSensor::DoorSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>DoorSensor::DoorSensor"));
	
	type = "doorsensor";
	
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<DoorSensor::DoorSensor\n"));
}

void DoorSensor::init()
{
	logger.print(tag, "\n\t >>init DoorSensor pin=" + String(pin));
	Sensor::init();

	pinMode(pin, INPUT);
	mode = MODE_NORMAL;
	logger.print(tag, F("\n\t <<init DoorSensor"));
}

void DoorSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["mode"] = mode;
}

bool DoorSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		lastCheckStatus = currMillis;
				
		if (mode.equals(MODE_NORMAL)) {
			if (digitalRead(pin) == LOW) {
				setStatus(STATUS_DOOROPEN);
				ret = Sensor::checkStatusChange();
			}
			else {
				setStatus(STATUS_DOORCLOSED);
				ret = Sensor::checkStatusChange();
			}
		}
		else if (mode.equals(MODE_TEST)) {
			ret = Sensor::checkStatusChange();
		}
		else if (mode.equals(MODE_TESTOPEN)) {			
			unsigned long curr = millis();
			if (curr - lastTestModeTime > TestModeTimeout || curr - lastTestModeTime < 0) {
				setStatus(STATUS_DOORCLOSED);
				ret = Sensor::checkStatusChange();
				mode = MODE_NORMAL;
			}
			else {
				ret = Sensor::checkStatusChange();
			}
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
	else if (command.equals("test")) {
		mode = MODE_TESTOPEN;
		lastTestModeTime = millis();
		if (payload.equals("open")) {
			setStatus(STATUS_DOOROPEN);
		} else if (payload.equals("closed")) {
			setStatus(STATUS_DOORCLOSED);
		}
	}
	logger.print(tag, F("\n\t <<DoorSensor::sendCommand"));
	return false;
}
