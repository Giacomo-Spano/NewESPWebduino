#include "HornSensor.h"
#include "Shield.h"

Logger HornSensor::logger;
String HornSensor::tag = "HornSensor";

HornSensor::HornSensor(String jsonStr) : Sensor(jsonStr)
{
	logger.print(tag, F("\n\t>>HornSensor::HornSensor"));
	
	//type = "alarmsensor";
	DynamicJsonBuffer jbuff;
	JsonObject& json = jbuff.parseObject(jsonStr);

	if (json.success()) {
				
		if (!json.containsKey("horntimeout")) {
			int timeout = json["horntimeout"];
			hornTimeout = timeout * 1000;
		}
		else {
			hornTimeout = 10 * 1000;
		}

		if (!json.containsKey("hornpausetimeout")) {
			int timeout = json["hornpausetimeout"];
			hornPauseTimeout = timeout * 1000;
		}
		else {
			hornPauseTimeout = 10 * 1000;
		}

		if (!json.containsKey("horntally")) {
			hornMaxTally = json["horntally"];
		}
		else {
			hornMaxTally = 3;
		}

		checkStatus_interval = 1000;
		lastCheckStatus = 0;

	}
	else {
		logger.print(tag, F("\n\t BAD DATA - FAILED"));
	}

	logger.print(tag, F("\n\t<<HornSensor::HornSensor\n"));
}

/*HornSensor::HornSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>HornSensor::HornSensor"));

	type = "hornsensor";
	checkStatus_interval = 1000;
	lastCheckStatus = 0;
	
	if (!json.containsKey("horntimeout")) {
		int timeout = json["horntimeout"];
		hornTimeout = timeout * 1000;
	}
	else {
		hornTimeout = 10 * 1000;
	}

	if (!json.containsKey("hornpausetimeout")) {
		int timeout = json["hornpausetimeout"];
		hornPauseTimeout = timeout * 1000;
	}
	else {
		hornPauseTimeout = 10 * 1000;
	}

	if (!json.containsKey("horntally")) {
		hornMaxTally = json["horntally"];
	}
	else {
		hornMaxTally = 3;
	}
	
	logger.print(tag, F("\n\t<<HornSensor::HornSensor\n"));
}*/

HornSensor::~HornSensor()
{
}

void HornSensor::init()
{
	logger.print(tag, F("\n\t >>init HornSensor"));
	
	Sensor::init();
	//pinMode(pin, OUTPUT);


	logger.print(tag, F("\n\t <<init HornSensor"));
}

void HornSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	/*json["horntimeout"] = (int) hornTimeout / 1000;
	json["hornpausetimeout"] = (int) hornPauseTimeout / 1000;
	json["horntally"] = hornMaxTally;*/
	//json.printTo(Serial);
}

bool HornSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	lastCheckStatus = currMillis;

	/*if (getStatus().equals(STATUS_ON)) {
		if ((currMillis - hornStartTime) > hornTimeout // timeout trascorso
			|| (currMillis - hornStartTime) < 0) { // oppure currmill resettato

			if (hornTallyCounter < hornMaxTally) {
				hornPauseStartTime = millis();
				setStatus(STATUS_PAUSE);
			}
			else {
				setStatus(STATUS_OFF);
			}
			return true;
		}
	} else if (getStatus().equals(STATUS_PAUSE)) {
		if ((currMillis - hornPauseStartTime) > hornPauseTimeout // timeout trascorso
			|| (currMillis - hornPauseStartTime) < 0) { // oppure currmill resettato

			if (hornTallyCounter < hornMaxTally) {
				hornTallyCounter++;
				hornStartTime = millis();
				setStatus(STATUS_ON);
			} else {
				setStatus(STATUS_OFF);
			}
			return true;
		}
	} else if (getStatus().equals(STATUS_PAUSE) || getStatus().equals(STATUS_IDLE)) {
		return false;
	}*/
	return false;
}

bool HornSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>HornSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("set")) {
		if (payload.equals("on")) {
			hornStartTime = millis();
			hornTallyCounter = 0;
			setStatus(STATUS_ON);
		}
		else if (payload.equals("off")) {
			setStatus(STATUS_OFF);
		}
		else if (payload.equals("pause")) {
			hornPauseStartTime = millis();
			setStatus(STATUS_PAUSE);
		}
	}
	
	logger.print(tag, F("\n\t <<HornSensor::sendCommand"));
	return false;
}
