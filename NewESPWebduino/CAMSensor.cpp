#include "CAMSensor.h"
#include "Shield.h"

Logger CAMSensor::logger;
String CAMSensor::tag = "DoorSensor";

String  CAMSensor::STATUS_DOOROPEN = "dooropen";
String  CAMSensor::STATUS_DOORCLOSED = "doorclosed";

String  CAMSensor::MODE_NORMAL = "normal";
String  CAMSensor::MODE_TEST = "test";
String  CAMSensor::MODE_TESTOPEN = "testopen";


#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>

boolean takeNewPhoto = false;

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22



CAMSensor::CAMSensor(JsonObject& json) : Sensor(json)
{
	logger.print(tag, F("\n\t>>CAMSensor::CAMSensor"));
	
	type = "camsensor";
	
	checkStatus_interval = 1000;
	lastCheckStatus = 0;

	logger.print(tag, F("\n\t<<CAMSensor::CAMSensor\n"));
}

void CAMSensor::init()
{
	logger.print(tag, "\n\t >>init CAMSensor pin=" + String(pin));
	Sensor::init();
	Serial.println(WiFi.localIP());

	// Turn-off the 'brownout detector'
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

	// OV2640 camera module
	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk = XCLK_GPIO_NUM;
	config.pin_pclk = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;

	if (psramFound()) {
		config.frame_size = FRAMESIZE_UXGA;
		config.jpeg_quality = 10;
		config.fb_count = 2;
	}
	else {
		config.frame_size = FRAMESIZE_SVGA;
		config.jpeg_quality = 12;
		config.fb_count = 1;
	}
	// Camera init
	esp_err_t err = esp_camera_init(&config);
	if (err != ESP_OK) {
		Serial.printf("Camera init failed with error 0x%x", err);
		ESP.restart();
	}




	//pinMode(pin, INPUT);
	mode = MODE_NORMAL;
	logger.print(tag, F("\n\t <<init CAMSensor"));
}

void CAMSensor::getJson(JsonObject& json) {
	Sensor::getJson(json);
	json["mode"] = mode;
}

void CAMSensor::checkStatusChange() {

	unsigned long currMillis = millis();
	unsigned long timeDiff = currMillis - lastCheckStatus;
	bool ret = false;
	if (timeDiff > checkStatus_interval) {
		lastCheckStatus = currMillis;
				
		if (takeNewPhoto) {
			capturePhotoSaveSpiffs();
			takeNewPhoto = false;
		}


	}
	Sensor::checkStatusChange();
}

bool CAMSensor::sendCommand(String command, String payload)
{
	logger.print(tag, F("\n\t >>CAMSensor::sendCommand"));

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
		} else if (payload.equals("closed")) {
			setStatus(STATUS_DOORCLOSED);
		}
	}
	logger.print(tag, F("\n\t <<DoorSensor::sendCommand"));
	return false;
}

// Check if photo capture was successful
bool CAMSensor::checkPhoto(fs::FS& fs) {
	File f_pic = fs.open(FILE_PHOTO);
	unsigned int pic_sz = f_pic.size();
	return (pic_sz > 100);
}

// Capture Photo and Save it to SPIFFS
void CAMSensor::capturePhotoSaveSpiffs(void) {
	camera_fb_t* fb = NULL; // pointer
	bool ok = 0; // Boolean indicating if the picture has been taken correctly

	do {
		// Take a photo with the camera
		Serial.println("Taking a photo...");

		fb = esp_camera_fb_get();
		if (!fb) {
			Serial.println("Camera capture failed");
			return;
		}

		// Photo file name
		Serial.printf("Picture file name: %s\n", FILE_PHOTO);
		File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

		// Insert the data in the photo file
		if (!file) {
			Serial.println("Failed to open file in writing mode");
		}
		else {
			file.write(fb->buf, fb->len); // payload (image), payload length
			Serial.print("The picture has been saved in ");
			Serial.print(FILE_PHOTO);
			Serial.print(" - Size: ");
			Serial.print(file.size());
			Serial.println(" bytes");
		}
		// Close the file
		file.close();
		esp_camera_fb_return(fb);

		// check if file has been correctly saved in SPIFFS
		ok = checkPhoto(SPIFFS);
	} while (!ok);
}

