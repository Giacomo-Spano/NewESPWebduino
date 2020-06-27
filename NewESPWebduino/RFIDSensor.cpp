#include "RFIDSensor.h"
#include "Shield.h"

#include <MFRC522.h> //library responsible for communicating with the module RFID-RC522
#include <SPI.h> //library responsible for communicating of SPI bus
#define SS_PIN    21
#define RST_PIN   22
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define greenPin     12
#define redPin       32
//used in authentication
MFRC522::MIFARE_Key key;
//authentication return status code
MFRC522::StatusCode status;
// Defined pins to module RC522
MFRC522 mfrc522(SS_PIN, RST_PIN);






Logger RFIDSensor::logger;
String RFIDSensor::tag = "RFIDSensor";

String  RFIDSensor::STATUS_DOOROPEN = "rfidopen";
String  RFIDSensor::STATUS_DOORCLOSED = "rfidclosed";

String  RFIDSensor::MODE_NORMAL = "normal";
String  RFIDSensor::MODE_TEST = "test";
String  RFIDSensor::MODE_TESTOPEN = "testopen";


RFIDSensor::RFIDSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>RFIDSensor::RFIDSensor"));

	type = "rfidsensor";

	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<RFIDSensor::RFIDSensor\n"));
}

void RFIDSensor::init()
{
	logger.print(tag, "\n\t >>init RFIDSensor pin=" + String(pin));
	Sensor::init();

	//pinMode(pin, INPUT);
	mode = MODE_NORMAL;




	SPI.begin(); // Init SPI bus
	//pinMode(greenPin, OUTPUT);
	//pinMode(redPin, OUTPUT);

	// Init MFRC522
	mfrc522.PCD_Init();


	mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
	Serial.println();






	logger.print(tag, F("\n\t <<init RFIDSensor"));
}

void RFIDSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["mode"] = mode;
}

void RFIDSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		lastCheckStatus = currMillis;

		// Aguarda a aproximacao do cartao
		//waiting the card approach
		if (!mfrc522.PICC_IsNewCardPresent()) {
			return;
		}

		// Select one of the cards
		if (!mfrc522.PICC_ReadCardSerial()) {
			return;
		}

		// Dump debug info about the card; PICC_HaltA() is automatically called
		//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

		//Show UID on serial monitor
		Serial.print("UID tag :");
		String content = "";
		byte letter;
		for (byte i = 0; i < mfrc522.uid.size; i++)
		{
			Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
			Serial.print(mfrc522.uid.uidByte[i], HEX);
			content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
			content.concat(String(mfrc522.uid.uidByte[i], HEX));
		}
		Serial.println();
		Serial.print("Message : ");
		content.toUpperCase();

		if (type0CallBackPointer != nullptr) {
			type0CallBackPointer->callBackEvent(sensorid, "cardread", content.substring(1));
		}
	}
	Sensor::checkStatusChange();
}

bool RFIDSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>DoorSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("testmode")) {
		if (payload.equals("on")) {
			mode = MODE_TEST;
		}
		else if (payload.equals("off")) {
			mode = MODE_NORMAL;
		}
	}
	else if (command.equals("settestdoorstatus")) {
		if (mode.equals(MODE_TEST)) {
			if (payload.equals("open")) {
				setStatus(STATUS_DOOROPEN);
			}
			else if (payload.equals("closed")) {
				setStatus(STATUS_DOORCLOSED);
			}
		}
	}
	else if (command.equals("test")) {
		mode = MODE_TESTOPEN;
		lastTestModeTime = millis();
		if (payload.equals("open")) {
			setStatus(STATUS_DOOROPEN);
		}
		else if (payload.equals("closed")) {
			setStatus(STATUS_DOORCLOSED);
		}
	}
	logger.print(tag, F("\n\t <<RFIDSensor::sendCommand"));
	return false;
}
