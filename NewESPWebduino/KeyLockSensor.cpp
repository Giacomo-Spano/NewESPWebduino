#include "KeyLockSensor.h"
#include "Shield.h"

Logger KeyLockSensor::logger;
String KeyLockSensor::tag = "KeyLockSensor";

#ifdef ESP8266
int KeyLockSensor::outputAPin = D1;// 2;// D1;
int KeyLockSensor::outputBPin = D2; // 4;// D2;
#endif
#ifdef ESP32
int KeyLockSensor::outputAPin = 18;// D18;// 2;// D1;
int KeyLockSensor::outputBPin = 19;// D19; // 4;// D2;
#endif

extern bool mqtt_publish(String topic, String message);

volatile long lastPosition = 0;
volatile long position = 0;// 360;
int lastMSB2 = 0;
int lastLSB2 = 0;
void ICACHE_RAM_ATTR updateEncoder() {
	int MSB = digitalRead(KeyLockSensor::outputAPin); //MSB = most significant bit
	int LSB = digitalRead(KeyLockSensor::outputBPin); //LSB = least significant bit

	int encoded = (MSB << 1) | LSB;
	//converting the 2 pin value to single number 
	int sum = (lastPosition << 2) | encoded;
	//adding it to the previous encoded value 
	if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) position++;
	if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) position--;
	lastPosition = encoded; //store this value for next time 

	//Serial.print("CUR POS = ");
	//Serial.println(position);
}

void KeyLockSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["lockstatus"] = status;

	json["steppin"] = Shield::getStrPin(stepPin);
	json["directionpin"] = Shield::getStrPin(directionPin);
	json["enablepin"] = Shield::getStrPin(enablePin);
	json["outputapin"] = Shield::getStrPin(outputAPin);
	json["outputbpin"] = Shield::getStrPin(outputBPin);
}

KeyLockSensor::KeyLockSensor(int id, uint8_t pin, bool enabled, String address, String name, uint8_t _stepPin, uint8_t _directionPin, uint8_t _enablePin, uint8_t _outputAPin, uint8_t _outputBPin) : Sensor(id, pin, enabled, address, name)
{	
	type = "keylocksensor";
	checkStatus_interval = 10;
	lastCheckStatus = 0;

	stepPin = _stepPin;
	directionPin = _directionPin;
	enablePin = _enablePin;
	outputAPin = _outputAPin;
	outputBPin = _outputBPin;
}

KeyLockSensor::~KeyLockSensor()
{
}

void KeyLockSensor::checkStatusChange()
{
	//logger.print(tag, F("\n\t\t checkStatusChange"));
	if (oldPositionStatus != position) {
		sendStatusUpdate();
		updateAttributes();
	}
	oldPositionStatus = position;
}

