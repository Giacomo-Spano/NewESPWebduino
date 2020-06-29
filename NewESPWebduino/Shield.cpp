#include "Shield.h"
#include "Logger.h"
#include "SensorFactory.h"
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
//#include "MQTTMessage.h"
#include "ESPDisplay.h"

//#include "DoorSensor.h"

#ifndef ESP8266
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#endif

//const int command_open = 1;
//const int command_close = 2;

//extern void resetWiFiManagerSettings();
//extern bool mqtt_publish(MQTTMessage mqttmessage);
//extern bool mqtt_subscribe(String topic);


//extern void reboot(String reason);

Logger Shield::logger;
String Shield::tag = "Shield";

Shield::Shield()
{
	lastRestartDate = "";
	swVersion = "1.99";

	id = 0; //// inizializzato a zero perch� viene impostato dalla chiamata a registershield
	localPort = 80;
	serverName = /*"192.168.1.209"; //*/"giacomocasa.duckdns.org";
	mqttServer = /*"192.168.1.209"; //*/"giacomocasa.duckdns.org";
	mqttPort = 1883;
	serverPort = 8080;////9090;// 8080;
	shieldName = "shieldName";
	mqttUser = "giacomo";
	mqttPassword = "giacomo";
	oleddisplay = false;
	nexiondisplay = false;
	mqttsim = false;

	powerStatus = "on"; // da aggiungere
	lastCheckHealth = 0;
	rebootreason = "unknown";
}

Shield::~Shield()
{
	//sensors.clear();
}

void Shield::writeConfig() {

	logger.println(tag, F(">>writeConfig"));
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["name"] = getBoardName();
	json["http_server"] = getServerName();
	json["http_port"] = getServerPort();
	json["mqtt_server"] = getMQTTServer();
	json["mqtt_port"] = getMQTTPort();
	json["mqtt_user"] = getMQTTUser();
	json["mqtt_password"] = getMQTTPassword();
	json["mqtt_topic"] = getMQTTTopic();
	json["user"] = getUser();
	json["password"] = getPassword();
	json["user2"] = getUser2();
	json["password2"] = getPassword2();
	json["mqttsim"] = getMQTTSIM();

//	json["resetsettings"] = getResetSettings();
	json["shieldid"] = getShieldId();
	json["oled"] = getOledDisplay();

	File configFile = SPIFFS.open(F("/config.json"), "w");
	if (!configFile) {
		logger.print(tag, F("<<writeConfig"));
		return;
	}
	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
	logger.println(tag, F("config file writtten"));

	readConfig();

	logger.println(tag, F("<<writeConfig"));
}

void Shield::readConfig() {

	logger.println(tag, F(">>readConfig"));

	if (SPIFFS.begin()) {
		logger.print(tag, F("\n\t mounted file system"));
		if (SPIFFS.exists("/config.json")) {
			//file exists, reading and loading
			logger.print(tag, F("\n\t reading config file"));
			File configFile = SPIFFS.open("/config.json", "r");
			if (configFile) {
				logger.print(tag, F("\n\t opened config file"));
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);
				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success()) {
					logger.print(tag, F("\n\t parsed json"));

					setConfig(json);
				}
				else {
					logger.print(tag, F("\n\t failed to load json config"));
					//clean FS, for testing
					//SPIFFS.format();
					writeConfig();
				}
			}
		}
		else {
			logger.print(tag, F("\n\t json config does not exist"));
		}
	}
	else {
		logger.print(tag, F("\n failed to mount FS"));
	}
	logger.print(tag, F("\n<<readConfig"));
}

