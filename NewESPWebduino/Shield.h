#ifndef _SHIELD_h
#define _SHIELD_h

#include <Arduino.h>
#include <string.h>
#include "Sensor.h"
#include <SimpleList.h>
#include "ESPDisplay.h"

class Shield
{
protected:
	int localPort;

	String name;
	String serverName;//[serverNameLen];
	int serverPort;
	String mqttServer;
	int mqttPort;
	String user;
	String password;
	String user2;
	String password2;
	//

	String shieldName;
	bool nexiondisplay;
	bool oleddisplay;
	bool loragatewayEnabled;

	bool mqttMode;
	bool configMode;
	bool resetSettings;
	String mqttUser;
	String mqttPassword;
	String mqttTopic;
	String rebootreason;
	String loraMessage;

	

public:
	int freeMemory = 0;
	int checkHealth_timeout = 86400000 / 2;  //12 ore
	unsigned long lastCheckHealth;
	String swVersion;

	bool settingFromServerReceived = false;

	unsigned long settingsRequest_interval = 1 * 60 * 1000;
	const int settingsRequest_timeout = 1 * 60 * 1000;
	unsigned long lastSettingRequest = 0;
	bool settingsNeedToBeUpdated = true;
	bool settingsRequestInprogress = false;

	const int timeSync_interval = 5 * 60 * 1000;// *12;// 60 secondi * 15 minuti
	const int timeRequest_timeout = 1 * 60 * 1000;
	unsigned long lastTimeRequest = 0;
	bool timeNeedToBeUpdated = true;
	bool timeRequestInprogress = false;
	unsigned long lastTimeUpdate = 0; // usato dall'orologio per aggiornare il display ogni secondo
	int id;// = 0; // inizializzato a zero perch� viene impostato dalla chiamata a registershield
	String powerStatus; // power
	String lastRestartDate;
	//static const int maxSensorNum = 10;
	bool loadSensors(JsonArray& json);
	void readSensorFromFile();
	bool writeSensorsToFile();
	String getSensors();
	bool updateSensor(JsonObject& json);
	bool addSensor(JsonObject& json);
	void parseMessageReceived(String topic, String message);
	void drawString(int x, int y, String txt, int size, int color);
	void drawDateTime();
	void drawStatus();
	void drawMemory();
	void drawEvent();
	void drawSensorsStatus();
	void drawSWVersion();
	void clearScreen();
	void invalidateDisplay();
	//bool getNextSensorId();
	//int getNextId();
	//void writeNextId(int nextid);

	void checkTimeUpdateStatus();
	
	void readConfig();
	void writeConfig();
	void readRebootReason();
	void writeRebootReason();

	//void setLoRaGateway(bool enable, String address, bool serverenabled);
	//bool sendLoRaMessage(String payload);

	void setFreeMem(int mem);

private:
	static String tag;
	static Logger logger;

	String status;
	String shieldEvent;
	ESPDisplay espDisplay;

	String oldDate;

protected:

	bool onShieldSettingsCommand(JsonObject& json);
	bool onPowerCommand(JsonObject& json);
	void checkSensorsStatus();
	void checkSensorStatus(Sensor* sensor);


	ESPDisplay display;
#ifdef ESP8266
	//TFTDisplay tftDisplay;
#endif

public:

	SimpleList<Sensor*> sensors = SimpleList<Sensor*>();

	void sendSensorCommand(int id, String command, String payload);

	Shield();
	~Shield();
	void init();
	virtual void getJson(JsonObject& json);
	void checkStatus();
	void setStatus(String txt);
	void setEvent(String txt);
	//bool receiveCommand(String jsonStr);
	bool requestTime();
	String localIP;
	void clearAllSensors();
	Sensor* getSensorFromId(int id, SimpleList<Sensor*>& sensors);

	int getShieldId() {
		return id;
	}

	void setShieldId(int shieldid) {
		id = shieldid;
	}
	String getLastRestartDate() { return lastRestartDate; }
	void setLastRestartDate(String date) { lastRestartDate = date; }

	String getModel()
	{
#ifdef ESP8266
		return "ESP8266";
#endif
#ifdef ESP32
		return "ESP32";
#endif

	}

