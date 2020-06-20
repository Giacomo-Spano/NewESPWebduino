#ifndef _SIMSENSOR_h
#define _SIMSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#ifdef SIMSENSOR


#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h> 

class SimSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual void getJson(JsonObject& json);

public:
	//const String STATUS_DOOROPEN = "dooropen";
	//const String STATUS_DOORCLOSED = "doorclosed";

	SimSensor(JsonObject& json);
	~SimSensor();

	virtual void init();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

#endif