void KeyLockSensor::init()
{
	logger.print(tag, F("\n\t >>init KeyLockSensor"));
	logger.print(tag, F("\n\t stepPin="));
	logger.print(tag, String(stepPin));
	logger.print(tag, F("\n\t directionPin="));
	logger.print(tag, String(directionPin));
	logger.print(tag, F("\n\t enablePin="));
	logger.print(tag, String(enablePin));
	logger.print(tag, F("\n\t outputAPin="));
	logger.print(tag, String(outputAPin));
	logger.print(tag, F("\n\t outputBPin="));
	logger.print(tag, String(outputBPin));

	pinMode(outputAPin, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
	pinMode(outputBPin, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
	attachInterrupt(digitalPinToInterrupt(outputAPin), updateEncoder, CHANGE);
	attachInterrupt(digitalPinToInterrupt(outputBPin), updateEncoder, CHANGE);

#ifdef ESP8266
	stepPin = D4; //GPIO0---D3 of Nodemcu--Step of stepper motor driver
	directionPin = D3; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver
	enablePin = D5; //GPI
#endif
#ifdef ESP32
	//stepPin = 19; //GPIO0---D3 of Nodemcu--Step of stepper motor driver
	//directionPin = 18; //GPIO2---D4 of Nodemcu--Direction of stepper motor driver
	//enablePin = 21; //GPI
#endif

	pinMode(stepPin, OUTPUT); //Steppin as output
	pinMode(directionPin, OUTPUT); //Directionpin as output
	pinMode(enablePin, OUTPUT); // Enablepin as output
	digitalWrite(stepPin, LOW); // Currently no stepper motor movement
	digitalWrite(directionPin, LOW);

	enableMotor(false);
	
	position = 700;
	openThreshold = 700;
	closedThreshold = 0;
	firstLockThreshold = 2 * (closedThreshold - openThreshold) / 3 + openThreshold;
	secondLockThreshold = 1 * (closedThreshold - openThreshold) / 3 + openThreshold;
	oldPositionStatus = -1;

	logger.print(tag, F("\n\t <<init KeyLockSensor"));
}

void KeyLockSensor::sendStatusUpdate()
{
	logger.print(tag, F("\n\t\t sendStatusUpdate"));
	String topic = "ESPWebduino/myboard1/" + type + "/" + String(sensorid) + "/pos";
	int pos = position;
	if (position < 0)
		position = 0;
	if (position > 700)
		position = 700;

	if (mqtt_publish(topic, String(position)))
		oldStatus = status;
}

bool KeyLockSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>KeyLockSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("set")) {
		if (payload.equals("OPEN")) {
			Serial.println("\n\n OPEN\n");
			bool ret = openLock();
			return ret;
		}
		else if (payload.equals("CLOSE")) {
			Serial.println("\n\n CLOSE\n");
			bool ret = closeLock();
			return ret;
		}
	}
	else if (command.equals("pos")) {
		int position = payload.toInt();
		Serial.println("\n\n SET POSITION\n");
		return setPosition(position);
	}
	else if (command.equals("zerocalibration")) {
		Serial.println("\n\n ZEROCALIBRATION\n");
		return zeroCalibration();
	}
	logger.print(tag, F("\n\t <<KeyLockSensor::sendCommand"));
	return false;
}

bool KeyLockSensor::closeLock()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::openLock"));

	SetMotorDirection(false);
	enableMotor(true);
	int count = 0;
	while (position < openThreshold) {
		rotateMotor();
		if (count++ > maxrotation) {
			enableMotor(false);
			logger.print(tag, F("\n\t FAILED\n<<KeyLockSensor::openLock"));
			return false;
		}
	}
	enableMotor(false);

	logger.print(tag, F("\n\t <<KeyLockSensor::openLock"));
	return true;
}


bool KeyLockSensor::openLock()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::closeLock"));

	SetMotorDirection(true);
	enableMotor(true);	
	int count = 0;
	while (position > closedThreshold) {
		rotateMotor();
		if (count++ > maxrotation) {
			enableMotor(false);
			logger.print(tag, F("\n\t FAILED\n<<KeyLockSensor::closeLock"));
			return false;
		}
	}
	enableMotor(false);

	logger.print(tag, F("\n\t <<KeyLockSensor::closeLock"));
	return true;
}

bool KeyLockSensor::setPosition(int targetpos) {

	logger.print(tag, "\n\t >>KeyLockSensor::setPosition " + String(targetpos));

	targetpos = (int)(targetpos / 100.0 * openThreshold);
	double delta = targetpos - position;
			
	int count = 0;
	if (delta < 0) {
		SetMotorDirection(true);
		enableMotor(true);
		while (position > targetpos) {
			rotateMotor();
			if (count++ > maxrotation) {
				enableMotor(false);
				logger.print(tag, F("\n\t FAILED\n<<KeyLockSensor::setPosition"));
				return false;
			}
		}
	}
	else {
		SetMotorDirection(false);
		enableMotor(true);
		while (position < targetpos) {
			rotateMotor();
			if (count++ > maxrotation) {
				enableMotor(false);
				logger.print(tag, F("\n\t FAILED\n<<KeyLockSensor::setPosition"));
				return false;
			}
		}
	}
	enableMotor(false);

	logger.print(tag, F("\n\t <<KeyLockSensor::closeLock"));
	return true;
}