	String getMACAddress()
	{
#ifdef ESP8266
		//return WiFi.macAddress();
#else
		uint8_t baseMac[6];
		// Get MAC address for WiFi station
		esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
		char baseMacChr[18] = { 0 };
		sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

		return String(baseMacChr);
#endif
	}

	String getSWVersion()
	{
		return swVersion;
	}

	int getServerPort()
	{
		return serverPort;
	}

	void setServerPort(int port)
	{
		logger.print(tag, "\n\t >>setServerPort");
		serverPort = port;
		logger.print(tag, "\n\t <<serverPort=" + String(serverPort));
	}

	int getNextionDisplay()
	{
		return nexiondisplay;
	}

	void setNextionDisplay(bool enable)
	{
		logger.print(tag, "\n\t >>setnextionDisplay");
		nexiondisplay = enable;
		logger.print(tag, "\n\t <<setnextionDisplay=" + Logger::boolToString(nexiondisplay));
	}

	bool getOledDisplay()
	{
		return oleddisplay;
	}

	void setOledDisplay(bool enable)
	{
		logger.print(tag, "\n\t >>setOledDisplay");
		oleddisplay = enable;
		logger.print(tag, "\n\t <<setOledDisplay=" + Logger::boolToString(oleddisplay));
	}


	

	static String getStrPin(uint8_t pin)
	{
#ifdef ESP8266
		if (pin == D0)
			return "D0";
		if (pin == D1)
			return "D1";
		if (pin == D2)
			return "D2";
		if (pin == D3)
			return "D3";
		if (pin == D4)
			return "D4";
		if (pin == D5)
			return "D5";
		if (pin == D6)
			return "D6";
		if (pin == D7)
			return "D7";
		if (pin == D8)
			return "D8";
		if (pin == D9)
			return "D9";
		if (pin == D10)
			return "D10";
#endif

#ifdef ESP32
		if (pin == 5)
			return "D5";
		if (pin == 18)
			return "D18";
		if (pin == 19)
			return "D19";
		if (pin == 21)
			return "D21";
		if (pin == 22)
			return "D22";
		if (pin == 23)
			return "D23";
#endif // ESP32
		return "";
	}

	static uint8_t pinFromStr(String str)
	{

		str.replace("\r\n", ""); // importante!!

		//logger.print(tag, "\n\t >> pinFromStr strPin=");
		//logger.print(tag, str);
#ifdef ESP8266
		if (str.equalsIgnoreCase("D0"))
			return D0;
		if (str.equals("D1"))
			return D1;
		if (str.equalsIgnoreCase("D2"))
			return D2;
		if (str.equals("D3"))
			return D3;
		if (str.equals("D4"))
			return D4;
		if (str.equals("D5"))
			return D5;
		if (str.equals("D6"))
			return D6;
		if (str.equals("D7"))
			return D7;
		if (str.equals("D8"))
			return D8;
#ifdef D9
		if (str.equals("D9"))
			return D9;
#endif
#ifdef D10
		if (str.equals("D10"))
			return D10;
#endif
#endif

#ifdef ESP32
		if (str.equals("D5"))
			return 5;
		if (str.equals("D18"))
			return 18;
		if (str.equals("D19"))
			return 19;
		if (str.equals("D21"))
			return 21;
		if (str.equals("D22"))
			return 22;
		if (str.equals("D23"))
			return 23;

#endif
		logger.print(tag, "\n\t PIN NOT FOUND");
		return 0;
	}


	int getLocalPort()
	{
		return localPort;
	}

	void setLocalPort(int port)
	{
		logger.print(tag, "\n\t >>setLocalPort");
		localPort = port;
		logger.print(tag, "\n\t >> localPort=" + String(localPort));
	}

	String getName()
	{
		return String(name);
	}

	void setName(String _name)
	{
		logger.print(tag, "\n\t >>setName");
		name = _name;
		logger.print(tag, "\n\t name=");
		logger.print(tag, name);
	}

	String getServerName()
	{
		return String(serverName);
	}

	void setServerName(String name)
	{
		logger.print(tag, "\n\t >>setServerName");
		serverName = name;
		logger.print(tag, "\n\t serverName=");
		logger.print(tag, serverName);
	}

	String getUser()
	{
		return String(user);
	}

