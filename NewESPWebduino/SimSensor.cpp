#include "SimSensor.h"
#include "Shield.h"

Logger SimSensor::logger;
String SimSensor::tag = "SimSensor";

// Your GPRS credentials (leave empty, if not needed)
const char apn[] = "TM";//"iliad"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = "";//"Iliad"; // GPRS User
const char gprsPass[] = ""; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char simPIN[] = "";//"1234"; 

// Server details
// The server variable can be just a domain name or it can have a subdomain. It depends on the service you are using
const char server[] = "giacomocasa.duckdns.org"; // domain name: example.com, maker.ifttt.com, etc
const char resource[] = "/webduino/time";         // resource path, for example: /post-data.php
const int  port = 8080;                             // server port number

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-data.php also needs to have the same key 
String apiKeyValue = "tPmAT5Ab3j7F9";

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
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb


#include <Wire.h>
#include <TinyGsmClient.h>

TinyGsm modem(SerialAT);

// I2C for SIM800 (to keep it running when powered from battery)
TwoWire I2CPower = TwoWire(0);

// TinyGSM Client for Internet connection
TinyGsmClient tinyGsmClient(modem);

#define uS_TO_S_FACTOR 1000000     /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3600        /* Time ESP32 will go to sleep (in seconds) 3600 seconds = 1 hour */

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

bool setPowerBoostKeepOn(int en) {
	I2CPower.beginTransmission(IP5306_ADDR);
	I2CPower.write(IP5306_REG_SYS_CTL0);
	if (en) {
		I2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
	}
	else {
		I2CPower.write(0x35); // 0x37 is default reg value
	}
	return I2CPower.endTransmission() == 0;
}

SimSensor::SimSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>SimSensor::SimSensor"));

	type = "simsensor";
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<SimSensor::SimSensor\n"));
}

SimSensor::~SimSensor()
{
}

void SimSensor::init()
{
	logger.print(tag, "\n\t >>init SimSensor pin=" + String(pin));
	//pinMode(pin, INPUT);

	// Start I2C communication
	I2CPower.begin(I2C_SDA, I2C_SCL, 400000);
	//I2CBME.begin(I2C_SDA_2, I2C_SCL_2, 400000);

	// Keep power when running from battery
	bool isOk = setPowerBoostKeepOn(1);
	SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

	// Set modem reset, enable, power pins
	pinMode(MODEM_PWKEY, OUTPUT);
	pinMode(MODEM_RST, OUTPUT);
	pinMode(MODEM_POWER_ON, OUTPUT);
	digitalWrite(MODEM_PWKEY, LOW);
	digitalWrite(MODEM_RST, HIGH);
	digitalWrite(MODEM_POWER_ON, HIGH);

	// Set GSM module baud rate and UART pins
	SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
	delay(3000);

	// Restart SIM800 module, it takes quite some time
	// To skip it, call init() instead of restart()
	SerialMon.println("Initializing modem...");
	modem.restart();
	// use modem.init() if you don't need the complete restart

	// Unlock your SIM card with a PIN if needed
	if (strlen(simPIN) && modem.getSimStatus() != 3) {
		modem.simUnlock(simPIN);
	}

	// You might need to change the BME280 I2C address, in our case it's 0x76
	/*if (!bme.begin(0x76, &I2CBME)) {
	  Serial.println("Could not find a valid BME280 sensor, check wiring!");
	  while (1);
	}*/

	// Configure the wake up source as timer wake up  
	esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

	logger.print(tag, F("\n\t <<init SimSensor"));
}

void SimSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	//json["mode"] = mode;
	//json.printTo(Serial);
}

void SimSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		logger.print(tag, "\SimSensor::checkStatusChange\n");
		lastCheckStatus = currMillis;

		/*if (mode.equals(MODE_NORMAL)) {
			if (digitalRead(pin) == LOW) {
				logger.print(tag, "\nOPENn pin=" + String(pin));
				setStatus(STATUS_DOOROPEN);
				ret = Sensor::checkStatusChange();
			}
			else {
				logger.print(tag, "\nCLOSED pin=" + String(pin));
				setStatus(STATUS_DOORCLOSED);
				ret = Sensor::checkStatusChange();
			}
		}
		else if (mode.equals(MODE_TEST)) {
			ret = Sensor::checkStatusChange();
		}*/

		SerialMon.print("Connecting to APN: ");
		SerialMon.print(apn);
		if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
			SerialMon.println(" fail");
		}
		else {
			SerialMon.println(" OK");

			SerialMon.print("Connecting to ");
			SerialMon.print(server);
			if (!tinyGsmClient.connect(server, port)) {
				SerialMon.println(" fail");
			}
			else {
				SerialMon.println(" OK");

				// Making an HTTP POST request
				SerialMon.println("Performing HTTP POST request...");
				// Prepare your HTTP POST request data (Temperature in Celsius degrees)
				String httpRequestData = "api_key=" + apiKeyValue + "&value1=" + "String(bme.readTemperature())"
					+ "&value2=" + "String(bme.readHumidity())" + "&value3=" + "String(bme.readPressure()/100.0F)" + "";
				// Prepare your HTTP POST request data (Temperature in Fahrenheit degrees)
				//String httpRequestData = "api_key=" + apiKeyValue + "&value1=" + String(1.8 * bme.readTemperature() + 32)
				//                       + "&value2=" + String(bme.readHumidity()) + "&value3=" + String(bme.readPressure()/100.0F) + "";

				// You can comment the httpRequestData variable above
				// then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
				//String httpRequestData = "api_key=tPmAT5Ab3j7F9&value1=24.75&value2=49.54&value3=1005.14";

				tinyGsmClient.print(String("POST ") + resource + " HTTP/1.1\r\n");
				tinyGsmClient.print(String("Host: ") + server + "\r\n");
				tinyGsmClient.println("Connection: close");
				tinyGsmClient.println("Content-Type: application/x-www-form-urlencoded");
				tinyGsmClient.print("Content-Length: ");
				tinyGsmClient.println(httpRequestData.length());
				tinyGsmClient.println();
				tinyGsmClient.println(httpRequestData);

				unsigned long timeout = millis();
				while (tinyGsmClient.connected() && millis() - timeout < 10000L) {
					// Print available data (HTTP response from server)
					while (tinyGsmClient.available()) {
						char c = tinyGsmClient.read();
						SerialMon.print(c);
						timeout = millis();
					}
				}
				SerialMon.println();

				// Close tinyGsmClient and disconnect
				tinyGsmClient.stop();
				SerialMon.println(F("Server disconnected"));
				modem.gprsDisconnect();
				SerialMon.println(F("GPRS disconnected"));
			}
		}
		// Put ESP32 into deep sleep mode (with timer wake up)
		//esp_deep_sleep_start();



	}
	Sensor::checkStatusChange();
}

bool SimSensor::sendCommand(String command, String payload)
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
