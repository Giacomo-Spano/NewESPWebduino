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

	String mode;
	virtual void getJson(JsonObject& json);
	void setMode(String _mode);
	
public:
	static String STATUS_ON;
	static String STATUS_OFF;
	static String MODE_PAUSE;
	static String MODE_NORMAL;

	HornSensor(JsonObject& json);
	~HornSensor();

	virtual void init();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
	virtual void setStatus(String status);
};
#endif

