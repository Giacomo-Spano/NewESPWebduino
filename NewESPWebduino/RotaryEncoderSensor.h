#ifndef _ROTARYENCODERSENSOR_h
#define _ROTARYENCODERSENSOR_h

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

class RotaryEncoderSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	virtual void getJson(JsonObject& json);
	int position = 0;
	
	//bool open = false;

	//uint8_t stepPin = 0; //GPIO0---D3 of Nodemcu--Step of stepper motor driver
	//uint8_t directionPin = 2; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver
	//uint8_t enablePin = 14; //GPI14---D5 of Nodemcu--Enable driver

	/*int stepPin = 0; //GPIO0---D3 of Nodemcu--Step of stepper motor driver
	int directionPin = 2; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver
	int enablePin = 14; //GPI14---D5 of Nodemcu--Enable driver
	*/

	

	




public:
	const String STATUS_DOOROPEN = "open";
	const String STATUS_DOORCLOSED = "closed";

	bool openLock();
	bool closeLock();

	RotaryEncoderSensor(int id, uint8_t pin, bool enabled, String address, String name, uint8_t stepPin, uint8_t directionPin, uint8_t enablePin);
	~RotaryEncoderSensor();

	virtual void init();
	//virtual void checkStatusChange();
	virtual void checkStatusChange();
	virtual bool sendCommand(String command, String payload);
	virtual void sendStatusUpdate();
	//bool receiveCommand(String command, int id, String uuid, String json);
	bool setPosition(int pos);

	bool loop();
	//void doEncoder();
};
#endif



