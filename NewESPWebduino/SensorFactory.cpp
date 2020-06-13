#include "SensorFactory.h"
#include "TemperatureSensor.h"
#include "KeylockSensor.h"
#include "OnewireSensor.h"
#include "DoorSensor.h"
#include "HornSensor.h"
#include "AlarmSensor.h"
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

Sensor* SensorFactory::createSensor(JsonObject& jsonref)
{
	logger.print(tag, F("\n\t >>SensorFactory::createSensor \njson = "));
	jsonref.printTo(Serial);

	uint8_t pin = 0;
	bool enabled = false;
	String name = "";
	if (!jsonref.containsKey("type") || !jsonref.containsKey("sensorid")) {
		logger.print(tag, F("\n\t invalidtype and sensorid"));
		return nullptr;
	}
	String type = jsonref["type"];
	type.replace("\r\n", "");

	String address = "0";// json[F("subaddress")];

	int sensorid = jsonref["sensorid"];

	if (sensorid >= nextSensorID)
		nextSensorID = sensorid + 1;

	if (sensorid == 0) {
		sensorid = getNextSensorId();
		jsonref["sensorid"] = sensorid;
	}

	/*if (json.containsKey(F("pin"))) {
		String strPin = json[F("pin")];
		strPin.replace("\r\n", ""); // importante!!
		pin = Shield::pinFromStr(strPin);
	}
	if (json.containsKey(F("enabled"))) {
		enabled = json["enabled"];
	}
	if (json.containsKey(F("name"))) {
		String str = json["name"];
		name = str;
	}*/

	DynamicJsonBuffer jbuff;
	//String str;
	//jsonref.printTo(str);
	//jsonref.printTo(Serial);
	/*JsonObject& root = jbuff.parseObject(str);
	if (!root.success()) {
		return nullptr;
	}*/
	Sensor* sensor = nullptr;


	if (type.equals(F("temperaturesensor"))) {
		logger.print(tag, F("\n\t creating temperature sensor"));
		sensor = new TemperatureSensor(jsonref);
	}
	/*else if (type.equals("heatersensor")) {
		logger.print(tag, F("\n\t creating heatersensor sensor"));
		sensor = new HeaterSensor(sensorid, pin, enabled, address, name);
	}*/
	else if (type.equals("doorsensor")) {
		logger.print(tag, "\n\t creating doorsensor sensor\n");
		String jsonstr;
		jsonref.printTo(jsonstr);
		sensor = new DoorSensor(jsonstr);
	}
	else if (type.equals(F("hornsensor"))) {
		logger.print(tag, F("\n\t creating hornsensor sensor."));
		/*String str;
		jsonref.printTo(str);
		JsonObject& root = jbuff.parseObject(str);
		if (root.success()) {
			return new HornSensor(root);
		}*/
		String jsonstr;
		logger.print(tag, F("\n\t step1"));
		jsonref.printTo(jsonstr);
		logger.print(tag, F("\n\t step1"));
		sensor = new HornSensor(jsonstr);
		logger.print(tag, F("\n\t step3"));
	}
	else if (type.equals(F("alarmsensor"))) {
		logger.print(tag, F("\n\t creating alarmsensor sensor"));
		String jsonstr;
		logger.print(tag, F("\n\t step1"));
		jsonref.printTo(jsonstr);
		logger.print(tag, F("\n\t step2"));
		sensor = new AlarmSensor(jsonstr);
		logger.print(tag, F("\n\t step3"));
	}
	else if (type.equals(F("keylocksensor"))) {
		logger.print(tag, F("\n\t creating keylocksensor sensor"));
		String str;
		jsonref.printTo(str);
		JsonObject& root = jbuff.parseObject(str);
		if (root.success()) {
			return new KeyLockSensor(root);
		}
		//sensor = new KeyLockSensor(jsonref);
	}
	else if (type.equals(F("onewiresensor"))) {
		String str;
		jsonref.printTo(str);
		JsonObject& root = jbuff.parseObject(str);
		if (root.success()) {
			return new OnewireSensor(root);
		}
		//sensor = new OnewireSensor(jsonref);
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

