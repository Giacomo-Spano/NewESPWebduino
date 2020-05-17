/*
 Name:		NewESPWebduino.ino
 Created:	5/15/2020 10:50:11 PM
 Author:	Giacomo
*/

#if defined(ESP8266)
#include <ESP8266WiFi.h>          
#else
#include <WiFi.h>          
#endif

#include <Arduino.h>

#include "Logger.h"
#include "MQTTClientClass.h"
#include "Shield.h"
#include "KeyLockSensor.h"


#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
//#include <SPIFFSEditor.h>
#ifndef ESP8266
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include "SPIFFS.h"
#endif

#ifdef ESP8266
#include <ESP8266httpUpdate.h>
#else
#include <ESP32httpUpdate.h>
#endif

Shield shield;
KeyLockSensor* keylockSensor;

extern bool mqtt_publish(String topic, String message);

const char* ssid = "FASTWEB-C16E33";
const char* password = "4GE4MEHHFG";
const char* http_username = "admin";
const char* http_password = "admin";


AsyncWebServer asyncServer(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)
//flag to use from web update to reboot the ESP
bool shouldReboot = false;


///////////
const int command_open = 1;
const int command_close = 2;
volatile int keylockcommand;

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

const char* PARAM_NAME = "name";
const char* PARAM_MQTT_SERVER = "mqtt_server";
const char* PARAM_MQTT_PORT = "mqtt_port";
const char* PARAM_MQTT_USER = "mqtt_user";
const char* PARAM_MQTT_PASSWORD = "mqtt_password";
const char* PARAM_MQTT_TOPIC = "mqtt_topic";
const char* PARAM_SERVER_NAME = "server_name";
const char* PARAM_SERVER_PORT = "server_port";
const char* PARAM_USER = "user";
const char* PARAM_PASSWORD = "password";
const char* PARAM_USER2 = "user2";
const char* PARAM_PASSWORD2 = "password2";
const char* PARAM_SENSORID = "sensorid";
const char* PARAM_TYPE = "type";
const char* PARAM_PIN = "pin";
const char* PARAM_ACTION = "action";



// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
  .button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;
	text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}      
  .button2 {background-color: #77878A;}</style></head>
  <body>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input2: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input3: <input type="text" name="input3">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";







Logger logger;
String tag = "Webduino";
MQTTClientClass mqttclient;

WiFiClient client;

//int Step = 0; //GPIO0---D3 of Nodemcu--Step of stepper motor driver
//int Dir = 2; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver
//int Enb = 14; //GPI14---D5 of Nodemcu--Enable driver

WiFiServer server(80);
// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void mqtt_messageReceived(char* topic, byte* payload, unsigned int length) {

	logger.println(tag, F(">>messageReceived"));
	logger.print(tag, F("\n\t topic="));
	logger.print(tag, String(topic));
	logger.print(tag, F("\n\t payload="));
	//logger.print(tag, payload);

	//keylockSensor->openLock();

#ifdef ESP8266
	ESP.wdtFeed();
#endif // ESP8266

	String message = "";
	for (int i = 0; i < length; i++) {
		message += char(payload[i]);
	}

	shield.parseMessageReceived(String(topic), message);
	logger.println(tag, F("<<messageReceived"));
}

void onRequest(AsyncWebServerRequest* request) {
	//Handle Unknown Request
	logger.println(tag, "\n\n\t>>>>>>>>>>>>>>>>>>>>>>onRequest\n\n");
	request->send(404);
}

void onBody(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
	//Handle body
	logger.println(tag, "\n\n\t>>>>>>>>>>>>>>>>>>>>>>onBody\n\n");
}

void onUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
	//Handle upload
	logger.println(tag, "\n\n\t>>>>>>>>>>>>>>>>>>>>>>onUpload\n\n");
	if (!index) {
		Serial.printf("UploadStart: %s\n", filename.c_str());
	}
	for (size_t i = 0; i < len; i++) {
		Serial.write(data[i]);
	}


	File file = SPIFFS.open("/" + filename, "w");
	if (!file) {
		logger.print(tag, "error to create file");
		return;
	}
	file.write(data,len);
	file.close();

	if (final) {
		Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
	}
}

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
	//Handle WebSocket event
	logger.println(tag, "\n\n\t>>>>>>>>>>>>>>>>>>>>>>onEvent\n\n");
}


