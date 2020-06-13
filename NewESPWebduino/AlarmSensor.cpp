#include "AlarmSensor.h"
#include "Shield.h"

Logger AlarmSensor::logger;
String AlarmSensor::tag = "AlarmSensor";

extern Sensor* getSensor(int id);

/*AlarmSensor::AlarmSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>AlarmSensor::AlarmSensor"));

	type = "alarmsensor";
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<AlarmSensor::DooAlarmSensorrSensor\n"));
}*/

AlarmSensor::AlarmSensor(String jsonStr) : Sensor(jsonStr)
{
	logger.print(tag, F("\n\t>>AlarmSensor::AlarmSensor"));

	type = "alarmsensor";
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<AlarmSensor::AlarmSensor\n"));
}

AlarmSensor::~AlarmSensor()
{
}

/*void* AlarmSensor::callbackmethod(String oldstatus, String newStatus) {

}*/
/*void AlarmSensor::callBack(String status)
{
	//_midiInCallback = midiInCallback;
}*/


void AlarmSensor::callBack(int sensorid, String status, String oldstatus) {
	logger.print(tag, "\n\t >> AlarmSensor::callBack sendor id=" + String(sensorid) + "status=" + status + "oldstatus=" + oldstatus);
	Serial.println("\n\t prova AlarmSensor");

	if (sensorid == doorsensordid) {
		if (getStatus().equals(STATUS_ARMED_HOME) || getStatus().equals(STATUS_ARMED_AWAY) || getStatus().equals(STATUS_ARMED_NIGHT)) {
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
	if (sensor != nullptr) {
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
	//json["mode"] = mode;
}

bool AlarmSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		//logger.print(tag, "\nDoorSensor::checkStatusChange\n");
		lastCheckStatus = currMillis;
				
		ret = Sensor::checkStatusChange();
	}
	return ret;
}

bool AlarmSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>AlarmSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	
	if (command.equals("set")) {
		logger.print(tag, String(F("\n\t\t process command set")));

		if (payload.equals("disarmed")) {
			logger.print(tag, String(F("\n\t\t pSTATUS_DISARMED")));
			setStatus(STATUS_DISARMED);
		}
		else if (payload.equals("armed_home")) {
			logger.print(tag, String(F("\n\t\t STATUS_ARMED_HOME")));
			setStatus(STATUS_ARMED_HOME);
		}
		else if (payload.equals("armed_away")) {
			setStatus(STATUS_ARMED_AWAY);
		}
		else if (payload.equals("armed_night")) {
			setStatus(STATUS_ARMED_NIGHT);
		}
		else if (payload.equals("armed_custom_bypass")) {
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
