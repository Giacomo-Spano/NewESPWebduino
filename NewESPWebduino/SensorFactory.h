#ifndef _SENSORFACTORY_h
#define _SENSORFACTORY_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

class SensorFactory
{
private:
	static String tag;
	static Logger logger;
	static int nextSensorID;
	
public:
	SensorFactory();
	~SensorFactory();

	Sensor* createSensor(JsonObject& json);
	static int getNextSensorId();
};

#endif