void notFound(AsyncWebServerRequest* request) {
	request->send(404, "text/plain", "Not found");
}


bool initMQTTServer() {
	logger.println(tag, F("\n\n >>initMQTTServer"));
	mqttclient.init(&client);
	mqttclient.setServer(shield.getMQTTServer(), shield.getMQTTPort());
	mqttclient.setCallback(mqtt_messageReceived);
	bool res = reconnect();
	logger.println(tag, String(F("\n <<initMQTTServer =")) + Logger::boolToString(res));
}

void setup() {
	Serial.begin(115200);
	delay(10);

	Serial.print("\n\n\ **************** RESTART *************************\n\n");
	Serial.print("sw ver = ");
	Serial.println(shield.getSWVersion());
	
	
	// Initialize SPIFFS
	SPIFFS.format();
	Serial.println(F("Inizializing FS..."));
	if (SPIFFS.begin()) {
		Serial.println(F("done."));
		Serial.println("\n\n----Listing files ----");
		listAllFiles();
	}
	else {
		Serial.println(F("fail."));
	}

	// Initialize Shield
	shield.readRebootReason();
	shield.readConfig();
	shield.init();
	

	// Connect to WiFi network
	Serial.print("\n\nConnecting to ");
	Serial.println(ssid);
	Serial.print("\n");

	WiFi.mode(WIFI_STA);//
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWiFi connected");
	Serial.print("IP Address: x");
	Serial.println(WiFi.localIP());
	Serial.print("\n\n");


	checkForSWUpdate();



	// attach AsyncWebSocket
	ws.onEvent(onEvent);
	//asyncServer.addHandler(&ws);

	// attach AsyncEventSource
	//asyncServer.addHandler(&events);


	//ws.onEvent(onWsEvent);
	asyncServer.addHandler(&ws);

	events.onConnect([](AsyncEventSourceClient* client) {
		client->send("hello!", NULL, millis(), 1000);
		});
	asyncServer.addHandler(&events);


	asyncServer.on("/heap", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
		});

	asyncServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");


	// respond to GET requests on URL /heap
	asyncServer.on("/heap", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
		});

	// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
	asyncServer.on("/getfiles", HTTP_GET, [](AsyncWebServerRequest* request) {

		Serial.print(" /getfiles request received\n");
		String sensorID;
		// GET input1 value on <ESP_IP>/get?input1=<inputMessage>
		request->send(200, "application/json", getFiles());
		return;

		//Serial.println("errore");
		//request->send(200, "text/html", "errore <br><a href=\"/\">Return to Home Page</a>");
		});

	asyncServer.serveStatic("/upload.html", SPIFFS, "/upload.html");

	asyncServer.on("/delete", HTTP_GET, [](AsyncWebServerRequest* request) {
		if (request->hasParam("filename")) {
			String filename = request->getParam("filename")->value();
			deleteFile(filename);
		}		
		request->send(200, "text/plain", "File deleted!");
		});

	// HTTP basic authentication
	asyncServer.on("/login", HTTP_GET, [](AsyncWebServerRequest* request) {
		if (!request->authenticate(http_username, http_password))
			return request->requestAuthentication();
		request->send(200, "text/plain", "Login Success!");
		});

	// Simple Firmware Update Form
	/*asyncServer.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
		});*/


	// Simple Firmware Update Form
	asyncServer.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
		checkForSWUpdate();
		request->send(200, "text/html", "HTTP fw updated<br><a href=\"/\">Return to Home Page</a>");
		});
	


	// respond to GET requests on URL /home
	asyncServer.on("/home", HTTP_GET, [](AsyncWebServerRequest* request) {
		logger.println(tag, "\n\n /home");
		request->send_P(200, "text/html", index_html);
		});

	// respond to GET requests on URL /
	asyncServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
		logger.println(tag, "\n\n /index.html");
		request->send(SPIFFS, "/index.html", String(), false, processor);
		});

	// Route for root / web page
	asyncServer.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {
		logger.println(tag, "\n\n /config.html");
		request->send(SPIFFS, "/config.html", String(), false, processor);
		});


	// Route to load style.css file
	asyncServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/style.css", "text/css");
		});

	// Route to load jquery-3.5.0.min.js file
	asyncServer.on("/jquery-3.5.0.min.js", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/jquery-3.5.0.min.js", "text/javascript");
		});

	// Route to set GPIO to HIGH
	asyncServer.on("/sensor.html", HTTP_GET, [](AsyncWebServerRequest* request) {
		//digitalWrite(ledPin, HIGH);
		//keylockSensor->openLock();
		keylockcommand = command_open;
		request->send(SPIFFS, "/sensor.html", String(), false, processor);
		});

	// Route to set GPIO to HIGH
	asyncServer.on("/temperature.html", HTTP_GET, [](AsyncWebServerRequest* request) {
		//digitalWrite(ledPin, HIGH);
		//keylockSensor->openLock();
		keylockcommand = command_open;
		request->send(SPIFFS, "/temperature.html", String(), false, processor);
		});

	// Route to set GPIO to HIGH
	asyncServer.on("/onewire.html", HTTP_GET, [](AsyncWebServerRequest* request) {
		//digitalWrite(ledPin, HIGH);
		//keylockSensor->openLock();
		keylockcommand = command_open;
		request->send(SPIFFS, "/sensor.html", String(), false, processor);
		});

	// Route to set GPIO to HIGH
	asyncServer.on("/keylock.html", HTTP_GET, [](AsyncWebServerRequest* request) {
		//digitalWrite(ledPin, HIGH);
		//keylockSensor->openLock();
		keylockcommand = command_open;
		request->send(SPIFFS, "/keylock.html", String(), false, processor);
		});

	asyncServer.on("/sensors", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(SPIFFS, "/sensors.html", String(), false, processor);
		});


	asyncServer.on("/open", HTTP_GET, [](AsyncWebServerRequest* request) {
		//digitalWrite(ledPin, HIGH);
		//keylockSensor->openLock();
		keylockcommand = command_open;
		request->send(SPIFFS, "/index.html", String(), false, processor);
		});

	// Route to set GPIO to LOW
	asyncServer.on("/close", HTTP_GET, [](AsyncWebServerRequest* request) {
		//digitalWrite(ledPin, LOW);
		keylockcommand = command_close;
		request->send(SPIFFS, "/index.html", String(), false, processor);
		});


	// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
	asyncServer.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
		//String inputMessage;
		//String inputParam;

		Serial.print("\n /get request received\n");
		if (request->hasParam(PARAM_NAME)) {
			String str = request->getParam(PARAM_NAME)->value();
			shield.setName(str);
		}
		if (request->hasParam(PARAM_MQTT_SERVER)) {
			String str = request->getParam(PARAM_MQTT_SERVER)->value();
			shield.setMQTTServer(str);
		}
		if (request->hasParam(PARAM_MQTT_PORT)) {
			String str = request->getParam(PARAM_MQTT_PORT)->value();
			shield.setMQTTPort(str.toInt());
		}
		if (request->hasParam(PARAM_MQTT_USER)) {
			String str = request->getParam(PARAM_MQTT_USER)->value();
			shield.setMQTTUser(str);
		}
		if (request->hasParam(PARAM_MQTT_PASSWORD)) {
			String str = request->getParam(PARAM_MQTT_PASSWORD)->value();
			shield.setMQTTPassword(str);
		}
		if (request->hasParam(PARAM_MQTT_TOPIC)) {
			String str = request->getParam(PARAM_MQTT_TOPIC)->value();
			shield.setMQTTTopic(str);
		}
		if (request->hasParam(PARAM_SERVER_NAME)) {
			String str = request->getParam(PARAM_SERVER_NAME)->value();
			shield.setServerName(str);
		}
		if (request->hasParam(PARAM_SERVER_PORT)) {
			String str = request->getParam(PARAM_SERVER_PORT)->value();
			shield.setServerPort(str.toInt());
		}
		if (request->hasParam(PARAM_USER)) {
			String str = request->getParam(PARAM_USER)->value();
			shield.setUser(str);
		}
		if (request->hasParam(PARAM_PASSWORD)) {
			String str = request->getParam(PARAM_PASSWORD)->value();
			shield.setPassword(str);
		}
		if (request->hasParam(PARAM_USER2)) {
			String str = request->getParam(PARAM_USER2)->value();
			shield.setUser2(str);
		}
		if (request->hasParam(PARAM_PASSWORD2)) {
			String str = request->getParam(PARAM_PASSWORD2)->value();
			shield.setPassword2(str);
		}

		shield.writeConfig();
		request->send(200, "text/html", "HTTP GET request sent to your ESP on input field <br><a href=\"/\">Return to Home Page</a>");
		//+ inputParam + ") with value: " + inputMessage*/ +
		//"<br><a href=\"/\">Return to Home Page</a>");
		});



	// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
	asyncServer.on("/sensor", HTTP_GET, [](AsyncWebServerRequest* request) {

		Serial.print("\n /sensor request received\n");
		if (request->hasParam(PARAM_ACTION)) {
			String action = request->getParam(PARAM_ACTION)->value();
			if (action.equals("add")) {
				Serial.print("\n add sensor\n");
				request->send(SPIFFS, "/sensor.html", String(), false, processor);
				return;
			}
			else if (request->hasParam(PARAM_SENSORID)) {
				String sensorID = request->getParam(PARAM_SENSORID)->value();
				Serial.print(" sensorid=");
				Serial.print(sensorID);
				if (action.equals("delete")) {
					Serial.print("\n delete sensor\n");
					for (int i = 0; i < shield.sensors.size(); i++) {
						Sensor* sensor = (Sensor*)shield.sensors.get(i);
						if (sensor->sensorid == sensorID.toInt()) {
							shield.sensors.remove(i);
							//request->send(SPIFFS, "/sensors.html", String(), false, processor);
							//request->send(200, "text/html", "errore - sensor not found<br><a href=\"/\">Return to Home Page</a>");
							request->send(200, "text/html", "Sensor deleted <br><a href=\"/sensors\">Ok</a>");
							return;
						}
					}
				}
				else if (action.equals("edit")) {
					Serial.print("\n update sensor\n");
					for (int i = 0; i < shield.sensors.size(); i++) {
						Sensor* sensor = (Sensor*)shield.sensors.get(i);

						Serial.print(" sensorname=");
						Serial.print(sensor->sensorname);
						Serial.print(" id=");
						Serial.print(String(sensor->sensorid));

						if (sensor->sensorid == sensorID.toInt()) {
							request->send(SPIFFS, "/sensor.html", String(), false, processor);
							return;
						}
					}
				}
				else {
					Serial.print("\n action not found\n");
					request->send(200, "text/html", "errore - sensor not found<br><a href=\"/\">Return to Home Page</a>");
				}
				Serial.print("\n sensor not found\n");
				request->send(200, "text/html", "errore - sensor not found<br><a href=\"/\">Return to Home Page</a>");
			}
		}
		else {
			Serial.println("errore");
			request->send(200, "text/html", "error - no action param <br><a href=\"/\">Return to Home Page</a>");
		}
		});


	// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
	asyncServer.on("/getsensors", HTTP_GET, [](AsyncWebServerRequest* request) {

		Serial.print(" /getsensors request received\n");
		String sensorID;
		// GET input1 value on <ESP_IP>/get?input1=<inputMessage>
		request->send(200, "application/json", shield.getSensors());
		return;

		//Serial.println("errore");
		//request->send(200, "text/html", "errore <br><a href=\"/\">Return to Home Page</a>");
		});


	// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
	asyncServer.on("/getsensor", HTTP_GET, [](AsyncWebServerRequest* request) {

		Serial.print(" getsensor request received\n");
		String sensorID;
		// GET input1 value on <ESP_IP>/get?input1=<inputMessage>
		if (request->hasParam(PARAM_SENSORID)) {
			sensorID = request->getParam(PARAM_SENSORID)->value();
			for (int i = 0; i < shield.sensors.size(); i++) {
				Sensor* sensor = (Sensor*)shield.sensors.get(i);
				if (sensor->sensorid == sensorID.toInt()) {
					request->send(200, "application/json", sensor->getStrJson());
					return;
				}
			}
		}
		Serial.println("new sensor - sensor not found");
		request->send(200, "text/html", "errore <br><a href=\"/\">Return to Home Page</a>");
		});


	asyncServer.on("/setsensor", HTTP_POST, [](AsyncWebServerRequest* request) {
		//nothing and dont remove it
		}, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {

			Serial.print("\n\n setsensor request received\n");
			for (size_t i = 0; i < len; i++) {
				Serial.write(data[i]);
			}
			Serial.print("\n");

			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject((const char*)data);
			if (root.success()) {
				if (root.containsKey("sensorid")) {
					//Serial.println(root["sensorid"].asString());
					//root.printTo(Serial);
					shield.updateSensor(root);
				}
				Serial.print(" \nsend response\n");
				request->send(200, "text/html", "Sensor saved <br><a href=\"/sensors\">Ok</a>");
			}
			else {
				Serial.print("\n\n BAD JSON\n");
				request->send(404, "text/html", "BAD JSON <br><a href=\"/sensors\">Ok</a>");
				//request->send(404, "text/html", "");
			}
		});

	


	// attach filesystem root at URL /fs
	asyncServer.serveStatic("/fs", SPIFFS, "/");

	// Catch-All Handlers
	// Any request that can not find a Handler that canHandle it
	// ends in the callbacks below.
	asyncServer.onNotFound(onRequest);
	asyncServer.onFileUpload(onUpload);
	asyncServer.onRequestBody(onBody);

	//asyncServer.onNotFound(notFound);
	asyncServer.begin();




	// Initialize MQTT
	initMQTTServer();
}

