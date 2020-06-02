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

	int openThreshold = 0;
	int firstLockThreshold = 0;
	int secondLockThreshold = 0;
	int closedThreshold = 0;
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
	const String STATUS_OPEN = "open";
	const String STATUS_FIRSTLOCK = "firstlock";
	const String STATUS_CLOSED = "closed";
	const String STATUS_OPENING = "opening";
	const String STATUS_CLOSING = "closing";
	const String STATUS_MOVING = "moving";

	// questi devono essere pubblic perche sono uisati dalla funzione di interrupt
	static int outputAPin;// 2;// D1;
	static int outputBPin; // 4;// D2;

	bool openLock();
	bool closeLock();
	//bool rotateLock(bool close, int delta);

	//KeyLockSensor(int id, uint8_t pin, bool enabled, String address, String name, uint8_t stepPin, uint8_t directionPin, uint8_t enablePin, uint8_t outputAPin, uint8_t outputbPin);
	KeyLockSensor(JsonObject& json);
	~KeyLockSensor();

	virtual void init();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
	virtual void sendStatusUpdate();
	bool setPosition(int pos);
	bool zeroCalibration();
};
#endif