String Shield::getConfig() {

	logger.print(tag, F("\n>>Shield::getConfig\n"));
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	
	json["name"] = getBoardName();
	json["http_server"] = getServerName();
	json["http_port"] = getServerPort();
	json["mqtt_server"] = getMQTTServer();
	json["mqtt_port"] = getMQTTPort();
	json["mqtt_user"] = getMQTTUser();
	json["mqtt_password"] = getMQTTPassword();
	json["mqtt_topic"] = getMQTTTopic();
	json["user"] = getUser();
	json["password"] = getPassword();
	json["user2"] = getUser2();
	json["password2"] = getPassword2();
	json["mqttsim"] = getMQTTSIM();
	json["shieldid"] = getShieldId();
	json["oled"] = getOledDisplay();
	
	String str;
	json.printTo(str);
	logger.print(tag, F("\n<<Shield::getConfig\n"));
	return str;
}

bool Shield::setConfig(JsonObject& json) {

	logger.print(tag, F("\n>>Shield::setConfig\n"));
	
	if (json.containsKey("name")) {
		logger.print(tag, F("\n\t name: "));
		String str = json["name"];
		logger.print(tag, str);
		setBoardName(str);
	}
	if (json.containsKey("http_server")) {
		logger.print(tag, F("\n\t http_server: "));
		String str = json["http_server"];
		logger.print(tag, str);
		setServerName(str);
	}
	if (json.containsKey("http_port")) {
		logger.print(tag, F("\n\t http_port: "));
		String str = json["http_port"];
		logger.print(tag, str);
		setServerPort(json["http_port"]);
	}
	if (json.containsKey("mqtt_server")) {
		logger.print(tag, "\n\t mqtt_server: ");
		String str = json["mqtt_server"];
		logger.print(tag, str);
		setMQTTServer(json["mqtt_server"]);
	}
	if (json.containsKey("mqtt_port")) {
		logger.print(tag, F("\n\t mqtt_port: "));
		String str = json["mqtt_port"];
		logger.print(tag, str);
		setMQTTPort(json["mqtt_port"]);
	}
	if (json.containsKey("mqtt_user")) {
		logger.print(tag, F("\n\t mqtt_user: "));
		String str = json["mqtt_user"];
		logger.print(tag, str);
		setMQTTUser(json["mqtt_user"]);
	}
	if (json.containsKey("mqtt_password")) {
		logger.print(tag, F("\n\t mqtt_password: "));
		String str = json["mqtt_password"];
		logger.print(tag, str);
		setMQTTPassword(json["mqtt_password"]);
	}
	if (json.containsKey("mqtt_topic")) {
		logger.print(tag, F("\n\t mqtt_topic: "));
		String str = json["mqtt_topic"];
		logger.print(tag, str);
		setMQTTTopic(json["mqtt_topic"]);
	}
	if (json.containsKey("user")) {
		logger.print(tag, F("\n\t user: "));
		String str = json["user"];
		logger.print(tag, str);
		setUser(json["user"]);
	}
	if (json.containsKey("password")) {
		logger.print(tag, F("\n\t password: "));
		String str = json["password"];
		logger.print(tag, str);
		setPassword(json["password"]);
	}
	if (json.containsKey("user2")) {
		logger.print(tag, F("\n\t user2: "));
		String str = json["user2"];
		logger.print(tag, str);
		setUser2(json["user2"]);
	}
	if (json.containsKey("password2")) {
		logger.print(tag, F("\n\t password2: "));
		String str = json["password2"];
		logger.print(tag, str);
		setPassword2(json["password2"]);
	}
	if (json.containsKey("mqttsim")) {
		logger.print(tag, F("\n\t mqttsim: "));
		bool enabled = json["mqttsim"];
		logger.print(tag, Logger::boolToString(mqttsim));
		setMQTTSIM(json["mqttsim"]);
	}

	if (json.containsKey("shieldid")) {
		logger.print(tag, F("\n\t shieldid: "));
		String str = json["shieldid"];
		logger.print(tag, str);
		setShieldId(json["shieldid"]);
	}
	if (json.containsKey("oled")) {
		logger.print(tag, F("\n\t oled: "));
		bool enabled = json["oled"];
		logger.print(tag, Logger::boolToString(enabled));
		setOledDisplay(json["oled"]);
	}

	return true;
}


