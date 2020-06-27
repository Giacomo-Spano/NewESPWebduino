#include "HornSensor.h"
#include "Shield.h"

Logger HornSensor::logger;
String HornSensor::tag = "HornSensor";

String HornSensor::STATUS_ON = "hornon";
String HornSensor::STATUS_OFF = "hornoff";

String  HornSensor::MODE_NORMAL = "normal";
String  HornSensor::MODE_PAUSE = "pause";

HornSensor::HornSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>HornSensor::HornSensor"));
	
	type = "hornsensor";
	
	if (json.success()) {
				
		if (json.containsKey("horntimeout")) {
			int timeout = json["horntimeout"];
			hornTimeout = timeout * 1000;
		}
		else {
			hornTimeout = 10000;
		}

		if (json.containsKey("hornpausetimeout")) {
			int timeout = json["hornpausetimeout"];
			hornPauseTimeout = timeout * 1000;
		}
		else {
			hornPauseTimeout = 60000;
		}

		if (json.containsKey("horntally")) {
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

HornSensor::~HornSensor()
{
}

void HornSensor::init()
{
	logger.print(tag, F("\n\t >>init HornSensor"));
	
	Sensor::init();
	pinMode(pin, OUTPUT);

	mode = MODE_NORMAL;
	setStatus(STATUS_OFF);	

	logger.print(tag, F("\n\t <<init HornSensor"));
}

void HornSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["mode"] = mode;
}

void HornSensor::checkStatusChange() {

	//logger.print(tag, F("\n\t >> HornSensor::checkStatusChange"));

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	lastCheckStatus = currMillis;

	if (mode.equals(MODE_NORMAL)) {
		if (getStatus().equals(STATUS_ON)) {
			if ((currMillis - hornStartTime) > hornTimeout // timeout trascorso
				|| (currMillis - hornStartTime) < 0) { // oppure currmill resettato

				setStatus(STATUS_OFF);
				logger.print(tag, "\n\t END HORN TIMEOUT tall=" + hornTallyCounter);
				if (hornTallyCounter < hornMaxTally) {
					hornPauseStartTime = millis();
					setMode(MODE_PAUSE);
				}
				else {
					setMode(MODE_NORMAL);
				}
			}
		}
	}
	else if (mode.equals(MODE_PAUSE)) {
		if ((currMillis - hornPauseStartTime) > hornPauseTimeout // timeout trascorso
			|| (currMillis - hornPauseStartTime) < 0) { // oppure currmill resettato

			logger.print(tag, "\n\t END HORN PAUSE TIMEOUT tall=" + hornTallyCounter);
			if (hornTallyCounter < hornMaxTally) {
				hornTallyCounter++;
				//hornStartTime = millis();
				setMode(MODE_NORMAL);
				setStatus(STATUS_ON);
			}
			else {
				setMode(MODE_NORMAL);
			}
		}
	}
	Sensor::checkStatusChange();
}

void HornSensor::setMode(String _mode) {
	mode = _mode;
	if (_mode.equals(MODE_PAUSE)) {
		mode = _mode;
	}
	else if (_mode.equals(MODE_NORMAL)) {
		mode = _mode;
	}
}

void HornSensor::setStatus(String _status) {
		
	if (_status.equals(STATUS_ON)) {
		logger.print(tag, "\n\t STATUS HORN ON");
		digitalWrite(pin, LOW);
		hornStartTime = millis();
	} else if (_status.equals(STATUS_OFF)) {
		digitalWrite(pin, HIGH);
		logger.print(tag, "\n\t STATUS HORN OFF");
	}
	Sensor::setStatus(_status);
}

bool HornSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>HornSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("set")) {
		logger.print(tag, "\n\t\t process set command");
		if (payload.equals("hornon")) {
			logger.print(tag, "\n\t\t hornon");
			//hornStartTime = millis();
			hornTallyCounter = 0;
			setStatus(STATUS_ON);
			setMode(MODE_NORMAL);
		}
		else if (payload.equals("hornon")) {
			logger.print(tag, "\n\t\t hornoff");
			setStatus(STATUS_OFF);
			setMode(MODE_NORMAL);
		}
		else if (payload.equals("pause")) {
			logger.print(tag, "\n\t\t pause");
			hornPauseStartTime = millis();
			setMode(MODE_PAUSE);
		}
	}
	
	logger.print(tag, F("\n\t <<HornSensor::sendCommand"));
	return false;
}
