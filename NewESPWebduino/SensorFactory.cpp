#include "SensorFactory.h"
#include "TemperatureSensor.h"
//#include "HeaterSensor.h"
//#include "DoorSensor.h"
#include "KeylockSensor.h"
#include "OnewireSensor.h"
#include "RotaryEncoderSensor.h"
//#include "HornSensor.h"
//#include "RFIDSensor.h"
//#include "IRSensor.h"
//#include "IRReceiveSensor.h"
#include "Shield.h"
//#include "Util.h"
//#include "ESP8266Webduino.h"

Logger SensorFactory::logger;
String SensorFactory::tag = "SensorFactory";

SensorFactory::SensorFactory()
{
}

SensorFactory::~SensorFactory()
{
}


Sensor * SensorFactory::createSensor(int id, String type, uint8_t pin, bool enabled, String address, String name)
{
	logger.print(tag, F("\n\t >>SensorFactory::createSensor type="));
	logger.print(tag, type);

	logger.print(tag, "\n\t id=" + String(id));
	logger.print(tag, "\n\t type=" + type);
	logger.print(tag, "\n\t pin=" + String(pin));
	logger.print(tag, "\n\t enabled=" + String(enabled));
	logger.print(tag, "\n\t address=" + address);
	logger.print(tag, "\n\t name=" + name);


	Sensor* sensor = nullptr;
	if (type.equals(F("temperaturesensor"))) {
		logger.print(tag, F("\n\t creating temperature sensor"));
		sensor = new TemperatureSensor(id, pin, enabled, address, name);
	}
	/*else if (type.equals("F(heatersensor")) {
		logger.print(tag, "\n\t creating heatersensor sensor");
		sensor = new HeaterSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("doorsensor"))) {
		logger.print(tag, "\n\t creating doorsensor sensor");
		sensor = new DoorSensor(id, pin, enabled, address, name);
	}
	*/
	else if (type.equals(F("onewiresensor"))) {
		logger.print(tag, F("\n\t creating onewiresensor sensor"));
		sensor = new OnewireSensor(id, pin, enabled, address, name);
	}
	/*else if (type.equals((F("hornsensor")))) {
		logger.print(tag, F("\n\t creating hornsensor sensor"));
		sensor = new HornSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("rfidsensor"))) {
		logger.print(tag, F("\n\t creating rfidsensor sensor"));
		sensor = new RFIDSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("irsensor"))) {
		logger.print(tag, "\n\t creating irsensor sensor");
		sensor = new IRSensor(id, pin, enabled, address, name);
	}
	else if (type.equals(F("irreceivesensor"))) {
		logger.print(tag, F("\n\t creating irreceivesensor sensor"));
		sensor = new IRReceiveSensor(id, pin, enabled, address, name);
	}*/
		

	/*logger.printFreeMem(tag, F("end create sensor"));
	logger.println(tag, F("<<SensorFactory::createSensor"));
	return sensor;*/
	if (sensor == nullptr)
		logger.print(tag, "invalid sensor type =" + type);
	return sensor;
}



Sensor * SensorFactory::createSensor(JsonObject& json)
{
	logger.print(tag, F("\n\t >>SensorFactory::createSensor  \njson = "));
	json.printTo(Serial);

	uint8_t pin = 0;
	bool enabled = false;
	String name = "";
		if (!json.containsKey("type") /*|| !json.containsKey("subaddress")*/ || !json.containsKey("sensorid")) {
		logger.print(tag, F("\n\t invalid address and typ="));
		return nullptr;
	}
	String type = json["type"];
	type.replace("\r\n", "");
	String address = "0";// json[F("subaddress")];
	int sensorid = json["sensorid"];	
	if (json.containsKey(F("pin"))) {
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
	}
	logger.print(tag, "\n\t type=");
	logger.print(tag, type);
	logger.print(tag, F("\n\t addr="));
	//logger.print(tag, address);
	logger.print(tag, F("\n\t sensorid="));
	logger.print(tag, sensorid);
	logger.print(tag, F("\n\t pin="));
	logger.print(tag, String(pin));
	logger.print(tag, F("\n\t enabled="));
	logger.print(tag, String(enabled));
	logger.print(tag, F("\n\t name="));
	logger.print(tag, name);
	
	Sensor* sensor = nullptr;
	if (type.equals(F("temperaturesensor"))) {
		logger.print(tag, F("\n\t creating temperature sensor"));
		sensor = new TemperatureSensor(sensorid, pin, enabled, address, name);
	}
	/*else if (type.equals("heatersensor")) {
		logger.print(tag, F("\n\t creating heatersensor sensor"));
		sensor = new HeaterSensor(sensorid, pin, enabled, address, name);
	}
	else if (type.equals(F("doorsensor"))) {
		logger.print(tag, F("\n\t creating doorsensor sensor"));
		sensor = new DoorSensor(sensorid, pin, enabled, address, name);
	}*/
	else if (type.equals(F("keylocksensor"))) {
		logger.print(tag, F("\n\t creating keylocksensor sensor"));
		if (!json.containsKey("steppin"))
			return nullptr;
		String strPin = json["steppin"];
		strPin.replace("\r\n", ""); // importante!!
		uint8_t stepPin = Shield::pinFromStr(strPin);
		
		if (!json.containsKey("directionpin"))
			return nullptr;
		String strDirectionPin = json["directionpin"];
		strDirectionPin.replace("\r\n", ""); // importante!!
		uint8_t directionPin = Shield::pinFromStr(strDirectionPin);
		
		if (!json.containsKey("enablepin"))
			return nullptr;
		String strEnablePin = json["enablepin"];
		strEnablePin.replace("\r\n", ""); // importante!!
		uint8_t enablePin = Shield::pinFromStr(strEnablePin);

		if (!json.containsKey("outputapin"))
			return nullptr;
		String strOutputAPin = json["outputapin"];
		strOutputAPin.replace("\r\n", ""); // importante!!
		uint8_t outputAPin = Shield::pinFromStr(strOutputAPin);
		
		if (!json.containsKey("outputbpin"))
			return nullptr;
		String strOutputBPin = json["outputbpin"];
		strOutputBPin.replace("\r\n", ""); // importante!!
		uint8_t outputBpin = Shield::pinFromStr(strOutputBPin);

		sensor = new KeyLockSensor(sensorid, pin, enabled, address, name, stepPin, directionPin, enablePin, outputAPin, outputBpin);
	}
	else if (type.equals(F("onewiresensor"))) {
		logger.print(tag, F("\n\t creating onewiresensor sensor"));
		sensor = new OnewireSensor(sensorid, pin, enabled, address, name);
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
	sensor->init();
	/*if (json.containsKey("childsensors")) {
		
		JsonArray& children = json["childsensors"];
		logger.print(tag, F("\n\t children="));
		//logger.print(tag, children);
		children.printTo(Serial);
		//JSONArray jarray(children);
		sensor->loadChildren(children);
	}*/

	logger.printFreeMem(tag, F("end create sensor"));
	logger.println(tag, F("\t<<SensorFactory::createSensor"));
	return sensor;
}