void Shield::init() {

	logger.print(tag, F("\n\t >> Shield::init"));

	setOledDisplay(false);

	readSensorFromFile();
	Sensor* sensor;

	//tftDisplay.init();
	//display.init();
	status = "restart";
	shieldEvent = "";

	if (getMQTTSIM()) {
		DynamicJsonBuffer jsonBuffer;
		JsonObject& json = jsonBuffer.createObject();
		json["type"] = "simsensor";
		json["sensorid"] = 0;
		json["sensorid"] = 0;	
#ifdef MQTTSIMSENSOR
		pMQTTSensor = new MQTTSimSensor(json);
		pMQTTSensor->setBoardName(getBoardName());
		pMQTTSensor->init();
#endif
	}

#ifdef ESP8266
	if (getOledDisplay()) {
		logger.print(tag, F("\n\t init ole display"));
		espDisplay.init(D3, D4);
	}


	//espDisplay.init(SDA, SCL); //sda scl
	//espDisplay.init(D4, D3); //sda scl
#endif

#ifdef TTGO
	espDisplay.init(4, 15);
#endif

	//espDisplay.init(SDA, SCL);

	logger.print(tag, F("\n\t << Shield::init"));
}

void Shield::clearAllSensors() {
}

Sensor* Shield::getSensor(int id) { // chiamata dall'esterno
	return getSensorFromId(id, sensors);
}

Sensor* Shield::getSensorFromId(int id, SimpleList<Sensor*>& list) {
	logger.print(tag, "\n\t >>Shield::getSensorFromId id=" + String(id));

	logger.print(tag, String(F("\n\t list.size=")) + String(list.size()));
	for (int i = 0; i < list.size(); i++)
	{
		logger.print(tag, "\n\t i=" + String(i));
		Sensor* sensor = (Sensor*)list.get(i);
		if (sensor->sensorid == id) {
			logger.print(tag, String(F("\n\t found")));
			return sensor;
		}
		logger.print(tag, String("\n\t childsensors.size=") + String(sensor->childsensors.size()));
		if (sensor->childsensors.size() > 0) {
			Sensor* sensor = getSensorFromId(id, sensor->childsensors);
			if (sensor != nullptr)
				return sensor;
		}
	}
	logger.print(tag, String(F("\n\t <<>Shield::getSensorFromId - NOT found!")));

	return nullptr;
}

void Shield::drawStatus() {
	if (getOledDisplay()) {
		espDisplay.drawString(0, 0, status);
	}
}

void Shield::drawMemory() {
	if (getOledDisplay()) {
		espDisplay.drawString(50, 0, "(" + String(freeMemory) + ")");
	}
}

void Shield::drawEvent() {
	if (getOledDisplay()) {
		espDisplay.drawString(0, 10, shieldEvent);
	}
}

void Shield::drawSWVersion() {
	if (getOledDisplay()) {
		String txt = logger.getStrDate();
		espDisplay.drawString(100, 0, "v" + swVersion);
	}
}

void Shield::drawDateTime() {

	if (getOledDisplay()) {
		String txt = logger.getStrDate();
		espDisplay.drawString(0, 20, txt);
	}
}

void Shield::drawSensorsStatus() {

	if (getOledDisplay()) {
		int lines = 3;
		for (int i = 0; i < sensors.size(); i++)
		{
			Sensor* sensor = (Sensor*)sensors.get(i);
			if (!sensor->enabled)
				continue;
			String txt = sensor->sensorname.substring(0, 5);
			espDisplay.drawString(0, lines++ * 10, txt + ": " + String(sensor->getStatusText()));
			for (int k = 0; k < sensor->childsensors.size(); k++) {
				Sensor* child = (Sensor*)sensor->childsensors.get(k);
				txt = child->sensorname.substring(0, 5);
				espDisplay.drawString(0, lines++ * 10, txt + ": " + String(child->getStatusText()));
			}
		}
	}
}

