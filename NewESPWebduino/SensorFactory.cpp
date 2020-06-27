#include "newespduino.h"

#include "Sensor.h"
#include "SensorFactory.h"
#include "TemperatureSensor.h"
#include "KeylockSensor.h"
#include "OnewireSensor.h"
#include "DoorSensor.h"
#include "HornSensor.h"
#include "AlarmSensor.h"
#ifdef SIMSENSOR
#include "SimSensor.h"
#endif
#ifdef MQTTSIMSENSOR
#include "MQTTSimSensor.h"
#endif
#include "RFIDSensor.h"
#ifdef CAMSENSOR
#include "CAMSensor.h"
#endif
#include "Shield.h"
#include "SensorListener.h"


Logger SensorFactory::logger;
String SensorFactory::tag = "SensorFactory";
int SensorFactory::nextSensorID = 1;

SensorFactory::SensorFactory()
{
}

SensorFactory::~SensorFactory()
{
}

int SensorFactory::getNextSensorId() {
	int res = nextSensorID;
	nextSensorID++;
	return res;
}

Sensor* SensorFactory::createSensor(JsonObject& json)
{
	logger.print(tag, "\n\t >>SensorFactory::createSensor");

	if (!json.containsKey("type") || !json.containsKey("sensorid")) {
		logger.print(tag, F("\n\t invalidtype and sensorid"));
		return nullptr;
	}
	String type = json["type"];
	type.replace("\r\n", "");
		
	int sensorid = json["sensorid"];	
	if (sensorid >= nextSensorID)
		nextSensorID = sensorid + 1;
	if (sensorid == 0) {
		sensorid = getNextSensorId();
		json["sensorid"] = sensorid;
	}
		
	Sensor* sensor = nullptr;
	if (type.equals(F("temperaturesensor"))) {
		logger.print(tag, F("\n\t creating temperature sensor"));
		sensor = new TemperatureSensor(json);
	}
	else if (type.equals("doorsensor")) {
		logger.print(tag, "\n\t creating doorsensor sensor\n");
		sensor = new DoorSensor(json);
	}
	else if (type.equals(F("hornsensor"))) {
		logger.print(tag, F("\n\t creating hornsensor sensor."));
		sensor = new HornSensor(json);
	}
	else if (type.equals(F("alarmsensor"))) {
		logger.print(tag, F("\n\t creating alarmsensor sensor"));
		sensor = new AlarmSensor(json);
	}
#ifdef  SIMSENSOR
	else if (type.equals(F("simsensor"))) {
		logger.print(tag, F("\n\t creating simsensor sensor"));
		sensor = new SimSensor(json);
		//ddd
	}
#endif //  SIMSENSOR
#ifdef MQTTSIMSENSOR
	else if (type.equals(F("mqttsimsensor"))) {
		logger.print(tag, F("\n\t creating mqttsimsensor sensor"));
		sensor = new MQTTSimSensor(json);
	}
#endif
	else if (type.equals(F("rfidsensor"))) {
		logger.print(tag, F("\n\t creating mqttsimsensor sensor"));
		sensor = new RFIDSensor(json);
	}
#ifdef CAMSENSOR
	else if (type.equals(F("camsensor"))) {
		logger.print(tag, F("\n\t creating camsensor sensor"));
		sensor = new CAMSensor(json);
	}
#endif
	else if (type.equals(F("keylocksensor"))) {
		logger.print(tag, F("\n\t creating keylocksensor sensor"));
		String str;
	}
	else if (type.equals(F("onewiresensor"))) {
		sensor = new OnewireSensor(json);
	}
	else {
		return nullptr;
	}
	
	logger.print(tag, F("\n\t<<SensorFactory::createSensor\n"));
	return sensor;
}


