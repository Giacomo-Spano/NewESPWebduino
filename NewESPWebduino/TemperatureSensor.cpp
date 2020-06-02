#include "TemperatureSensor.h"
#include "Shield.h"

Logger TemperatureSensor::logger;
String TemperatureSensor::tag = "TemperatureSensor";


/*TemperatureSensor::TemperatureSensor(int id, uint8_t pin, bool enabled, String address, String name) : Sensor(id,pin, enabled, address, name)
{
	type = "temperaturesensor";
	unavailable = true;
}*/

TemperatureSensor::TemperatureSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>OnewireSensor::OnewireSensor json"));

	type = "temperaturesensor";
	unavailable = true;

	logger.print(tag, F("\n\t<<OnewireSensor::OnewireSensor json\n"));
}

TemperatureSensor::~TemperatureSensor()
{
}

void TemperatureSensor::init()
{
}

String TemperatureSensor::getStatusText()
{
	String text = "" + String(temperature) + "°C";
	return text;
}

void TemperatureSensor::getJson(JsonObject& json) {
	logger.print(tag, F("\n\t>>TemperatureSensor::getJson\n"));
	Sensor::getJson(json);
	json["temp"] = temperature;
	//logger.printJson(json);
	logger.print(tag, F("\n\t<<TemperatureSensor::getJson\n"));
}