void Shield::drawString(int x, int y, String txt, int size, int color) {
	if (getOledDisplay()) {
		//tftDisplay.drawString(x, y, txt, 1, ST7735_WHITE);
		//tftDisplay.drawString(int row, int col, String txt, int size, int color);
	}
}

/*Just as a heads up, to use the "drawXBitmap" function, I also had
to redefine the array from "static unsigned char" to "static const uint8_t PROGMEM"
in my image file.If I just used the original file, then garbage would appear on my 32x32 gird.
I think the array has to be defined as "PROGMEM" because the "drawXBitmap" function uses the
"pgm_read_byte" function, which reads a byte from program memory.I also had to include
the line "#include <avr/pgmspace.h>".After that, everything worked fine for me.*/

//#include <avr/pgmspace.h>
/*#define temperature_width 32
#define temperature_height 32
static const uint8_t temperature_bits[] = {
	0x00, 0x80, 0x01, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x70, 0x0e, 0x00,
	0x00, 0x18, 0x18, 0x00, 0x00, 0x08, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x18, 0x10, 0x00, 0x00, 0x78, 0x10, 0x00, 0x00, 0x18, 0x10, 0x00,
	0x00, 0x18, 0x10, 0x00, 0x00, 0x78, 0x10, 0x00, 0x00, 0x18, 0x10, 0x00,
	0x00, 0x08, 0x10, 0x00, 0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00,
	0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00,
	0x00, 0xc8, 0x13, 0x00, 0x00, 0xc8, 0x13, 0x00, 0x00, 0xcc, 0x33, 0x00,
	0x00, 0xc4, 0x23, 0x00, 0x00, 0xe6, 0x67, 0x00, 0x00, 0xe2, 0x67, 0x00,
	0x00, 0xe6, 0x67, 0x00, 0x00, 0xe6, 0x67, 0x00, 0x00, 0xe6, 0x67, 0x00,
	0x00, 0xce, 0x71, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x3c, 0x3c, 0x00,
	0x00, 0xf0, 0x0f, 0x00, 0x00, 0xc0, 0x03, 0x00 };*/

void Shield::clearScreen() {
	if (getOledDisplay()) {
		//tftDisplay.clear();

		//tftDisplay.drawXBitmap(90, 50, temperature_bits, temperature_width, temperature_height, 0xF800/*ST3577_RED*/);
	}
}

void Shield::parseMessageReceived(String topic, String message) {

	logger.print(tag, F("\n\n\t >>parseMessageReceived"));

	logger.print(tag, String(F("\n\n\t topic: ")) + topic);
	logger.print(tag, String(F("\n\t message: ")) + message);
	logger.print(tag, "\n");

	String subtopic = topic;
	subtopic.replace("ESPWebduino/" + getBoardName() + "/command/", "");

	int index = subtopic.indexOf('/');
	if (index >= 0) {
		String type = subtopic.substring(0, index);
		logger.print(tag, "\ntype=" + type);
		subtopic.replace(type + '/', "");
		int index = subtopic.indexOf("/");
		if (index >= 0) {
			String id = subtopic.substring(0, index);
			logger.print(tag, "\nid=" + id);
			subtopic.replace(id + '/', "");
			String command = subtopic;
			logger.print(tag, "\ncommand=" + command);
			//sendSensorCommand(type, id.toInt(), command, message);
			sendSensorCommand(/*type, */id.toInt(), command, message);
		}
	}

	else {
		logger.print(tag, F("\n\t PARSE NOT FOUND"));
	}

	logger.printFreeMem(tag, F("parseMessageReceived"));
	logger.print(tag, F("\n\t <<parseMessageReceived\n"));
}




void Shield::getJson(JsonObject& json) {
	logger.print(tag, F("\n\t >>Shield::getJson"));
	//json["MAC"] = getMACAddress();
	json["swversion"] = swVersion;
	json["lastreboot"] = lastRestartDate;
	json["lastcheckhealth"] = String(lastCheckHealth);
	json["freemem"] = String(freeMemory);
	json["localIP"] = localIP;
	json["localPort"] = String(Shield::getLocalPort());
	logger.print(tag, F("\n\t <<Shield::getJson json="));
	logger.printJson(json);
}