bool reconnect() {
	logger.print(tag, F("\n\n\t >>reconnect"));
	//shield.setStatus(F("CONNECTING.."));

	// Loop until we're reconnected
	String clientId = "ESP8266Client" + WiFi.macAddress(); /*shield.getMACAddress();*/
	int i = 0;
	while (!mqttclient.connected() && i < 3) {
#ifdef ESP8266
		ESP.wdtFeed();
#endif // ESP8266
		logger.print(tag, F("\n\t Attempting MQTT connection..."));
		logger.print(tag, F("\n\t temptative "));
		logger.print(tag, String(i));
		// Attempt to connect			
		if (mqttclient.connect(clientId, shield.getMQTTUser(), shield.getMQTTPassword())) {
			logger.print(tag, F("\n\t connected"));
			// Once connected, publish an announcement...
			String topic = "ESPWebduino/" + shield.getName() + "/command/#";

			logger.print(tag, String("\n\t topic = ") + topic);
			mqttclient.subscribe(topic.c_str());

			//shield.setStatus(F("ONLINE"));
			logger.print(tag, F("\n\t <<reconnect\n\n"));
			return true;
		}
		else {
			logger.print(tag, F("\n\tfailed, rc="));
			logger.print(tag, mqttclient.state());
			logger.print(tag, F("\n\ttry again in 1 seconds"));
			// Wait 1 seconds before retrying
			delay(1000);
			i++;
		}
	}









	//shield.setStatus(F("OFFLINE"));
	logger.print(tag, F("\n\t<<reconnect FAILED\n\n"));
	return false;
}

