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

	/*Serial.print("CUR POS = ");
	Serial.println(position);*/
}

KeyLockSensor::KeyLockSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>KeyLockSensor::KeyLockSensor"));

	type = "keylocksensor";
	checkStatus_interval = 10;
	lastCheckStatus = 0;

	/*stepPin = _stepPin;
	directionPin = _directionPin;
	enablePin = _enablePin;
	outputAPin = _outputAPin;
	outputBPin = _outputBPin;*/

	if (!json.containsKey("steppin"))
		return;
	String strStepPin = json["steppin"];
	strStepPin.replace("\r\n", ""); // importante!!
	stepPin = Shield::pinFromStr(strStepPin);

	if (!json.containsKey("directionpin"))
		return;
	String strDirectionPin = json["directionpin"];
	strDirectionPin.replace("\r\n", ""); // importante!!
	directionPin = Shield::pinFromStr(strDirectionPin);

	if (!json.containsKey("enablepin"))
		return;
	String strEnablePin = json["enablepin"];
	strEnablePin.replace("\r\n", ""); // importante!!
	enablePin = Shield::pinFromStr(strEnablePin);

	if (!json.containsKey("outputapin"))
		return;
	String strOutputAPin = json["outputapin"];
	strOutputAPin.replace("\r\n", ""); // importante!!
	outputAPin = Shield::pinFromStr(strOutputAPin);

	if (!json.containsKey("outputbpin"))
		return;
	String strOutputBPin = json["outputbpin"];
	strOutputBPin.replace("\r\n", ""); // importante!!
	outputBPin = Shield::pinFromStr(strOutputBPin);	

	logger.print(tag, F("\n\t<<KeyLockSensor::KeyLockSensor\n"));
}

KeyLockSensor::~KeyLockSensor()
{
}

void KeyLockSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["position"] = position;
	json["openthreshold"] = openThreshold;
	json["firstlockthreshold"] = firstLockThreshold;
	json["secondlockthreshold"] = secondLockThreshold;
	json["closedthreshold"] = closedThreshold;

	json["lockstatus"] = status;
	json["steppin"] = Shield::getStrPin(stepPin);
	json["directionpin"] = Shield::getStrPin(directionPin);
	json["enablepin"] = Shield::getStrPin(enablePin);
	json["outputapin"] = Shield::getStrPin(outputAPin);
	json["outputbpin"] = Shield::getStrPin(outputBPin);
	//json.printTo(Serial);
}

bool KeyLockSensor::checkStatusChange()
{
	//Sensor::checkStatusChange();

	oldPositionStatus = position;
		
	if (status.equals(STATUS_STOPPING)) {
		updateStatus();
		logger.print(tag, F("\n\t STOPPED"));
		enableMotor(false);

	} else if (status.equals(STATUS_OPENING)) {
		if (position > openThreshold) {
			rotateMotor();
			if (countRotation++ > maxrotation || position < zeroThreshold) {
				enableMotor(false);
				logger.print(tag, F("\n\t FAILED TO OPEN"));
				updateStatus();
				return Sensor::checkStatusChange();
			}
		}
		else
		{
			updateStatus();
			logger.print(tag, F("\n\t END OPENING"));
			enableMotor(false);
		}
	}
	else if (status.equals(STATUS_CLOSING)) {
		if (position < closedThreshold) {
			rotateMotor();
			if (countRotation++ > maxrotation || position > maxThreshold) {
				enableMotor(false);
				logger.print(tag, F("\n\t FAILED TO CLOSE"));
				updateStatus();
				return Sensor::checkStatusChange();;
			}
		}
		else
		{
			//setStatus(STATUS_CLOSED);
			updateStatus();
			logger.print(tag, F("\n\t END CLOSING"));
			enableMotor(false);
		}
	}
	else if (status.equals(STATUS_MOVING)) {
		if ((delta < 0 && position > targetpos) || (delta > 0 && position < targetpos)) {
			rotateMotor();
			if (countRotation++ > maxrotation) {
				enableMotor(false);
				logger.print(tag, F("\n\t FAILED TO SET POSITION"));
				updateStatus();
				return Sensor::checkStatusChange();;
			}
		}
		else {
			updateStatus();
			logger.print(tag, F("\n\t END SET POSITION"));
			enableMotor(false);
		}
	}

	return Sensor::checkStatusChange();
	/*if (!status.equals(oldStatus)) {
		//logger.print(tag, F("\n\t\t SEND STATUS\n\n"));
		sendStatusUpdate();
		//updateAttributes();
	}*/
}