void Shield::setStatus(String txt) {
	if (status.equals(txt))
		return;
	status = txt;
	invalidateDisplay();
}

void Shield::setEvent(String txt) {

	if (getOledDisplay()) {
		if (shieldEvent.equals(txt))
			return;
		shieldEvent = txt;
		invalidateDisplay();
	}
}

void Shield::invalidateDisplay() {
	if (getOledDisplay()) {
		espDisplay.clear();

		drawStatus();
		drawMemory();
		drawSWVersion();
		drawEvent();
		drawDateTime();
		drawSensorsStatus();
		espDisplay.update();
	}
}

void Shield::setFreeMem(int mem)
{
	freeMemory = mem;
}

void Shield::checkStatus()
{
#ifdef MQTTSIMSENSOR
	if (getMQTTSIM() && pMQTTSensor != nullptr) {
		pMQTTSensor->checkStatusChange();
	}	
#endif

	checkSensorsStatus();

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastTimeUpdate;

	if (timeDiff > 1000) {
		//logger.print(tag, "Shield::checkStatus");
		lastTimeUpdate = currMillis;
		invalidateDisplay();
	}
}

void Shield::sendSensorCommand(int id, String command, String payload)
{
	logger.print(tag, F("\n\t >>Shield::sendSensorCommand"));

	logger.print(tag, String(F("\n\t\ id=")) + id);
	logger.print(tag, String(F("\n\t\ command=")) + command);
	logger.print(tag, String(F("\n\t\ payload=")) + payload);

	Sensor* sensor = getSensorFromId(id, sensors);
	if (sensor != nullptr)
		sensor->sendCommand(command, payload);

	logger.print(tag, F("\n\t <<Shield::sendSensorCommand"));
}

void Shield::checkSensorsStatus()
{
	for (int i = 0; i < sensors.size(); i++)
	{
		Sensor* sensor = (Sensor*)sensors.get(i);
		checkSensorStatus(sensor);
		/*if (!sensor->enabled) {
			continue;
		}
		sensor->updateAvailabilityStatus();
		if (sensor->checkStatusChange())
			sensor->sendStatusUpdate();*/
	}
}

void Shield::checkSensorStatus(Sensor* sensor)
{
	sensor->updateAvailabilityStatus();
	sensor->checkStatusChange();

	if (sensor->childsensors.size() > 0) {
		for (int i = 0; i < sensor->childsensors.size(); i++)
		{
			Sensor* child = (Sensor*)sensor->childsensors.get(i);
			checkSensorStatus(child);
		}
	}
}

bool Shield::requestTime() {
	return true;
	/*logger.print(tag, F("\n\t >>requestTimeFromServer"));
	setEvent(F("request server setting.."));
	timeNeedToBeUpdated = false;
	lastTimeRequest = millis();
	Command command;
	bool res = command.requestTime(getMACAddress());
	if (res)
		timeRequestInprogress = true;
	else
		timeRequestInprogress = false;
	logger.print(tag, F("\n\t <<requestTimeFromServer res="));
	logger.print(tag, Logger::boolToString(res));
	return res;*/
}

void Shield::checkTimeUpdateStatus() {
	unsigned long timediff;

	timediff = millis() - lastTimeRequest;
	if (timediff > timeSync_interval) {
		timeNeedToBeUpdated = true;
	}

	if (timeRequestInprogress && timediff > timeRequest_timeout) {
		timeNeedToBeUpdated = true;
	}

	if (timeNeedToBeUpdated && !timeRequestInprogress) {
		logger.println(tag, F("\n\n\n\-----------TIME UPDATE TIMEOUT --------\n\n"));
		setEvent(F("Request time"));
		bool res = requestTime();
		if (!res) { // se fallisce rirpova tra un minuto
			logger.println(tag, F("\n request setting failed"));
			lastTimeRequest = millis() - timeSync_interval + 1 * 60 * 1000;
		}
	}
}

