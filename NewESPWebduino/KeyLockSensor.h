#ifndef _KEYLOCKSENSOR_h
#define _KEYLOCKSENSOR_h

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

class KeyLockSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual void getJson(JsonObject& json);
	int oldPositionStatus;


	int zeroThreshold = 0;
	int maxThreshold = 700;
	int openThreshold = 50;
	int firstLockThreshold = 0;
	int secondLockThreshold = 0;
	int closedThreshold = 650;
	int maxrotation = 5000;

	int countRotation = 0;
	int targetpos = 0;
	double delta = 0;

	
	int stepPin;// = 0; //GPIO0---D3 of Nodemcu--Step of stepper motor driver
	int directionPin;// = 2; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver
	int enablePin;// = 14; //GPI14---D5 of Nodemcu--Enable driver

	

	void enableMotor(bool enable);
	void rotateMotor();
	void SetMotorDirection(bool clockwise);
	void updateStatus();
	bool motorEnabled = false;

public:
	static String STATUS_OPEN;
	static String STATUS_FIRSTLOCK;
	static String STATUS_CLOSED;
	static String STATUS_OPENING;
	static String STATUS_CLOSING;
	static String STATUS_MOVING;
	static String STATUS_STOPPING;

	// questi devono essere pubblic perche sono uisati dalla funzione di interrupt
	static int outputAPin;// 2;// D1;
	static int outputBPin; // 4;// D2;

	bool stopLock();
	bool openLock();
	bool closeLock();
	bool setPosition(int pos);
	bool zeroCalibration();
	bool setOpenThreshold();
	bool setFirstLockThreshold();
	bool setClosedThreshold();
	
	KeyLockSensor(JsonObject& json);
	~KeyLockSensor();

	virtual void init();
	virtual bool checkStatusChange();
	virtual bool sendCommand(String command, String payload);
	//virtual void sendStatusUpdate();

};
#endif



