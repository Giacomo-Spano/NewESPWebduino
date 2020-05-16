#include "Sensor.h"
#include "Shield.h"
#include "Logger.h"
#include "MQTTMessage.h"

Logger Sensor::logger;
String Sensor::tag = "Sensor";

extern bool mqtt_publish(String topic, String message);

Sensor::Sensor(int id, uint8_t pin, bool enabled, String address, String name)
{
	this->sensorid = id;
	this->pin = pin;
	this->enabled = enabled;
	this->address = address;
	this->sensorname = name;

	checkStatus_interval = 10000;//60000; // 60 seconds
	lastCheckStatus = 0;// = 0;//-flash_interval;
}

Sensor::Sensor()
{
}

Sensor::~Sensor()
{
	//childsensors.clear();
}

void Sensor::setStatus(String _status) {
	
	oldStatus = status;
	status = _status;
}

void Sensor::checkStatusChange()
{
	//logger.print(tag, F("\n\t\t checkStatusChange"));
	if (!status.equals(oldStatus)) {
		//logger.print(tag, F("\n\t\t SEND STATUS\n\n"));
		sendStatusUpdate();
		updateAttributes();
	} 	
}

void Sensor::sendStatusUpdate()
{
	logger.print(tag, F("\n\t\t sendStatusUpdate"));
	String topic = "ESPWebduino/myboard1/" + type + "/status";
	if (mqtt_publish(topic, status))
		oldStatus = status;
}



void Sensor::updateAvailabilityStatus() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastUpdateAvailabilityStatus;
	
	if (timeDiff > updateAvailabilityStatus_interval) {
		logger.print(tag, "\n\n\t >>>> send availability status update");
		lastUpdateAvailabilityStatus = currMillis;
		String topic = "ESPWebduino/myboard1/" + type + "/"+ sensorid + "/availability";
		mqtt_publish(topic, "online");
		logger.print(tag, "\n");
	}
}

void Sensor::updateAttributes() {
	
	logger.print(tag, "\n\n\t >>>> send atttributes");
	String topic = "ESPWebduino/myboard1/" + type + "/" + sensorid + "/attributes";
	String strJson = getStrJson();
	mqtt_publish(topic, strJson);
}



void Sensor::show() {

	/*logger.print(tag, "\n\t sensorid=");
	logger.print(tag, String(sensorid));
	logger.print(tag, "\n\t name=");
	logger.print(tag, sensorname);
	logger.print(tag, "\n\t addr= ");
	logger.print(tag, address);
	logger.print(tag, "\n\t enabled= ");
	logger.print(tag, String(enabled));
	logger.print(tag, "\n\t pin= ");
	logger.print(tag, String(pin));*/
}

String Sensor::toString()
{
	String str = "sensor: " + sensorname + ";type: " + type + ";sensorid: " + sensorid;
	/*if (childsensors.size() > 0)
		str += ";chidren:" + String(childsensors.size());*/
	return str;
}

Sensor * Sensor::getSensorFromId(int id)
{
	logger.print(tag, F("\n\t >>Sensor::getSensorFromId "));

	if (sensorid == id)
		return (Sensor*)this;
	
	/*if (childsensors.size() > 0) {
		for (int i = 0; i < childsensors.size(); i++) {
			//logger.print(tag, "\n\t i= " + String(i));
			Sensor* child = (Sensor*)childsensors.get(i);
			Sensor* sensor = child->getSensorFromId(id);
			if (sensor->sensorid == id)
				return (Sensor*)this;
		}
	}*/
	logger.println(tag, F("<<Sensor::getSensorFromId"));
	return nullptr;
}

String prova;

void Sensor::getJson(JsonObject& json) {

	logger.println(tag, F(">>Sensor::getJson\n"));
	json["sensorid"] = sensorid;
	json["status"] = status;
	json["addr"] = address;
	json["statustext"] = getStatusText();

	json["pin"] = Shield::getStrPin(pin);
	json["name"] = sensorname;
	json["enabled"] = String(enabled);
	json["type"] = type;


		
	/*if (childsensors.size() > 0) {

		
		for (int i = 0; i < childsensors.size(); i++) {
			Sensor* child = (Sensor*)childsensors.get(i);
			JsonArray& children = json["children"];
			JsonObject& childjson = children[i];

			child->getJson(childjson);
		}
	}*/
	logger.printJson(json);

	logger.println(tag, F("<<Sensor::getJson"));
}

String Sensor::getStrJson() {

	logger.println(tag, F(">>Sensor::getStrJson\n"));

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	getJson(json);
	
	/*if (childsensors.size() > 0) {
		
		JsonArray& children = json.createNestedArray("children");
		for (int i = 0; i < childsensors.size(); i++) {
			logger.println(tag, F("\nchild "));
			logger.println(tag, i);

			Sensor* child = (Sensor*)childsensors.get(i);
			
			StaticJsonBuffer<300> jsonBuffer;
			JsonObject& childjson = jsonBuffer.createObject();
			child->getJson(childjson);
			children.add(childjson);
		}
		boolean res = json.set("children", children);
		logger.print(tag, F("res="));
		logger.print(tag, Logger::boolToString(res));
	}*/
	//logger.printJson(json);

	String str;
	json.printTo(str);
	logger.print(tag, str);

	logger.println(tag, F("str="));
	logger.println(tag, str);
	logger.println(tag, F("<<Sensor::getStrJson"));
	return str;
}

void Sensor::loadChildren(JsonArray& json)
{
}

void Sensor::init()
{
}


String Sensor::getStatusText()
{
	return status;
}

bool Sensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>Sensor::receiveCommand"));
	logger.print(tag, String(F("\n\t >>command=")) + command);
	if (command.equals("requestsensorstatus")) {// richiesta stato di un singolo sensore
		/*logger.print(tag, F("\n\t requestsensorstatus"));
		DynamicJsonBuffer jsonBuffer;
		JsonObject& jsonresult = jsonBuffer.createObject();
		getJson(jsonresult);
		String jsonStr;
		logger.printJson(jsonresult);
		jsonresult.printTo(jsonStr);
		logger.print(tag, F("\n\t jsonstr="));
		logger.print(tag, jsonStr);
		return sendCommandResponse(uuid, jsonStr);*/
	}
	logger.print(tag, F("\n\t <<Sensor::receiveCommand"));
	return false;
}



bool Sensor::receiveCommand(String command, int id, String uuid, String jsoncmd)
{
	logger.print(tag, F("\n\t >>Sensor::receiveCommand"));
	logger.print(tag, String(F("\n\t >>command=")) + command);
	if (command.equals("requestsensorstatus")) {// richiesta stato di un singolo sensore
		logger.print(tag, F("\n\t requestsensorstatus"));
		DynamicJsonBuffer jsonBuffer;
		JsonObject& jsonresult = jsonBuffer.createObject();
		getJson(jsonresult);
		String jsonStr;
		logger.printJson(jsonresult);
		jsonresult.printTo(jsonStr);
		logger.print(tag, F("\n\t jsonstr="));
		logger.print(tag, jsonStr);
		return sendCommandResponse(uuid, jsonStr);
	}
	logger.print(tag, F("\n\t <<Sensor::receiveCommand"));
	return false;
}

bool Sensor::sendCommandResponse(String uuid, String response)
{
	logger.print(tag, "\n\t sendCommandResponse uuid=" + uuid + "response" + response);
	String topic = "toServer/response/" + uuid + "/success";
	String message = response;
	return mqtt_publish(topic,message);
}

