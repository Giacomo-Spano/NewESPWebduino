#include "AlarmSensor.h"
#include "Shield.h"

Logger AlarmSensor::logger;
String AlarmSensor::tag = "AlarmSensor";

String AlarmSensor::STATUS_DISARMED = "disarmed";
String AlarmSensor::STATUS_ARMED_HOME = "armed_home";
String AlarmSensor::STATUS_ARMED_AWAY = "armed_away";
String AlarmSensor::STATUS_ARMED_NIGHT = "armed_night";
String AlarmSensor::STATUS_ARMED_CUSTOM_BYPASS = "armed_custom_bypass";
String AlarmSensor::STATUS_PENDING = "pending";
String AlarmSensor::STATUS_TRIGGERED = "triggered";
String AlarmSensor::STATUS_ARMING = "arming";
String AlarmSensor::STATUS_DISARMING = "disarming";


extern Sensor* getSensor(int id);

AlarmSensor::AlarmSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>AlarmSensor::AlarmSensor"));

	//String jsonstr;
	//json.printTo(jsonstr);
	//logger.print(tag, jsonstr);

	if (json.containsKey("hornsensorid")) {
		hornSensorId = json["hornsensorid"].as<int>();
		logger.print(tag, "\n\t hornsensorid:" + String(hornSensorId));
	}
	if (json.containsKey("doorsensorid")) {
		String str = json["doorsensorid"];
		doorSensorId = json["doorsensorid"].as<int>();
		logger.print(tag, "\n\t doorsensorid:" + String(doorSensorId));
	}
	if (json.containsKey("rfidsensorid")) {
		rfidSensorId = json["rfidsensorid"].as<int>();
		logger.print(tag, "\n\t rfidsensorid:" + String(rfidSensorId));
	}

	type = "alarmsensor";
	checkStatus_interval = 500;
	lastCheckStatus = 0;

	//logger.print(tag, "\n\t hornsensors,size=" + String(hornsensors.size()));
	logger.print(tag, F("\n\t<<AlarmSensor::AlarmSensor\n"));
}

AlarmSensor::~AlarmSensor()
{
}

void AlarmSensor::callBack(int sensorid, String status, String oldstatus) {
	logger.print(tag, "\n\t >> AlarmSensor::callBack sendor id=" + String(sensorid) + "status=" + status + "oldstatus=" + oldstatus);

	if (sensorid == doorSensorId) {
		if (getStatus().equals(STATUS_ARMED_HOME) || getStatus().equals(STATUS_ARMED_AWAY) || getStatus().equals(STATUS_ARMED_NIGHT)) {
			logger.print(tag, "\n\t >> AlarmSensor::callBack TRIGGER ALARM");
			setStatus(STATUS_TRIGGERED);
		}
	}

	logger.print(tag, "\n\t << AlarmSensor::callBack");
};

void AlarmSensor::callBackEvent(int sensorid, String event, String param) {
	logger.print(tag, "\n\t >> Sensor::callBackEvent sendor id=" + String(sensorid) + "event=" + event + "param=" + param);

	if (sensorid == rfidSensorId && event.equals("cardread")) {
		if (param == "A0 D1 16 7C" ||
			param == "76 FD 97 BB" ||
			param == "86 C8 96 BB") //change here the UID of the card/cards that you want to give access
		{
			logger.print(tag, "\n\t Authorized access - card " + param);
			
			if (getStatus().equals(STATUS_ARMED_HOME) || getStatus().equals(STATUS_ARMED_AWAY)
				|| getStatus().equals(STATUS_ARMED_NIGHT) || getStatus().equals(STATUS_TRIGGERED)) {
				
				logger.print(tag, "\n\t >> AlarmSensor::callBack DISARM");
				setStatus(STATUS_DISARMED);
			}
			else {
				setStatus(STATUS_ARMED_HOME);
			}
		}
		else {
			Serial.println(" Access denied");
			//delay(3000);
		}
	}
	logger.print(tag, "\n\t << AlarmSensor::callBackEvent");
};


void AlarmSensor::init()
{
	logger.print(tag, "\n\t >>init AlarmSensor id=" + String(sensorid));

	Sensor::init();

	//doorsensordid = 1;
	Sensor* doorsensor = getSensor(doorSensorId);
	if (doorsensor != nullptr) { // si registra agli eventi del door sensor
		logger.print(tag, "\n\t door sendor id " + String(doorSensorId) + "found");
		doorsensor->addType0CallBack(this);
	}
	else {
		logger.print(tag, "\n\t door sendor id " + String(doorSensorId) + "not found");
	}

	Sensor* rfidsensor = getSensor(rfidSensorId);
	if (rfidsensor != nullptr) { // si registra agli eventi del door sensor
		logger.print(tag, "\n\t rfidsendorid " + String(rfidSensorId) + "found");
		rfidsensor->addType0CallBack(this);
	}
	else {
		logger.print(tag, "\n\t rfidsendorid " + String(rfidSensorId) + "not found");
	}

	logger.print(tag, F("\n\t <<init AlarmSensor"));
}

void AlarmSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["hornsensorid"] = hornSensorId;
	json["doorsensorid"] = doorSensorId;
	json["rfidsensorid"] = rfidSensorId;
}

void AlarmSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		lastCheckStatus = currMillis;

		if (getStatus().equals(STATUS_IDLE))
			setStatus(STATUS_DISARMED);

	}
	Sensor::checkStatusChange();
}

void AlarmSensor::setStatus(String _status) {

	Sensor::setStatus(_status);

	if (getStatus().equals(STATUS_TRIGGERED)) {
		Sensor* phorn = getSensor(hornSensorId);
		if (phorn != nullptr) {
			phorn->sendCommand("set", "on");
		}
	}
	else if (getStatus().equals(STATUS_DISARMED)) {
		Sensor* phorn = getSensor(hornSensorId);
		if (phorn != nullptr) {
			phorn->sendCommand("set", "off");
		}
	}
}

bool AlarmSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>AlarmSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);

	if (command.equals("set")) {
		logger.print(tag, String(F("\n\t\t process command set")));

		if (payload.equals("DISARM")) {
			logger.print(tag, String(F("\n\t\t pSTATUS_DISARMED")));
			setStatus(STATUS_DISARMED);
		}
		else if (payload.equals("ARM_HOME")) {
			logger.print(tag, String(F("\n\t\t STATUS_ARMED_HOME")));
			setStatus(STATUS_ARMED_HOME);
		}
		else if (payload.equals("ARM_AWAY")) {
			setStatus(STATUS_ARMED_AWAY);
		}
		else if (payload.equals("ARM_NIGHT")) {
			setStatus(STATUS_ARMED_NIGHT);
		}
		else if (payload.equals("ARM_CUSTOM_BYPASS")) {
			setStatus(STATUS_ARMED_CUSTOM_BYPASS);
		}
		else if (payload.equals("pending")) {
			setStatus(STATUS_PENDING);
		}
		else if (payload.equals("triggered")) {
			setStatus(STATUS_TRIGGERED);
		}
		else if (payload.equals("arming")) {
			setStatus(STATUS_ARMING);
		}
		else if (payload.equals("disarming")) {
			setStatus(STATUS_DISARMING);
		}
	}

	logger.print(tag, F("\n\t <<AlarmSensor::sendCommand"));
	return false;
}
