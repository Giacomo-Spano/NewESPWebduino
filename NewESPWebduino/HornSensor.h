// DoorSensor.h

#ifndef _HORNSENSOR_h
#define _HORNSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h> 

class HornSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	unsigned long hornTimeCounter;
	unsigned long hornStartTime;
	unsigned long hornPauseStartTime;
	int hornTallyCounter;

	unsigned long hornTimeout;
	unsigned long hornPauseTimeout;
	unsigned long hornMaxTally;

	//String mode;
	virtual void getJson(JsonObject& json);
	
public:
	const String STATUS_ON = "hornon";
	const String STATUS_OFF = "hornoff";
	const String STATUS_PAUSE = "hornpause";

	HornSensor(JsonObject& json);
	~HornSensor();

	virtual void init();
	virtual bool checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