	void setUser(String _user)
	{
		logger.print(tag, "\n\t >>setUser");
		user = _user;
		logger.print(tag, "\n\t setUser=");
		logger.print(tag, user);
	}

	String getPassword()
	{
		return String(password);
	}

	void setPassword(String _password)
	{
		logger.print(tag, "\n\t >>setPassword");
		password = _password;
		logger.print(tag, "\n\t setPassword=");
		logger.print(tag, password);
	}

	String getUser2()
	{
		return String(user2);
	}

	void setUser2(String _user)
	{
		logger.print(tag, "\n\t >>setUser2");
		user2 = _user;
		logger.print(tag, "\n\t setUse2r=");
		logger.print(tag, user2);
	}

	String getPassword2()
	{
		return String(password2);
	}

	void setPassword2(String _password)
	{
		logger.print(tag, "\n\t >>setPassword2");
		password2 = _password;
		logger.print(tag, "\n\t setPassword2=");
		logger.print(tag, password2);
	}



	/*String getShieldName()
	{
		return String(shieldName);
	}

	void setShieldName(String name)
	{
		logger.print(tag, "\n\t >>setShieldName");
		shieldName = name;
		logger.print(tag, "\n\t<< shieldName=");
		logger.print(tag, shieldName);
	}*/

	String getPowerStatus()
	{
		return powerStatus;
	}

	void setPowerStatus(String status)
	{
		logger.print(tag, "\n\t>>setpowerStatus");
		powerStatus = status;
		logger.print(tag, "\n\t<< powerSatus=" + String(powerStatus));
	}

	bool getMQTTmode()
	{
		return mqttMode;
	}

	void setMQTTMode(bool enabled)
	{
		logger.print(tag, "\n\t>> setMQTTMode");
		mqttMode = enabled;
		logger.print(tag, "\n\t<< setMQTTMode=" + String(mqttMode));
	}

	bool getConfigMode()
	{
		return configMode;
	}

	void setConfigMode(bool enabled)
	{
		logger.print(tag, "\n\t>> setConfigMode");
		configMode = enabled;
		logger.print(tag, "\n\t<< setConfigMode=" + String(configMode));
	}

	bool getResetSettings()
	{
		return resetSettings;
	}

	void setResetSettings(bool enabled)
	{
		logger.print(tag, "\n\t>> setResetSettings");
		resetSettings = enabled;
		logger.print(tag, "\n\t<< setResetSettings=" + String(resetSettings));
	}

	String getMQTTServer()
	{
		return /*"192.168.1.21";////*/  mqttServer /*serverName*/;
	}

	void setMQTTServer(String server)
	{
		logger.print(tag, "\n\t>> setMQTTServer");
		mqttServer = server;
		logger.print(tag, "\n\t<< setMQTTServer=" + String(mqttServer));
	}

	String getMQTTUser()
	{
		return mqttUser;
	}

	void setMQTTUser(String user)
	{
		logger.print(tag, "\n\t>> setMQTTUser");
		mqttUser = user;
		logger.print(tag, "\n\t<< setMQTTUser=" + String(mqttUser));
	}

	String getMQTTPassword()
	{
		return mqttPassword;
	}

	void setMQTTPassword(String password)
	{
		logger.print(tag, "\n\t>> setMQTTPassword");
		mqttPassword = password;
		logger.print(tag, "\n\t<< setMQTTPassword=" + String(mqttPassword));
	}


	String getMQTTTopic()
	{
		return mqttTopic;
	}

	void setMQTTTopic(String topic)
	{
		logger.print(tag, "\n\t>> setMQTTTopic");
		mqttTopic = topic;
		logger.print(tag, "\n\t<< setMQTTTopic=" + String(mqttTopic));
	}

	int getMQTTPort()
	{
		return mqttPort;
	}

	void setMQTTPort(int port)
	{
		logger.print(tag, "\n\t>> setMQTTPort");
		mqttPort = port;
		logger.print(tag, "\n\t<< setMQTTPort=" + String(mqttPort));
	}

	String getRebootReason()
	{
		return rebootreason;
	}

	void setRebootReason(String reason)
	{
		//logger.print(tag, "\n\t>> setMQTTPort");
		rebootreason = reason;
		//logger.print(tag, "\n\t<< setMQTTPort=" + String(mqttPort));
	}
};

#endif