void loop() {



	//Serial.println(encoderValue);



	///////////////////////////////////////////////
	shield.checkStatus();


	if (keylockcommand == command_open) {
		Serial.println("\n\n OPEN\n");
		//Serial.print("init= ");
		//Serial.println(encoderValue);
		shield.sendSensorCommand("keylocksensor", 1, "set", "OPEN");
		keylockcommand = 0;
		//Serial.print("\nnd= ");
		//Serial.println(encoderValue);
	}
	else if (keylockcommand == command_close) {
		Serial.println("\n\n CLOSE\n");
		//Serial.print("init= ");
		//Serial.println(encoderValue);
		shield.sendSensorCommand("keylocksensor", 1, "set", "CLOSE");
		keylockcommand = 0;
		//Serial.print("\nend= ");
		//Serial.println(encoderValue);
	}

	if (client.connected()) {
		//shield.checkTimeUpdateStatus();
		//shield.checkSettingResquestStatus();
		mqttclient.loop();
	}
	else {
		logger.println(tag, F("\n\n\tSERVER DISCONNECTED!!!\n"));
		reconnect();
		delay(5000);
	}



	if (shouldReboot) {
		Serial.println("Rebooting...");
		delay(100);
		ESP.restart();
	}
	static char temp[128];
	sprintf(temp, "Seconds since boot: %u", millis() / 1000);
	events.send(temp, "time"); //send event "time"


	//return;
	//////////////
	/*WiFiClient client = server.available();   // Listen for incoming clients

	if (client) {                             // If a new client connects,
		Serial.println("\n\nNew Client.");          // print a message out in the serial port
		String currentLine = "";                // make a String to hold incoming data from the client
		currentTime = millis();
		previousTime = currentTime;
		while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
			currentTime = millis();
			if (client.available()) {             // if there's bytes to read from the client,
				char c = client.read();             // read a byte, then
				Serial.write(c);                    // print it out the serial monitor
				header += c;
				if (c == '\n') {                    // if the byte is a newline character
				  // if the current line is blank, you got two newline characters in a row.
				  // that's the end of the client HTTP request, so send a response:
					if (currentLine.length() == 0) {
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the client knows what's coming, then a blank line:
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println("Connection: close");
						client.println();

						// turns the GPIOs on and off
						if (header.indexOf("GET /5/on") >= 0) {
							Serial.println("GPIO 5 on");
							//output5State = "on";
							//digitalWrite(output5, HIGH);
						}
						else if (header.indexOf("GET /5/off") >= 0) {
							Serial.println("GPIO 5 off");
							//output5State = "off";
							//digitalWrite(output5, LOW);
						}
						else if (header.indexOf("GET /4/on") >= 0) {
							Serial.println("GPIO 4 on");
							//output4State = "on";
							//digitalWrite(output4, HIGH);
						}
						else if (header.indexOf("GET /4/off") >= 0) {
							Serial.println("GPIO 4 off");
							//output4State = "off";
							//digitalWrite(output4, LOW);
						}

						// Display the HTML web page
						client.println("<!DOCTYPE html><html>");
						client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
						client.println("<link rel=\"icon\" href=\"data:,\">");
						// CSS to style the on/off buttons
						// Feel free to change the background-color and font-size attributes to fit your preferences
						client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
						client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
						client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
						client.println(".button2 {background-color: #77878A;}</style></head>");

						// Web Page Heading
						client.println("<body><h1>ESP8266 Web Server</h1>");

						// Display current state, and ON/OFF buttons for GPIO 5
						//client.println("<p>GPIO 5 - State " + output5State + "</p>");
						// If the output5State is off, it displays the ON button
							client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
						//}

						// Display current state, and ON/OFF buttons for GPIO 4
						client.println("<p>GPIO 4 - State </p>");
						// If the output4State is off, it displays the ON button
							client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
						//}
						client.println("</body></html>");

						// The HTTP response ends with another blank line
						client.println();
						// Break out of the while loop
						break;
					}
					else { // if you got a newline, then clear currentLine
						currentLine = "";
					}
				}
				else if (c != '\r') {  // if you got anything else but a carriage return character,
					currentLine += c;      // add it to the end of the currentLine
				}
			}
		}
		// Clear the header variable
		header = "";
		// Close the connection
		client.stop();
		Serial.println("Client disconnected.");
		Serial.println("");*/
		//}

		/////////////////////////
		// Check if a client has connected
		/*client = server.available();
		if (!client) {
			return;
		}

		// Wait until the client sends some data
		Serial.println("new client");
		while (!client.available()) {
			delay(1);
		}

		// Read the first line of the request
		String request = client.readStringUntil('\r');
		Serial.println(request);
		client.flush();

		// Match the request
		int i = 0;
		int value = LOW;

		if (request.indexOf("/Command=forward") != -1) { //Move 50 steps forward

			digitalWrite(Enb, LOW); // Enable stepper motor

			digitalWrite(Dir, LOW); //Rotate stepper motor in clock wise direction
			for (i = 1; i <= 200; i++) {
				digitalWrite(Step, HIGH);
				delay(3);
				digitalWrite(Step, LOW);
				delay(3);
			}
			value = HIGH;

			digitalWrite(Enb, HIGH); // Disable stepper motor
		}

		if (request.indexOf("/Command=backward") != -1) { //Move 50 steps backwards

			digitalWrite(Enb, LOW); // Enable stepper motor

			digitalWrite(Dir, HIGH ); //Rotate stepper motor in anti clock wise direction
			for (i = 1; i <= 200; i++) {
				digitalWrite(Step, HIGH);
				delay(3);
				digitalWrite(Step, LOW);
				delay(3);
			}
			value = LOW;

			digitalWrite(Enb, HIGH); // Disable stepper motor
		}

		// Return the response
		client.println("HTTP/1.1 200 OK");
		client.println("Content-Type: text/html");
		client.println(""); //  do not forget this one
		client.println("<!DOCTYPE HTML>");
		client.println("<html>");
		client.println("<h1 align=center>Stepper motor controlled over WiFi</h1><br><br>");
		client.print("Stepper motor moving= ");

		if (value == HIGH) {
			client.print("Forward");
		}
		else {
			client.print("Backward");
		}
		client.println("<br><br>");
		client.println("<a href=\"/Command=forward\"\"><button>Forward </button></a>");
		client.println("<a href=\"/Command=backward\"\"><button>Backward </button></a><br />");
		client.println("</html>");
		delay(1);
		Serial.println("Client disonnected");
		Serial.println("");
		*/


}

