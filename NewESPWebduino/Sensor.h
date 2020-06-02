#ifndef _SENSOR_h
#define _SENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif



#include "Logger.h"
//#include "CommandResponse.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
//#include <LinkedList.h>
#include <SimpleList.h>

class Sensor
{
private:
	static String tag;
	static Logger logger;

protected:
	DynamicJsonBuffer* jsonBuffer[];

public:
	//Sensor();
	//Sensor(int id, uint8_t pin, bool enabled, String address, String name);
	//Sensor(int id, uint8_t pin, bool enabled, String address, String name, JsonArray& children);
	Sensor(JsonObject& json);
	~Sensor();

	const String STATUS_IDLE = "idle";
	
	SimpleList<Sensor*> childsensors = SimpleList<Sensor*>();
	int sensorid;
	String sensorname;
	String type;
	bool enabled;
	uint8_t pin;
	String address;
	String status = STATUS_IDLE;
	String oldStatus = STATUS_IDLE;
		
	unsigned long lastCheckStatus;// = 0;//-flash_interval;
	int checkStatus_interval = 1000;	// il valore corretto per ogni tipo di sensore
									// è impostato nel costruttore
									// default 1 secondo

	unsigned lastUpdateAvailabilityStatus = 0;
	int updateAvailabilityStatus_interval = 10000; // intervallo minimo di aggiornamentto 
									// default 1 minuto
	
	void updateAvailabilityStatus();
	void updateAttributes();
	virtual void checkStatusChange();
	virtual void sendStatusUpdate();

	
	void setStatus(String status);
	Sensor* getSensorFromId(int id);
	virtual void getJson(JsonObject& json);
	virtual void loadChildren(JsonArray& jsonarray);
	virtual String toString();	
	virtual void init();
	virtual String getStatusText();
	virtual bool sendCommand(String command, String payload);
	virtual bool receiveCommand(String command, int actuatorId, String uuid, String json);
	bool sendCommandResponse(String uuid, String response);
};

#endif