#include "Sensor.h"
#include "Shield.h"
#include "Logger.h"
#include "MQTTMessage.h"
#include "SensorFactory.h"

Logger Sensor::logger;
String Sensor::tag = "Sensor";

String Sensor::STATUS_IDLE = "idle";

extern bool mqtt_publish(String topic, String message);

void Sensor::addType0CallBack(AbstractCallBack* ptr) {
	type0CallBackPointer = ptr;
}

void Sensor::callBack(int sensorid, String status, String oldstatus) {
	logger.print(tag, "\n\t >> Sensor::callBack sendor id=" + String(sensorid) + "status=" + status + "oldstatus=" + oldstatus);
	logger.print(tag, "\n\t << Sensor::callBack");
};

Sensor::Sensor() {

	status = STATUS_IDLE;
	oldStatus = STATUS_IDLE;

}

Sensor::Sensor(JsonObject& json)
{
	logger.print(tag, F("\n\t >>Sensor::Sensor"));

	status = STATUS_IDLE;
	oldStatus = STATUS_IDLE;

	String type = json["type"];
	type.replace("\r\n", "");

	this->sensorid = json["sensorid"];
	logger.print(tag, "\n\t sensorid=" + String(sensorid));
	if (json.containsKey(F("pin"))) {
		String strPin = json[F("pin")];
		strPin.replace("\r\n", ""); // importante!!
		this->pin = Shield::pinFromStr(strPin);
	}
	logger.print(tag, "\n\t pin=" + String(pin));
	if (json.containsKey("enabled"))
		this->enabled = json["enabled"];
	if (json.containsKey("name"))
		this->sensorname = json["name"].asString();

	if (json.containsKey("children")) {
		String str = json["children"];
		DynamicJsonBuffer jsonBuffer;
		JsonArray& jsonarray = jsonBuffer.parseArray(str.c_str());
		if (jsonarray.success()) {
			logger.print(tag, F("\n\t parsed children json"));
			jsonarray.printTo(Serial);
			loadChildren(jsonarray);
		}
		else {
			childsensors.clear();
			logger.print(tag, F("\n\t failed to parsed children json"));
		}
	}

	checkStatus_interval = 10000;//60000; // 60 seconds
	lastCheckStatus = 0;// = 0;//-flash_interval;
	logger.print(tag, F("\n\t <<Sensor::Sensor"));
}

Sensor::~Sensor()
{
	for (int i = 0; i < childsensors.size(); i++) {
		delete jsonBuffer[i];
	}
	childsensors.clear();
}

void Sensor::setStatus(String _status) {

	oldStatus = status;
	status = _status;

	if (oldStatus != status) {
		logger.print(tag, "\n\t STATUS CHANGE - sensorid: " + String(sensorid) + " name: " + sensorname + " status: " + status + " oldstatus: " + oldStatus);
		if (type0CallBackPointer != nullptr) {
			type0CallBackPointer->callBack(sensorid, status, oldStatus);
		}
	}
}

String Sensor::getStatus() {

	return status;
}

bool Sensor::checkStatusChange()
{
	if (!status.equals(oldStatus)) {
		//logger.print(tag, "\n\t STATUS CHANGE - sensorid: " + String(sensorid) + " name: " + sensorname + " status: " + status);
		return true;
	}
	unsigned long timeDiff = millis() - lastUpdateAvailabilityStatus;
	if (timeDiff > updateStatus_interval || timeDiff < 0) { // se è passato troppo tempo aggiorna comunque
		logger.print(tag, "\n\t UPDATE STATUS TIMEOUT - sensorid: " + String(sensorid) + " name: " + sensorname + " status: " + status);
		return true;
	}
	return false;
}

void Sensor::sendStatusUpdate(String boardname)
{
	logger.print(tag, F("\n\t>>Sensor::::sendStatusUpdate"));

	unsigned long currMillis = millis();
	lastUpdateStatus = currMillis;

	String topic = "ESPWebduino/" + boardname + "/" + type + "/" + sensorid + "/status";
	if (mqtt_publish(topic, status))
		oldStatus = status;
	updateAttributes(boardname);
	logger.print(tag, F("\n\t<<Sensor::::sendStatusUpdate"));
}

void Sensor::updateAvailabilityStatus(String boardname) {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastUpdateAvailabilityStatus;

	if (timeDiff > updateAvailabilityStatus_interval || timeDiff < 0) {
		logger.print(tag, "\n\n\t >>>> send availability status update");
		lastUpdateAvailabilityStatus = currMillis;
		String topic = "ESPWebduino/" + boardname + "/" + type + "/" + sensorid + "/availability";
		mqtt_publish(topic, "online");
		logger.print(tag, "\n");
	}
}