bool mqtt_publish(String topic, String message) {

	logger.print(tag, String(F("\n\t >>mqtt_publish \n\t topic:")) + topic);
	logger.print(tag, String(F("\n\t message:")) + message);

	bool res = false;
	if (!client.connected()) {
		logger.print(tag, F("\n\t OFFLINE - payload NOT sent!!!\n"));
	}
	else {
		res = mqttclient.publish(topic.c_str(), message.c_str());
		if (!res) {
			logger.print(tag, F("\n\t MQTT Message not sent!!!\n"));
			return false;
		}
		else {
			logger.print(tag, F("\n\t MQTT Message sent!!!\n"));
		}
	}
	logger.print(tag, F("\n\t << mqtt_publish"));
	return true;
}

// Replaces placeholder with LED state value
String processor(const String& var) {
	String ledState;
	Serial.println(var);
	if (var == "STATE") {
		ledState = "ON";
		/*if (digitalRead(ledPin)) {
			ledState = "ON";
		}
		else {
			ledState = "OFF";
		}*/
		Serial.print(ledState);
		return ledState;
	}
	else if (var == "NAME") {
		return shield.getName();
	}
	else if (var == "MQTT_SERVER") {
		return shield.getMQTTServer();
	}
	else if (var == "MQTT_PORT") {
		return String(shield.getMQTTPort());
	}
	else if (var == "MQTT_USER") {
		return shield.getMQTTUser();
	}
	else if (var == "MQTT_PASSWORD") {
		return String(shield.getMQTTPassword());
	}
	else if (var == "MQTT_TOPIC") {
		return String(shield.getMQTTTopic());
	}
	else if (var == "SERVER_NAME") {
		return shield.getServerName();
	}
	else if (var == "SERVER_PORT") {
		return String(shield.getServerPort());
	}
	else if (var == "USER") {
		return shield.getUser();
	}
	else if (var == "PASSWORD") {
		return shield.getPassword();
	}
	else if (var == "USER2") {
		return shield.getUser2();
	}
	else if (var == "PASSWORD2") {
		return shield.getPassword2();
	}



	else if (var == "TEMPERATURE") {
		return "20";
		//return getTemperature();
	}
	else if (var == "HUMIDITY") {
		return "20%";
		//return getHumidity();
	}
	else if (var == "PRESSURE") {
		return "20PA";
		//return getPressure();
	}
	else if (var == "KEYLOCK") {
		return "false";
		//return getPressure();
	}
	/*else if (var == "SENSORS") {

		String str = "";
		for (int i = 0; i < shield.sensors.size(); i++)
		{
			Sensor* sensor = (Sensor*)shield.sensors.get(i);

			if (!sensor->enabled) {
				continue;
			}
			str +=
				"<form action = '/sensor'>" \
				+ sensor->sensorname +
				"	<input type = 'text' name = '" + PARAM_SENSORID + "' value='" + sensor->sensorid + "'>" \
				"   <input type = 'submit' name='" + PARAM_SUBMITBUTTON + "' value = 'edit'>" \
				"   <input type = 'submit' name='" + PARAM_SUBMITBUTTON + "' value = 'delete'>" \
				"</form><br>";
		}*/
	return "";
	//return getPressure();
}


