#include "OnewireSensor.h"
#include "Shield.h"
#include "TemperatureSensor.h"
#include "SensorFactory.h"

Logger OnewireSensor::logger;
String OnewireSensor::tag = "OnewireSensor";

OnewireSensor::OnewireSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>OnewireSensor::OnewireSensor json"));
	
	createSensor();
	logger.print(tag, F("\n\t<<OnewireSensor::OnewireSensor json\n"));
}

void OnewireSensor::createSensor() {
	logger.print(tag, F("\n\t>>OnewireSensor::createSensor"));

	checkStatus_interval = 20000;//60000;
	lastCheckStatus = 0;
	type = "onewiresensor";

	beginTemperatureSensors();

	logger.print(tag, F("\n\t<<OnewireSensor::createSensor\n"));
}

void OnewireSensor::loadChildren(JsonArray& jsonarray)
{
	logger.print(tag, "\n\n\t>>OnewireSensor::loadChildren jsonarray.size()=" + String(jsonarray.size()));
	Sensor::loadChildren(jsonarray);
	/*for (int i = 0; i < jsonarray.size(); i++) {
		
		DynamicJsonBuffer* pDynamicJsonBuffer = new DynamicJsonBuffer();
		jsonBuffer[i] = pDynamicJsonBuffer;
		
		String name = jsonarray[i]["name"];
		String subaddress = "sub-" + String(i);
		TemperatureSensor* child = (TemperatureSensor*)SensorFactory::createSensor(jsonarray[i]);
		if (child != nullptr) {
			//child->id = id++;
			childsensors.add(child);
		}
	}*/
	logger.print(tag, F("\n\t<<OnewireSensor::loadChildren\n"));
}

OnewireSensor::~OnewireSensor()
{
}

void OnewireSensor::init()
{

}

void OnewireSensor::beginTemperatureSensors()
{
	logger.print(tag, F("\n\n\t >>beginTemperatureSensors pin="));
	logger.print(tag, String(pin));
	oneWirePtr = new OneWire(pin);
	pDallasSensors = new DallasTemperature(oneWirePtr);
	pDallasSensors->begin();
			
	logger.print(tag, "\n\t search for 1-Wire devices.....");
	for (int i = 0; i < childsensors.size(); i++) {
		TemperatureSensor* child = (TemperatureSensor*)childsensors.get(i);
		uint8_t* address = child->sensorAddr;
		child->unavailable = false;
		if (oneWirePtr->search(address)) {
			logger.print(tag, F("\n\tFound \'1-Wire\' device with _address:"));
			for (int i = 0; i < 8; i++) {
				logger.print(tag, "0x");
				if (address[i] < 16) {
					logger.print(tag, '0');
				}
				logger.print(tag, address[i]);
				if (i < 7) {
					logger.print(tag, ", ");
				}
			}
			if (OneWire::crc8(address, 7) != address[7]) {
				logger.print(tag, F("\n\t CRC is not valid!"));
				return;
			}
			
			float dallasTemperature = pDallasSensors->getTempC(address);
			child->temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);

			logger.print(tag, F("\n\t dallas Temperature   is: "));
			logger.print(tag, String(dallasTemperature));
			logger.print(tag, F("\n\t addr="));
			logger.println(tag, child->getPhisicalAddress());

		}
		else {
			logger.print(tag, F("\n\n\t TEMPERATURE SENSOR NOT FOUND\n")); 
			break;
		}
	}
	oneWirePtr->reset_search();

	logger.print(tag, F("\n\t<<beginTemperatureSensors\n"));
}

bool OnewireSensor::readTemperatures() {

	logger.print(tag, F("\n\n\t >>readTemperatures"));
	// questa funzione ritorna true se è cambiata almeno uan tempertura
	if (childsensors.size() == 0)
		return false;

	int res = false; // 

	pDallasSensors->requestTemperatures(); // Send the command to get temperatures

	for (int i = 0; i < childsensors.size(); i++) {
		TemperatureSensor* tempSensor = (TemperatureSensor*)childsensors.get(i);
		if (tempSensor->unavailable)
			continue;

		// call dallasSensors.requestTemperatures() to issue a global temperature 
		// request to all devices on the bus
		logger.print(tag, F("n\t sensor: "));
		logger.print(tag, tempSensor->name);
		logger.print(tag, F("\n\t index: "));
		logger.print(tag, i);
		logger.print(tag, F("\n\t addr "));
		logger.print(tag, tempSensor->getPhisicalAddress());

		float oldTemperature = tempSensor->temperature;
		logger.print(tag, F("\n\t old Temperature   is: "));
		logger.print(tag, String(oldTemperature));

		float dallasTemperature = pDallasSensors->getTempC(tempSensor->sensorAddr);

		if (dallasTemperature == -127)
			tempSensor->setStatus(tempSensor->STATUS_UNAVAILABLE);
		else 
			tempSensor->setStatus(STATUS_IDLE);

		logger.print(tag, F("\n\t dallas Temperature   is: "));
		logger.print(tag, String(dallasTemperature));

		tempSensor->temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		//temperatureSensors[i].temperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		logger.print(tag, F("\n\t rounded Temperature  is: "));
		logger.print(tag, String(tempSensor->temperature));

		// se è cambiata almeno una temperatura ritorna true
		if (oldTemperature != tempSensor->temperature)
			res = true;

		if (avTempCounter < avTempsize) {
			avTemp[avTempCounter] = tempSensor->temperature;
			avTempCounter++;
		}
		else {
			for (int i = 0; i < avTempCounter - 1; i++)
			{
				avTemp[i] = avTemp[i + 1];
			}
			avTemp[avTempCounter - 1] = tempSensor->temperature;
		}
		float average = 0.0;
		for (int i = 0; i < avTempCounter; i++) {
			average += avTemp[i];
		}
		average = average / (avTempCounter);
		tempSensor->avTemperature = (((int)(dallasTemperature * 10 + .5)) / 10.0);
		//temperatureSensors[i].avTemperature = ((int)(average * 100 + .5) / 100.0);

		logger.print(tag, F("\n\tAverage temperature  is: "));
		logger.print(tag, String(tempSensor->avTemperature));
		logger.print(tag, F("\n"));
	}

	if (res)
		logger.print(tag, F("\n\n\t --temperatura cambiata"));
	else
		logger.print(tag, F("\n\n\t >>readTemperatures - temperatura non cambiata"));

	return res;
}

void OnewireSensor::getJson(JsonObject& json) {
	logger.print(tag, F("\n\t>>OnewireSensor::getJson"));
	Sensor::getJson(json);
	//logger.printJson(json);
	logger.print(tag, F("\n\t<<OnewireSensor::getJson"));
}

void OnewireSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	//logger.println(tag, "\n\t currMillis="+String(currMillis) + "timeDiff=" + String(timeDiff));
	if (timeDiff > checkStatus_interval) {
		logger.print(tag, F("\n\n"));
		//logger.println(tag, ">>checkStatusChange()::checkTemperatures timeDiff:" + String(timeDiff) + " checkStatus_interval:" + String(checkStatus_interval));
		lastCheckStatus = currMillis;
		bool temperatureChanged = readTemperatures();
		if (temperatureChanged)
			logger.println(tag, F("temperatura cambiata"));
		else
			logger.println(tag, F("temperatura NON cambiata"));
		logger.print(tag, F("\n\n"));
		logger.println(tag, F("<<checkStatusChange()::checkTemperatures"));

		//sendStatusUpdate();
		//return temperatureChanged;
		Sensor::checkStatusChange();
	}
	//return false;
}
