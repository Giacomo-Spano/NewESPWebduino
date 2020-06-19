// OnewireSensor.h

#ifndef _ONEWIRESENSOR_h
#define _ONEWIRESENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Sensor.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Logger.h"


class OnewireSensor :
	public Sensor
{
private:
	static String tag;
	static Logger logger;

	OneWire* oneWirePtr;
	DallasTemperature* pDallasSensors;

public:
	static const int avTempsize = 10;

	OnewireSensor(JsonObject& json);
	void createSensor();
	~OnewireSensor();
	virtual void checkStatusChange();

	void beginTemperatureSensors();
	virtual void getJson(JsonObject& json);

	static const int maxTempSensors = 10; // max num sensori onewire sullo stesso pin

	int avTempCounter = 0;
	float avTemp[avTempsize];

	bool readTemperatures();
	virtual void init();
	virtual void loadChildren(JsonArray& jsonarray);
};

#endif