void listAllFiles() {

	logger.print(tag, F("\n\n listAllFiles"));
	logger.print(tag, F("\n\tname \t\tsize"));
	/*Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		//Serial.print(dir.fileName());
		logger.print(tag, F("\n\t"));
		logger.print(tag, dir.fileName());
		File f = dir.openFile("r");
		logger.print(tag, F("\t"));
		logger.print(tag, f.size());
		//Serial.println(f.size());

		logger.print(tag, F("\n"));

		if (dir.fileName().equals("/sensors.json")) {
			String data = f.readString();
			Serial.println(data);
		}
		f.close();
		logger.print(tag, F("\n"));

	}*/
	logger.print(tag, F("\n"));
}

#ifdef ESP8266
String getFiles() {

	logger.println(tag, F(">>Shield::getFiles\n"));

	StaticJsonBuffer<1000> jsonArrayBuffer;
	JsonArray& jarray = jsonArrayBuffer.createArray();

	StaticJsonBuffer<1000> jsonBuffer;

	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {

		File f = dir.openFile("r");
		Serial.print("FILE: ");
		Serial.println(dir.fileName());

		JsonObject& json = jsonBuffer.createObject();
		json["filename"] = dir.fileName();
		json["size"] = f.size();
		jarray.add(json);
		f.close();
	}
	jarray.printTo(Serial);
	String str;
	jarray.printTo(str);
	logger.println(tag, F("<<Shield::getFiles"));
	return str;
}
#endif

