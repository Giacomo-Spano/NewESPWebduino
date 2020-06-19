#include "MQTTSimSensor.h"
#include "Shield.h"

Logger MQTTSimSensor::logger;
String MQTTSimSensor::tag = "MQTTSimSensor";

String MQTTSimSensor::STATUS_DOOROPEN = "dooropen";
String MQTTSimSensor::STATUS_DOORCLOSED = "doorclosed";

// Your GPRS credentials (leave empty, if not needed)
const char xxapn[] = "TM";//"iliad"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char xxgprsUser[] = "";//"Iliad"; // GPRS User
const char xxgprsPass[] = ""; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char xxsimPIN[] = "";//"1234"; 

// Server details
// The server variable can be just a domain name or it can have a subdomain. It depends on the service you are using
const char xxserver[] = "giacomocasa.duckdns.org"; // domain name: example.com, maker.ifttt.com, etc
const char xxresource[] = "/webduino/time";         // resource path, for example: /post-data.php
const int  xxport = 8080;                             // server port number

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-data.php also needs to have the same key 
//String apiKeyValue = "tPmAT5Ab3j7F9";

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22
// BME280 pins
//#define I2C_SDA_2            18
//#define I2C_SCL_2            19

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define xxSerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define xxSerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
//#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb


// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG xxSerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Add a reception delay - may be needed for a fast processor at a slow baud rate
// #define TINY_GSM_YIELD() { delay(2); }

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// MQTT details
const char* broker = "giacomocasa.duckdns.org";

const char* topicLed = "GsmClientTest/led";
const char* topicInit = "GsmClientTest/init";
const char* topicLedStatus = "GsmClientTest/ledStatus";

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm xxmodem(xxSerialAT);

// I2C for SIM800 (to keep it running when powered from battery)
TwoWire xxI2CPower = TwoWire(0);

#endif
TinyGsmClient xxclient(xxmodem);
PubSubClient xxmqtt(xxclient);

//#define LED_PIN 13
//int ledStatus = LOW;

uint32_t lastReconnectAttempt = 0;

void xxmqttCallback(char* topic, byte * payload, unsigned int len) {
	xxSerialMon.print("Message arrived [");
	xxSerialMon.print(topic);
	xxSerialMon.print("]: ");
	xxSerialMon.write(payload, len);
	xxSerialMon.println();

	// Only proceed if incoming message's topic matches
	if (String(topic) == topicLed) {
		//ledStatus = !ledStatus;
		//digitalWrite(LED_PIN, ledStatus);
		xxmqtt.publish(topicLedStatus, "value1");
	}
}

boolean xxmqttConnect() {
	xxSerialMon.print("Connecting to :\n");
	xxSerialMon.print(broker);

	// Connect to MQTT Broker
	boolean status = xxmqtt.connect("GsmClientTest","giacomo","giacomo");

	// Or, if you want to authenticate MQTT:
	//boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

	if (status == false) {
		xxSerialMon.println(" fail");
		return false;
	}
	xxSerialMon.println(" success");
	xxmqtt.publish(topicInit, "GsmClientTest started");
	xxmqtt.subscribe(topicLed);
	return xxmqtt.connected();
}

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00


bool xxsetPowerBoostKeepOn(int en) {
	xxI2CPower.beginTransmission(IP5306_ADDR);
	xxI2CPower.write(IP5306_REG_SYS_CTL0);
	if (en) {
		xxI2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
	}
	else {
		xxI2CPower.write(0x35); // 0x37 is default reg value
	}
	return xxI2CPower.endTransmission() == 0;
}










MQTTSimSensor::MQTTSimSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>MQTTSimSensor::SimSensor"));

	type = "mqttsimsensor";
	checkStatus_interval = 10000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<MQTTSimSensor::SimSensor\n"));
}

MQTTSimSensor::~MQTTSimSensor()
{
}

