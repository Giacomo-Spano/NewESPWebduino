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
//#include "CommandResponse.h"
#include <ArduinoJson.h> 

class DoorSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	String mode;
	virtual void getJson(JsonObject& json);
	
public:
	const String STATUS_DOOROPEN = "dooropen";
	const String STATUS_DOORCLOSED = "doorclosed";

	const String MODE_NORMAL = "normal";
	const String MODE_TEST = "test";


	//DoorSensor(int id, uint8_t pin, bool enabled, String address, String name);
	DoorSensor(JsonObject& json);
	~DoorSensor();

	virtual void init();
	virtual bool checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

