#ifndef _SENSORLISTENER_h
#define _SENSORLISTENER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Logger.h"
class SensorListener {
public:
	SensorListener() {};
	void fireEvent(String oldStatus, String newStatus);
	
	
		
	};
#endif