void Sensor::updateAttributes(String boardname) {

	logger.print(tag, "\n\n\t >>>> send atttributes");
	String topic = "ESPWebduino/" + boardname + "/" + type + "/" + sensorid + "/attributes";
	//String topic = "ESPWebduino/myboard1/" + type + "/" + sensorid + "/attributes";
	//String strJson = getStrJson();
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	getJson(json);
	String str = "";
	json.printTo(str);
	mqtt_publish(topic, str/*strJson*/);
}

String Sensor::toString()
{
	String str = "sensor: " + sensorname + ";type: " + type + ";sensorid: " + sensorid;
	return str;
}

Sensor* Sensor::getSensorFromId(int id)
{
	logger.print(tag, F("\n\t >>Sensor::getSensorFromId "));

	if (sensorid == id)
		return (Sensor*)this;

	if (childsensors.size() > 0) {
		for (int i = 0; i < childsensors.size(); i++) {
			logger.print(tag, "\n\t i= " + String(i));
			Sensor* child = (Sensor*)childsensors.get(i);
			Sensor* sensor = child->getSensorFromId(id);
			if (sensor->sensorid == id)
				return (Sensor*)this;
		}
	}
	logger.println(tag, F("<<Sensor::getSensorFromId"));
	return nullptr;
}

void Sensor::getJson(JsonObject& json) {

	logger.print(tag, "\n\t>>Sensor::getJson id=" + String(sensorid) + " name=" + sensorname + " type=" + type);
	json["sensorid"] = sensorid;
	json["status"] = status;
//	json["addr"] = address;
	json["statustext"] = getStatusText();
	json["pin"] = Shield::getStrPin(pin);
	json["name"] = sensorname;
	json["enabled"] = enabled;
	json["type"] = type;

	if (childsensors.size() > 0) {
		JsonArray& children = json.createNestedArray("children");
		for (int i = 0; i < childsensors.size(); i++) {
			Sensor* child = (Sensor*)childsensors.get(i);
			JsonObject& childjson = jsonBuffer[i]->createObject();
			child->getJson(childjson);
			children.add(childjson);
		}
		boolean res = json.set("children", children);
	}
	logger.print(tag, "\n\n\t<<Sensor::getJson sensorid=" + String(sensorid) + " \n\n");
}

void Sensor::getStatusJson(JsonObject& json) {

	logger.print(tag, "\n\t>>Sensor::getStatusJson id=" + String(sensorid) + " name=" + sensorname + " type=" + type);
	json["sensorid"] = sensorid;
	json["status"] = status;

	if (childsensors.size() > 0) {
		JsonArray& children = json.createNestedArray("children");
		for (int i = 0; i < childsensors.size(); i++) {
			Sensor* child = (Sensor*)childsensors.get(i);
			JsonObject& childjson = jsonBuffer[i]->createObject();
			child->getStatusJson(childjson);
			children.add(childjson);
		}
		boolean res = json.set("children", children);
	}
	logger.print(tag, "\n\n\t<<Sensor::getStatusJson sensorid=" + String(sensorid) + " \n\n");
}


void Sensor::loadChildren(JsonArray& jsonarray)
{
	logger.print(tag, "\n\n\t>>Sensor::loadChildren jsonarray.size()=" + String(jsonarray.size()));

	for (int i = 0; i < jsonarray.size(); i++) {

		int sensorid = jsonarray[i]["sensorid"].as<int>();
		String type = jsonarray[i]["type"].asString();
		logger.print(tag, "\n\tsensorid=" + String(sensorid));
		logger.print(tag, "\n\ttype=" + type);

		SensorFactory sf;
		Sensor* child = sf.createSensor(jsonarray[i]);
		if (child != nullptr) {
			child->init();
			childsensors.add(child);
		}
	}
	logger.print(tag, "\n\t>>Sensor::loadChildren jsonarray.size()=" + String(childsensors.size()));
}

void Sensor::init()
{
	status = STATUS_IDLE;
	if (childsensors.size() == 0)
		return;
	for (int i = 0; i < childsensors.size(); i++) {
		Sensor* child = childsensors.get(i);
		if (child != nullptr) {
			child->init();
		}
	}
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
	return mqtt_publish(topic, message);
}

