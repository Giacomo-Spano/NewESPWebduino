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

/*class Type0CallBack : public AbstractCallBack {
public:
	void callBack(void* = nullptr, void* = nullptr) {
		Serial.println("prova");
	};
};*/

class AlarmSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	int doorsensordid;

	unsigned long lastTestModeTime;
	const long TestModeTimeout = 30000;

	String mode;
	virtual void getJson(JsonObject& json);
	
public:
	void callBack(int sensorid, String status, String oldstatus);

	const String STATUS_DISARMED = "disarmed";
	const String STATUS_ARMED_HOME = "armed_home";
	const String STATUS_ARMED_AWAY = "armed_away";
	const String STATUS_ARMED_NIGHT = "armed_night";
	const String STATUS_ARMED_CUSTOM_BYPASS = "armed_custom_bypass";
	const String STATUS_PENDING = "pending";
	const String STATUS_TRIGGERED = "triggered";
	const String STATUS_ARMING = "arming";
	const String STATUS_DISARMING = "disarming";

	//AlarmSensor(JsonObject& json);
	AlarmSensor(String jsonStr);
	~AlarmSensor();

	virtual void init();
	virtual bool checkStatusChange();
	virtual bool sendCommand(String command, String payload);
};
#endif

