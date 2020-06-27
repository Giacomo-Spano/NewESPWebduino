#include "SwitchSensor.h"
#include "Shield.h"

Logger SwitchSensor::logger;
String SwitchSensor::tag = "SwitchSensor";

String  SwitchSensor::STATUS_RELE_ON = "on";
String  SwitchSensor::STATUS_RELE_OFF = "off";
String  SwitchSensor::STATUS_RELE_TIMERON = "timeron";

SwitchSensor::SwitchSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>SwitchSensor::SwitchSensor"));
	
	type = "Switchsensor";
	
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<SwitchSensor::SwitchSensor\n"));
}

void SwitchSensor::setStatus(String _status) {

	if (_status.equals(STATUS_RELE_ON)) {
		logger.print(tag, F("\n\t RELE ON"));
		digitalWrite(pin, LOW);
	}
	else if (_status.equals(STATUS_RELE_OFF)) {
		logger.print(tag, F("\n\t RELE OFF"));
		digitalWrite(pin, HIGH);
	} else if (_status.equals(STATUS_RELE_TIMERON)) {
		logger.print(tag, F("\n\t RELE ON"));
		digitalWrite(pin, LOW);

		timerOnStartTime = millis();
	}
	Sensor::setStatus(_status);
}

void SwitchSensor::init()
{
	logger.print(tag, "\n\t >>init SwitchSensor pin=" + String(pin));
	Sensor::init();

	pinMode(pin, OUTPUT);
	setStatus(STATUS_RELE_OFF);
	logger.print(tag, F("\n\t <<init SwitchSensor"));
}

void SwitchSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["mode"] = mode;
}

void SwitchSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		lastCheckStatus = currMillis;

		if (getStatus().equals(STATUS_RELE_TIMERON)) {
			if ((currMillis - timerOnStartTime) > timerOnTimeout || (currMillis - timerOnStartTime) < 0) {
				setStatus(STATUS_RELE_OFF);
			}
		}
		Sensor::checkStatusChange();
	}
}

bool SwitchSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>SwitchSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("set")) {
		if (payload.equals("on")) {
			setStatus(STATUS_RELE_ON);
		}
		else if (payload.equals("off")) {
			setStatus(STATUS_RELE_OFF);
		}
	}
	else if (command.equals("timer")) {
		timerOnTimeout = payload.toInt() * 1000;
		setStatus(STATUS_RELE_TIMERON);
	}
	logger.print(tag, F("\n\t <<SwitchSensor::sendCommand"));
	return false;
}
