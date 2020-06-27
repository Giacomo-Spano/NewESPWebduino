// DoorSensor.h

#ifndef _ALARMSENSOR_h
#define _ALARMSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h> 


//#define maxhornsensor	10

/*class Horn {
public: 
	int id;
};*/

class AlarmSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	unsigned long lastTestModeTime;
	const long TestModeTimeout = 30000;

	String mode;
	virtual void getJson(JsonObject& json);
	virtual void setStatus(String status);

	//SimpleList<Horn*> hornsensors = SimpleList<Horn*>();
	int hornSensorId;
	int doorSensorId;
	int rfidSensorId;
	
public:
	//SimpleList<Horn*> hornsensors = SimpleList<Horn*>();
	//const int maxhornsensor = 10;
	
	//int hornsensoridnum;

	void callBack(int sensorid, String status, String oldstatus);
	void callBackEvent(int sensorid, String event, String param);

	static String STATUS_DISARMED;
	static String STATUS_ARMED_HOME;
	static String STATUS_ARMED_AWAY;
	static String STATUS_ARMED_NIGHT;
	static String STATUS_ARMED_CUSTOM_BYPASS;
	static String STATUS_PENDING;
	static String STATUS_TRIGGERED;
	static String STATUS_ARMING;
	static String STATUS_DISARMING;

	AlarmSensor(JsonObject& json);
	~AlarmSensor();

	virtual void init();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