void KeyLockSensor::updateStatus() {
	
	if (position <= firstLockThreshold) {
		logger.print(tag, "\n\t STATUS open " + String(position));
		setStatus(STATUS_OPEN);
	}
	else if (position >= secondLockThreshold) {
		logger.print(tag, "\n\t STATUS closed " + String(position));
		setStatus(STATUS_CLOSED);
	}
	else {
		logger.print(tag, "\n\t STATUS firstlock " + String(position));
		setStatus(STATUS_FIRSTLOCK);
	}

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

	position = 0;
	//openThreshold = 0;
	//closedThreshold = 700;
	firstLockThreshold = 1 * (closedThreshold - openThreshold) / 4;
	secondLockThreshold = 3 * (closedThreshold - openThreshold) / 4;
	oldPositionStatus = -1;

	logger.print(tag, F("\n\t <<init KeyLockSensor"));
}

/*void KeyLockSensor::sendStatusUpdate()
{
	logger.print(tag, F("\n\t\t sendStatusUpdate"));
	String topic = "ESPWebduino/myboard1/" + type + "/" + String(sensorid) + "/pos";
	//int pos = position;
	if (position < 0)
		position = 0;
	if (position > 700)
		position = 700;

	if (mqtt_publish(topic, String(position)))
		oldStatus = status;
}*/

bool KeyLockSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>KeyLockSensor::sendCommand"));

	logger.print(tag, String(F("\n\t\tcommand=")) + command);
	logger.print(tag, String(F("\n\t\tpayload=")) + payload);
	if (command.equals("set")) {
		if (payload.equals("OPEN")) {
			return openLock();
		}
		else if (payload.equals("CLOSE")) {
			return closeLock();
		}
		else if (payload.equals("STOP")) {
			return stopLock();
		}
	}
	else if (command.equals("pos")) {
		int position = payload.toInt();
		return setPosition(position);
	}
	else if (command.equals("zerocalibration")) {
		return zeroCalibration();
	}
	else if (command.equals("openthreshold")) {
		return setOpenThreshold();
	}
	else if (command.equals("firstlockthreshold")) {
		return setFirstLockThreshold();
	}
	else if (command.equals("closedthreshold")) {
		return setClosedThreshold();
	}

	logger.print(tag, F("\n\t <<KeyLockSensor::sendCommand"));
	return false;
}

bool KeyLockSensor::setOpenThreshold()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::setOpenThresold"));
	openThreshold = position;
	logger.print(tag, F("\n\t <<KeyLockSensor::setOpenThresold"));
	return true;
}

bool KeyLockSensor::setFirstLockThreshold()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::setFirstLockThresold"));
	firstLockThreshold = position;
	logger.print(tag, F("\n\t <<KeyLockSensor::setFirstLockThresold"));
	return true;
}

bool KeyLockSensor::setClosedThreshold()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::setClosedThresold"));
	closedThreshold = position;
	logger.print(tag, F("\n\t <<KeyLockSensor::setClosedThresold"));
	return true;
}

bool KeyLockSensor::stopLock()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::stopLock"));
	setStatus(STATUS_STOPPING);
	logger.print(tag, F("\n\t <<KeyLockSensor::stopLock"));
	return true;
}

bool KeyLockSensor::openLock()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::openLock"));

	setStatus(STATUS_OPENING);
	SetMotorDirection(true);
	enableMotor(true);
	countRotation = 0;

	logger.print(tag, F("\n\t <<KeyLockSensor::openLock"));
	return true;
}

bool KeyLockSensor::closeLock()
{
	logger.print(tag, F("\n\t >>KeyLockSensor::closeLock"));

	setStatus(STATUS_CLOSING);
	SetMotorDirection(false);
	enableMotor(true);
	countRotation = 0;

	logger.print(tag, F("\n\t <<KeyLockSensor::closeLock"));
	return true;
}

bool KeyLockSensor::setPosition(int _targetpos) {

	logger.print(tag, "\n\t >>KeyLockSensor::setPosition " + String(_targetpos) + "%");

	setStatus(STATUS_MOVING);
	targetpos = (int)(_targetpos / 100.0 * closedThreshold);
	logger.print(tag, "\n\t targetpos=" + String(targetpos));
	delta = targetpos - position;
	logger.print(tag, "\n\t delta=" + String(delta));

	countRotation = 0;
	if (delta < 0)
		SetMotorDirection(true);
	else
		SetMotorDirection(false);
	enableMotor(true);

	logger.print(tag, F("\n\t <<KeyLockSensor::setPosition"));
	return true;
}

bool KeyLockSensor::zeroCalibration() {
	logger.print(tag, F("\n\t >>KeyLockSensor::zeroCalibration"));

	SetMotorDirection(true);
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
			position = zeroThreshold;
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
}
