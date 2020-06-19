#include "SensorFactory.h"
#include "TemperatureSensor.h"
#include "KeylockSensor.h"
#include "OnewireSensor.h"
#include "DoorSensor.h"
#include "HornSensor.h"
#include "AlarmSensor.h"
#include "SimSensor.h"
#include "MQTTSimSensor.h"
#include "CAMSensor.h"
#include "Shield.h"
#include "SensorListener.h"

//extern bool getNextSensorId();

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
	else if (type.equals(F("simsensor"))) {
		logger.print(tag, F("\n\t creating simsensor sensor"));
		sensor = new SimSensor(json);
	}
	else if (type.equals(F("mqttsimsensor"))) {
		logger.print(tag, F("\n\t creating mqttsimsensor sensor"));
		sensor = new MQTTSimSensor(json);
	}
	else if (type.equals(F("camsensor"))) {
		logger.print(tag, F("\n\t creating camsensor sensor"));
		sensor = new CAMSensor(json);
	}
	else if (type.equals(F("keylocksensor"))) {
		logger.print(tag, F("\n\t creating keylocksensor sensor"));
		String str;
	}
	else if (type.equals(F("onewiresensor"))) {
		sensor = new OnewireSensor(json);
	}
	/*else if (type.equals(F("hornsensor"))) {
		logger.print(tag, F("\n\t creating doorsensor sensor"));
		sensor = new HornSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("rfidsensor"))) {
		logger.print(tag, F("\n\t creating rfidsensor sensor"));
		sensor = new RFIDSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("irsensor"))) {
		logger.print(tag, F("\n\t creating irsensor sensor"));
		sensor = new IRSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("irreceivesensor"))) {
		logger.print(tag, F("\n\t creating irreceivesensor sensor"));
		sensor = new IRReceiveSensor(sensorid, pin, enabled, address, name);
	}*/
	else {
		return nullptr;
	}
	//sensor->init();
	/*if (json.containsKey("childsensors")) {

		JsonArray& children = json["childsensors"];
		logger.print(tag, F("\n\t children="));
		//logger.print(tag, children);
		children.printTo(Serial);
		//JSONArray jarray(children);
		sensor->loadChildren(children);
	}*/

	logger.print(tag, F("\n\t<<SensorFactory::createSensor\n"));
	return sensor;
}