void Shield::readSensorFromFile() {
	logger.print(tag, F("\n\t >>readSensorFromFile"));

	if (SPIFFS.begin()) {
		logger.print(tag, F("\n\t mounted file system"));
		if (SPIFFS.exists("/sensors.json")) {
			//file exists, reading and loading
			logger.print(tag, F("\n\t reading config file"));
			File configFile = SPIFFS.open("/sensors.json", "r");
			if (configFile) {
				logger.print(tag, F("\n\t opened config file"));
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);
				configFile.readBytes(buf.get(), size);

				DynamicJsonBuffer jsonBuffer;
				JsonArray& jsonarray = jsonBuffer.parseArray(buf.get());

				if (jsonarray.success()) {
					logger.print(tag, F("\n\t parsed json"));
					jsonarray.printTo(Serial);
					loadSensors(jsonarray);
				}
				else {
					logger.print(tag, F("\n\t failed to load sensors file"));
				}
			}
		}
	}
	else {
		logger.print(tag, F("\n\t failed to mount FS"));
		//SPIFFS.format();
	}
	logger.print(tag, F("\n\t <<readSensorFromFile\n\n"));
}

bool Shield::writeSensorsToFile() {

	logger.print(tag, F("\n\t>>Shield::writeSensorToFile\n"));

	File configFile = SPIFFS.open("/sensors.json", "w");
	if (!configFile) {
		logger.print(tag, F("\n\t failed to open config file for writing"));
		return false;
	}
	String str = getSensors();
	configFile.print(str);
	configFile.close();

	logger.println(tag, F("<<Shield::writeSensorToFile\n"));
	return true;
}


String Shield::getSensors() {

	logger.print(tag, F("\n>>Shield::getSensors\n"));
	logger.printFreeMem(tag, F("++getSensors"));
	//StaticJsonBuffer<2000> jsonBuffer;
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	String result = "[";
	for (int i = 0; i < sensors.size(); i++)
	{
		if (i != 0)
			result += ",";
		Sensor* sensor = (Sensor*)sensors.get(i);
		//String str = sensor->getStrJson();		
		sensor->getJson(json);
		logger.print(tag, "\n return from getjson");
		String str = "";
		json.printTo(str);
		logger.print(tag, "\n post print tojson");
		logger.print(tag, str);
		result += str;
		logger.print(tag, "\n result=" + result);
		//result += str;
		//json
	}

	result += "]";
	logger.print(tag, "\n result=" + result);
	logger.printFreeMem(tag, F("--getSensors"));
	logger.print(tag, F("\n<<Shield::getSensors\n"));
	return result;
}

bool Shield::updateSensor(String jsonstr/*JsonObject& json*/) {

	logger.print(tag, F("\n\n\t>>Shield::updateSensor\n"));

	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(jsonstr);
	if (!json.success()) {
		logger.print(tag, F("\n\n\tfailed to parse json\n"));
		return false;
	}
	int sensorid = json["sensorid"].as<int>();
	String type = json["type"].asString();
	logger.print(tag, "\n\tsensorid=" + String(sensorid));
	logger.print(tag, "\n\ttype=" + type);


	if (json.containsKey("sensorid") && json.containsKey("type") && json.containsKey("pin")) {
		if (sensorid == 0) {
			return addJsonSensor(json);
		}
		else {
			logger.println(tag, F("\n Update existing sensor\n"));
			// remove current sensor
			for (int i = 0; i < sensors.size(); i++) {
				Sensor* sensor = (Sensor*)sensors.get(i);
				if (sensor->sensorid == sensorid) {
					sensors.remove(i);
					break;
				}
			}
			return addJsonSensor(json);
		}
	}
	else {
		logger.print(tag, F("\n\n\tbad json\n"));
	}
	return false;
}