#ifdef ESP32
String getFiles() {

	logger.println(tag, F(">>Shield::getFiles\n"));

	//StaticJsonBuffer<1000> jsonArrayBuffer;
	//JsonArray& jarray = jsonArrayBuffer.createArray();
	//StaticJsonBuffer<1000> jsonBuffer;

	Serial.println("file opened");
	if (!SPIFFS.begin()) {
		logger.print(tag, F("\n\t error mounting file system"));
		return "";
	}



	File root = SPIFFS.open("/");
	File file = root.openNextFile();

	Serial.println("file opened");

	while (file) {
	
		Serial.print("FILE: ");
		Serial.println(file.name());
	
		//JsonObject& json = jsonBuffer.createObject();


		//json["filename"] = file.name();
		//json["size"] = file.size();
		

		//jarray.add(json);

		//f.close();
	}
	//root.close();

	//jarray.printTo(Serial);
	String str;
	//jarray.printTo(str);
	logger.println(tag, F("<<Shield::getSensors"));
	return str;
}
#endif

bool deleteFile(String filename) {

	logger.println(tag, F(">>deleteFile\n"));

	return SPIFFS.remove(filename);

	logger.println(tag, F("<<deleteFile"));
}

void checkForSWUpdate() {

#ifdef ESP8266

	logger.println(tag, F(">>checkForSWUpdate"));
	//delay(2000);

	String updatePath = "http://192.168.1.203:8080/webduino/ota";// + /*shield.getServerName() +*/  "//webduino/ota";
	logger.print(tag, "\n\t check for sw update " + updatePath);
	logger.print(tag, "\n\t current version " + shield.swVersion);
	t_httpUpdate_return ret = ESPhttpUpdate.update(updatePath, shield.swVersion);
	//t_httpUpdate_return ret = ESPhttpUpdate.update(updatePath);

	switch (ret) {
	case HTTP_UPDATE_FAILED:

		logger.print(tag, F("\n\t HTTP_UPDATE_FAILD Error "));
		logger.print(tag, String(ESPhttpUpdate.getLastError()));
		logger.print(tag, F(" "));
		logger.print(tag, ESPhttpUpdate.getLastErrorString().c_str());

		shield.setEvent(F("sw Update failed"));
		break;

	case HTTP_UPDATE_NO_UPDATES:
		logger.print(tag, F("\n\t HTTP_UPDATE_NO_UPDATES"));
		shield.setEvent(F("no sw update available"));
		break;

	case HTTP_UPDATE_OK:
		logger.print(tag, F("\n\t HTTP_UPDATE_OK"));
		shield.setEvent(F("sw updated"));
		break;
	}
	logger.println(tag, F("<<checkForSWUpdate"));

#endif
}