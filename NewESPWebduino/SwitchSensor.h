// DoorSensor.h

#ifndef _SwitchSENSOR_h
#define _SwitchSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include "Logger.h"
#include <ArduinoJson.h> 

class SwitchSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	unsigned long lastTestModeTime;
	static const long TestModeTimeout = 30000;

	unsigned long timerOnStartTime;
	unsigned long timerOnTimeout;


	String mode;
	virtual void getJson(JsonObject& json);
  void setMode(String _mode);

	static String MODE_NORMAL;
	static String MODE_TIMER_ON;
  
	
public:
	static String STATUS_RELE_ON;
	static String STATUS_RELE_OFF;
	//static String STATUS_RELE_TIMERON;

	SwitchSensor(JsonObject& json);
	~SwitchSensor();
	virtual void init();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
	virtual void setStatus(String status);
  virtual String getStatusText();
  //void set_mode(String _mode);
};
#endif
