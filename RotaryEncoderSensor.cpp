#include "RotaryEncoderSensor.h"
//#include "Util.h"
//#include "ESP8266Webduino.h"
//#include "Shield.h"


Logger RotaryEncoderSensor::logger;
String RotaryEncoderSensor::tag = "RotaryEncoderSensor";

extern bool mqtt_publish(String topic, String message);






void RotaryEncoderSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	//json["card"] = lastcard;
}

RotaryEncoderSensor::RotaryEncoderSensor(int id, uint8_t pin, bool enabled, String address, String name, uint8_t stepPin, uint8_t directionPin, uint8_t enablePin) : Sensor(id, pin, enabled, address, name)
{
	type = "rotaryencodersensor";

	checkStatus_interval = 1000;
	lastCheckStatus = 0;
}

RotaryEncoderSensor::~RotaryEncoderSensor()
{
}

void RotaryEncoderSensor::init()
{
	logger.print(tag, F("\n\t >>init RotaryEncoderSensor"));





	/*pinMode(encoder0PinA, INPUT_PULLUP);
	pinMode(encoder0PinB, INPUT_PULLUP);
	pinMode(encoder0Btn, INPUT_PULLUP);
	attachInterrupt(0, doEncoder, CHANGE);
	encoderPos = 0;*/



	logger.print(tag, F("\n\t <<init RotaryEncoderSensor"));
}

void RotaryEncoderSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	//logger.println(tag, "\n\t currMillis="+String(currMillis) + "timeDiff=" + String(timeDiff));
	if (timeDiff > checkStatus_interval) {
		logger.print(tag, F("\n\n"));

		lastCheckStatus = currMillis;

		bool posChanged = loop();
		if (posChanged)
			logger.println(tag, F("temperatura cambiata"));
		else
			logger.println(tag, F("temperatura NON cambiata"));
		logger.print(tag, F("\n\n"));
		logger.println(tag, F("<<checkStatusChange()::checkStatusChange"));

		sendStatusUpdate();

	}
}


void RotaryEncoderSensor::sendStatusUpdate()
{
	logger.print(tag, F("\n\t\t sendStatusUpdate"));
	String topic = "ESPWebduino/myboard1/" + type + "/" + String(sensorid) + "/pos";
	if (mqtt_publish(topic, String(position / 1200)))
		oldStatus = status;
}

bool RotaryEncoderSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>RotaryEncoderSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("set")) {
		if (payload.equals("OPEN")) {
			return openLock();
		}
		else if (payload.equals("CLOSE")) {
			return closeLock();
		}
	}
	else if (command.equals("pos")) {
		int position = payload.toInt();
		return setPosition(100);
	}
	logger.print(tag, F("\n\t <<RotaryEncoderSensor::sendCommand"));
	return false;
}

bool RotaryEncoderSensor::setPosition(int pos) {

	/*double delta = pos * 800.0 / 6 - position;

	digitalWrite(enablePin, LOW); // Enable stepper motor
	if (delta < 0) {
		delta = -delta;
		digitalWrite(directionPin, LOW); //Rotate stepper motor in clock wise direction
	}
	else {
		digitalWrite(directionPin, HIGH); //Rotate stepper motor in clock wise direction
	}

	for (int i = 0; i < delta; i++) {
		digitalWrite(stepPin, HIGH);
		delay(3);
		digitalWrite(stepPin, LOW);
		delay(3);
	}
	//setStatus(STATUS_DOOROPEN);
	digitalWrite(enablePin, HIGH); // Disable stepper motor
	*/
	return true;
}



bool RotaryEncoderSensor::openLock()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::openLock"));
	/*
	digitalWrite(enablePin, LOW); // Enable stepper motor
	digitalWrite(directionPin, LOW); //Rotate stepper motor in clock wise direction
	for (int i = 1; i <= 400; i++) {
		digitalWrite(stepPin, HIGH);
		delay(3);
		digitalWrite(stepPin, LOW);
		delay(3);
		position--;
	}
	setStatus(STATUS_DOOROPEN);

	digitalWrite(enablePin, HIGH); // Disable stepper motor
	*/
	logger.print(tag, F("\n\t <<RotaryEncoderSensor::openLock"));

	return true;
}
bool RotaryEncoderSensor::closeLock()
{
	logger.print(tag, F("\n\t >>RotaryEncoderSensor::closeLock"));

	/*digitalWrite(enablePin, LOW); // Enable stepper motor
	digitalWrite(directionPin, HIGH); //Rotate stepper motor in anti clock wise direction
	for (int i = 1; i <= 400; i++) {
		digitalWrite(stepPin, HIGH);
		delay(3);
		digitalWrite(stepPin, LOW);
		delay(3);
		position++;
	}
	setStatus(STATUS_DOORCLOSED);

	digitalWrite(enablePin, HIGH); // Disable stepper motor
	*/
	logger.print(tag, F("\n\t <<RotaryEncoderSensor::closeLock"));

	return true;
}

bool RotaryEncoderSensor::loop()
{
	/*int btn = digitalRead(encoder0Btn);
	Serial.print(btn);
	Serial.print(" ");
	Serial.print(valRotary);
	if (valRotary > lastValRotary)
	{
		Serial.print("  CW");
	}
	if (valRotary) {

		Serial.print("  CCW");
	}
	lastValRotary = valRotary;
	Serial.println(" ");
	delay(250);
	return true;*/
}