void MQTTSimSensor::init()
{
	logger.print(tag, "\n\t >>init MQTTSimSensor pin=" + String(pin));


	/*xxSerialMon.println("Wait...");
	// Set GSM module baud rate
	// TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
	xxSerialAT.begin(9600);
	delay(6000);*/


	// Start I2C communication
	xxI2CPower.begin(I2C_SDA, I2C_SCL, 400000);
	//I2CBME.begin(I2C_SDA_2, I2C_SCL_2, 400000);

	// Keep power when running from battery
	bool isOk = xxsetPowerBoostKeepOn(1);
	xxSerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

	// Set modem reset, enable, power pins
	pinMode(MODEM_PWKEY, OUTPUT);
	pinMode(MODEM_RST, OUTPUT);
	pinMode(MODEM_POWER_ON, OUTPUT);
	digitalWrite(MODEM_PWKEY, LOW);
	digitalWrite(MODEM_RST, HIGH);
	digitalWrite(MODEM_POWER_ON, HIGH);

	// Set GSM module baud rate and UART pins
	xxSerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
	delay(3000);






	// Restart takes quite some time
	// To skip it, call init() instead of restart()
	xxSerialMon.println("Initializing modem...");
	xxmodem.restart();
	// modem.init();

	String modemInfo = xxmodem.getModemInfo();
	xxSerialMon.print("Modem Info: ");
	xxSerialMon.println(modemInfo);

#if TINY_GSM_USE_GPRS
	// Unlock your SIM card with a PIN if needed
	if (GSM_PIN && xxmodem.getSimStatus() != 3) {
		xxmodem.simUnlock(GSM_PIN);
	}
#endif

	xxSerialMon.print("Waiting for network...");
	if (!xxmodem.waitForNetwork()) {
		xxSerialMon.println(" fail");
		delay(10000);
		return;
	}
	xxSerialMon.println(" success");

	if (xxmodem.isNetworkConnected()) {
		xxSerialMon.println("Network connected");
	}

#if TINY_GSM_USE_GPRS
	// GPRS connection parameters are usually set after network registration
	xxSerialMon.print(F("Connecting to "));
	xxSerialMon.print(xxapn);
	if (!xxmodem.gprsConnect(xxapn, xxgprsUser, xxgprsPass)) {
		xxSerialMon.println(" fail");
		delay(10000);
		return;
	}
	xxSerialMon.println(" success");

	if (xxmodem.isGprsConnected()) {
		xxSerialMon.println("GPRS connected");
	}
#endif

	// MQTT Broker setup
	xxmqtt.setServer(broker, 1883);
	xxmqtt.setCallback(xxmqttCallback);
	logger.print(tag, F("\n\t <<init SimSensor"));
}

void MQTTSimSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	//json["mode"] = mode;
	//json.printTo(Serial);
}

void MQTTSimSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		logger.print(tag, "\SimSensor::checkStatusChange\n");
		lastCheckStatus = currMillis;

		if (!xxmqtt.connected()) {
			xxSerialMon.println("=== MQTT NOT CONNECTED ===");
			// Reconnect every 10 seconds
			uint32_t t = millis();
			if (t - lastReconnectAttempt > 10000L) {
				lastReconnectAttempt = t;
				if (xxmqttConnect()) {
					lastReconnectAttempt = 0;
				}
			}
			delay(100);
			return;
		}
		xxmqtt.loop();	
	}
	Sensor:checkStatusChange();
}

bool MQTTSimSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>SimSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("testmode")) {
		if (payload.equals("on")) {
			//mode = MODE_TEST;
		}
		else if (payload.equals("off")) {
			//mode = MODE_NORMAL;
		}
	}
	else if (command.equals("settestdoorstatus")) {
		/*if (mode.equals(MODE_TEST)) {
			if (payload.equals("open")) {
				setStatus(STATUS_DOOROPEN);
			}
			else if (payload.equals("closed")) {
				setStatus(STATUS_DOORCLOSED);
			}
		}*/
	}
	logger.print(tag, F("\n\t <<SimSensor::sendCommand"));
	return false;
}
