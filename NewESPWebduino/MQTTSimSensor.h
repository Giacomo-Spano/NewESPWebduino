#ifndef _MQTTSIMSENSOR_h
#define _MQTTSIMSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h> 

class MQTTSimSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual void getJson(JsonObject& json);

public:
	static String STATUS_DOOROPEN;// = "dooropen";
	static String STATUS_DOORCLOSED;// = "doorclosed";

	MQTTSimSensor(JsonObject& json);
	~MQTTSimSensor();

	virtual void init();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

