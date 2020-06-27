// DoorSensor.h

#ifndef _RFIDSENSOR_h
#define _RFIDSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h> 

class RFIDSensor :
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

	RFIDSensor(JsonObject& json);
	~RFIDSensor();
	virtual void init();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