bool Shield::addJsonSensor(JsonObject& json) {

	logger.printFreeMem(tag, "loadSensors");
	
	SensorFactory sf;
	Sensor* sensor = sf.createSensor(json);
	if (sensor == nullptr) {
		logger.print(tag, "create Sensor Failed!");
		return false;
	}
	
	sensors.add(sensor);
	writeSensorsToFile();

	delay(300);
	ESP.restart(); //riavvia dopo aggiunta di un sensore
	return true;
}


bool  Shield::loadSensors(JsonArray& jsonarray) {

	logger.print(tag, F("\n\n\t>>loadSensors"));
	logger.printFreeMem(tag, F("loadSensors"));

	//clearAllSensors(); // serve per inizializzare

	for (int i = 0; i < jsonarray.size(); i++) {

		logger.print(tag, "\n\n\t SENSOR: " + String(i) + "\n");
		jsonarray[i].printTo(Serial);
		int sensorid = jsonarray[i]["sensorid"].as<int>();
		String type = jsonarray[i]["type"].asString();
		type.replace("\r\n", "");

		logger.print(tag, "\n\tsensorid=" + String(sensorid));
		logger.print(tag, "\n\ttype=" + type);

		//String jsonstr;
		//jsonarray[i].printTo(jsonstr);

		//Sensor* sensor = nullptr;
		//sensor = createSensor(/*sensorid, type, jsonstr*/jsonarray[i]);
		//sensor->sensorid = sensorid;

		SensorFactory sf;
		Sensor* sensor = sf.createSensor(jsonarray[i]);
		if (sensor == nullptr) {
			logger.print(tag, "create Sensor Failed!");
			continue;
		} else {
			sensors.add(sensor);
		}
	
		logger.printFreeMem(tag, "\n\n\tloadSensors i=" + String(i));
		/*if (sensor != nullptr) {
			//sensor->createSensor(jsonstr);
			sensors.add(sensor);
		}
		else {
			logger.print(tag, "create Sensor Failed!");
			continue;
		}*/
	}

	for (int i = 0; i < sensors.size(); i++)
	{
		Sensor* sensor = (Sensor*)sensors.get(i);
		sensor->init();
	}

	logger.print(tag, F("\n\t<<loadSensors\n"));
	return true;
}

void Shield::writeRebootReason() {

	logger.println(tag, " >>writeRebootReason config");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["rebootreason"] = getRebootReason();

	File configFile = SPIFFS.open("/reason.json", "w");
	if (!configFile) {
		logger.print(tag, "\n\t failed to open rebootreason file for writing");
		return;
	}

	json.printTo(Serial);
	json.printTo(configFile);
	configFile.close();
	logger.print(tag, "\n\t reason file written");
	logger.println(tag, "writeRebootReason config");
	//end save
}

void Shield::readRebootReason() {

	logger.println(tag, F(">>readRebootReason"));

	if (SPIFFS.begin()) {
		logger.print(tag, F("\n\t mounted file system"));
		if (SPIFFS.exists("/config.json")) {
			//file exists, reading and loading
			logger.print(tag, F("\n\t reading config file"));
			File configFile = SPIFFS.open("/reason.json", "r");
			if (configFile) {
				logger.print(tag, F("\n\t opened config file"));
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);
				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success()) {
					logger.print(tag, F("\n\t parsed json"));

					if (json.containsKey("rebootreason")) {
						Serial.println("rebootreason: ");
						String str = json["rebootreason"];
						logger.print(tag, str);
						String reason = json["rebootreason"];
						setRebootReason(str);
					}
				}
				else {
					logger.print(tag, F("\n\t failed to load json config"));
					//clean FS, for testing
					SPIFFS.format();
					writeConfig();
				}
			}
		}
	}
	else {
		logger.print(tag, F("failed to mount FS"));
	}
	logger.print(tag, F("<<readRebootReason"));
}
