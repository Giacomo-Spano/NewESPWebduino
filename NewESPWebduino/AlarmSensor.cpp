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
		
	if (json.containsKey("hornsensorid")) {
		hornSensorsid = json["hornsensorid"];
	}
	if (json.containsKey("doorsensorid")) {
		doorSensorsid = json["doorsensorid"];
	}

	type = "alarmsensor";
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	//logger.print(tag, "\n\t hornsensors,size=" + String(hornsensors.size()));
	logger.print(tag, F("\n\t<<AlarmSensor::AlarmSensor\n"));
}

AlarmSensor::~AlarmSensor()
{
}

void AlarmSensor::callBack(int sensorid, String status, String oldstatus) {
	logger.print(tag, "\n\t >> AlarmSensor::callBack sendor id=" + String(sensorid) + "status=" + status + "oldstatus=" + oldstatus);
	
	if (sensorid == doorsensordid) {
		if (getStatus().equals(STATUS_ARMED_HOME) || getStatus().equals(STATUS_ARMED_AWAY) || getStatus().equals(STATUS_ARMED_NIGHT)) {
			logger.print(tag, "\n\t >> AlarmSensor::callBack TRIGGER ALARM");
			setStatus(STATUS_TRIGGERED);
		}
	}

	logger.print(tag, "\n\t << AlarmSensor::callBack");
};

void AlarmSensor::init()
{
	logger.print(tag, "\n\t >>init AlarmSensor id=" + String(sensorid));

	Sensor::init();
		
	doorsensordid = 1;
	Sensor* sensor = getSensor(doorsensordid);
	if (sensor != nullptr) { // si registra agli eventi del door sensor
		logger.print(tag, "\n\t door sendor id " + String(doorsensordid) + "found");
		sensor->addType0CallBack(this);
	}
	else {
		logger.print(tag, "\n\t door sendor id " + String(doorsensordid) + "not found");
	}

	logger.print(tag, F("\n\t <<init AlarmSensor"));
}

void AlarmSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["hornsensorid"] = hornSensorsid;
	json["doorsensorid"] = doorSensorsid;
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
		Sensor* phorn = getSensor(hornSensorsid);
		if (phorn != nullptr) {
			phorn->sendCommand("set", "on");
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
