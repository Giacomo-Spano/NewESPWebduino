#ifndef _SENSOR_h
#define _SENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

//#define SIMSENSOR
//#define MQTTSIMSENSOR



#include "Logger.h"
//#include "CommandResponse.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
//#include <LinkedList.h>
#include <SimpleList.h>

class AbstractCallBack {
public:
	virtual void callBack(int sensorid, String status, String oldstatus) {};
	virtual void callBackEvent(int sensorid, String event, String param) {};
};

class Sensor : public AbstractCallBack
{
private:
	static String tag;
	static Logger logger;

	

	String status;
	String oldStatus;

protected:
	DynamicJsonBuffer* jsonBuffer[];

public:
	AbstractCallBack* type0CallBackPointer = nullptr;
	void addType0CallBack(AbstractCallBack*);
	void callBack(int sensorid, String status, String oldstatus);
	void callBackEvent(int sensorid, String event, String param);
	//Sensor(String jsonStr);
	Sensor(JsonObject& json);
	Sensor();
	~Sensor();

	//virtual bool createSensor(JsonObject& json);

	static String STATUS_IDLE;
	
	SimpleList<Sensor*> childsensors = SimpleList<Sensor*>();
	int sensorid;
	String sensorname;
	String type;
	bool enabled;
	uint8_t pin;
	//String address;
		
	unsigned long lastCheckStatus;// = 0;//-flash_interval;
	int checkStatus_interval = 1000;	// il valore corretto per ogni tipo di sensore
									// è impostato nel costruttore
									// default 1 secondo

	unsigned lastUpdateAvailabilityStatus = 0;
	int updateAvailabilityStatus_interval = 60000; // intervallo minimo di aggiornamentto 
									// default 1 minuto

	unsigned lastUpdateStatus = 0;
	int updateStatus_interval = 60000; // intervallo minimo di aggiornamentto 
									// default 1 minuto
	
	void updateAvailabilityStatus();
	void updateAttributes();
	virtual void checkStatusChange();
	virtual void sendStatusUpdate();

	
	virtual void setStatus(String status);
	String getStatus();
	Sensor* getSensorFromId(int id);
	virtual void getJson(JsonObject& json);
	virtual void getStatusJson(JsonObject& json);
	virtual void loadChildren(JsonArray& jsonarray);
	virtual String toString();	
	virtual void init();
	virtual String getStatusText();
	virtual bool sendCommand(String command, String payload);

};

#endif