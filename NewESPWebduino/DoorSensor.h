// DoorSensor.h

#ifndef _DOORSENSOR_h
#define _DOORSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h> 

class DoorSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	unsigned long lastTestModeTime;
	static const long TestModeTimeout = 30000;

	String mode;
	virtual void getJson(JsonObject& json);
	
public:
	static String STATUS_DOOROPEN;
	static String STATUS_DOORCLOSED;

	static String MODE_NORMAL;
	static String MODE_TEST;
	static String MODE_TESTOPEN;

	DoorSensor(JsonObject& json);
	~DoorSensor();
	virtual void init();
	virtual bool checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