bool KeyLockSensor::zeroCalibration() {
	logger.print(tag, F("\n\t >>KeyLockSensor::zeroCalibration"));

	SetMotorDirection(false);
	enableMotor(true);
	int count = 0;
	int lockedCounter = 0;
	long oldPos = position;
	while (true) {
		rotateMotor();

		logger.print(tag, "\npos= " + String(position));
		if (oldPos == position) {
			lockedCounter++;
		}
		else {
			lockedCounter = 0;
			oldPos = position;
		}

		if (lockedCounter > 10) {
			
			enableMotor(false);
			position = openThreshold;
			oldPos = -1;
			setStatus(STATUS_OPEN);
			logger.print(tag, "\n\t CALIBRATION SUCCESFUL");
			logger.print(tag, "\n\t <<KeyLockSensor::zeroCalibration");
			return true;
		}


		if (count++ > maxrotation) {
			enableMotor(false);
			logger.print(tag, "\n\t CALIBRATION FAILED");
			logger.print(tag, F("\n\t <<KeyLockSensor::zeroCalibration"));
			return false;
		}
	}
	enableMotor(false);
	logger.print(tag, "\n\t CALIBRATION FAILED");
	logger.print(tag, "\n\t <<KeyLockSensor::zeroCalibration");
	return false;

}

void KeyLockSensor::enableMotor(bool enable) {
	if (enable)
		digitalWrite(enablePin, LOW); // Disable stepper motor
	else 
		digitalWrite(enablePin, HIGH); // Disable stepper motor
}

void KeyLockSensor::SetMotorDirection(bool clockwise) {
	if (clockwise)
		digitalWrite(directionPin, HIGH); //Rotate stepper motor in clock wise direction
	else
		digitalWrite(directionPin, LOW); //Rotate stepper motor in clock wise direction
}

void KeyLockSensor::rotateMotor() {

	digitalWrite(stepPin, HIGH);
	delay(1);
	digitalWrite(stepPin, LOW);
	delay(1);

	if (position <= firstLockThreshold) {
		//logger.print(tag, "\n\t STATUS open " + String(position));
		setStatus(STATUS_OPEN);
	}
	else if (position >= secondLockThreshold) {
		//logger.print(tag, "\n\t STATUS closed " + String(position));
		setStatus(STATUS_CLOSED);
	}
	else {
		//logger.print(tag, "\n\t STATUS firstlock " + String(position));
		setStatus(STATUS_FIRSTLOCK);
	}
}

/*
bool KeyLockSensor::rotateLock(bool close, int delta) {

	logger.print(tag, F("\n\t >>KeyLockSensor::rotateLock"));
	logger.print(tag, "\n\t INITIAL POSITION = " + String(position));

	if (!close) {
		digitalWrite(directionPin, LOW); //Rotate stepper motor in clock wise direction
	}
	else {
		digitalWrite(directionPin, HIGH); //Rotate stepper motor in clock wise direction
	}

	digitalWrite(enablePin, LOW); // Enable stepper motor

	int oldPos = position;
	int count = 0;
	for (int i = 0; i < delta; i++) {
		digitalWrite(stepPin, HIGH);
		delay(1);
		digitalWrite(stepPin, LOW);
		delay(1);

		if (position <= firstLockThreshold) {
			logger.print(tag, "\n\t STATUS OPEN " + String(position));

			setStatus(STATUS_OPEN);
		}
		else if (position >= secondLockThreshold) {
			logger.print(tag, "\n\t STATUS CLOSEd " + String(position));
			setStatus(STATUS_CLOSED);
		}
		else {
			logger.print(tag, "\n\t STATUS FIRSTLOCK " + String(position));
			setStatus(STATUS_FIRSTLOCK);
		}

		if (i > 1000)
			break;
	}
	digitalWrite(enablePin, HIGH); // Disable stepper motor

	logger.print(tag, "\n\t CURRENT POSITION = " + String(position));
	logger.print(tag, F("\n\t <<KeyLockSensor::rotateLock"));
	return true;
}
*/