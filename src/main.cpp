#include <Arduino.h>

// #include "SPIFFS.h"
#include "global.h"
#include "network.h"
#include "fileSystem.h"
#include "Web.h"
#include "display.h"
#include "sensors.h"




void manageScreen(void * parameter){
	init_Screen();
	for (;;) {
		redrawScreen();
		if (Working || (!Working && OnOff)){
			displayStatus(OnOff);
			refreshScreen();
			vTaskDelay( pdMS_TO_TICKS(100));
		} else {
			// Serial.print(".");
			refreshScreen();
			vTaskDelay( pdMS_TO_TICKS(100));
		}
		if (DeepSleepNow){
			vTaskDelay( pdMS_TO_TICKS(300));
			// enableDeepSleep();
		}
		// Serial.printf("OnOff = %d\n",OnOff);
		// Serial.printf("Working = %d\n",Working);
		// Serial.printf("DeepSleepNow = %d\n",DeepSleepNow); // DeepSleepNow = uploadData
	}
}
void setup_task() {
	xTaskCreatePinnedToCore(
		manageScreen,
		"xTask acces to screen",		// Name of the task (for debugging)
		20000,							// Stack size (bytes)
		NULL,							// Parameter to pass
		6,								// Task priority
		NULL,							// Task handle
		1 								// force CPU 1
	);
	xTaskCreatePinnedToCore(
		getSensorData,
		"Read sensor data and put on server.",   // Name of the task (for debugging)
		20000,								// Stack size (bytes)
		NULL,								// Parameter to pass
		5,									// Task priority
		NULL,								// Task handle
		0
	);
	xTaskCreatePinnedToCore(
		heartPulse,
		"Toogle LD1 on working",    // Name of the task (for debugging)
		2000, 						// Stack size (bytes)
		NULL,    				    // Parameter to pass
		5,     				        // Task priority
		NULL,						// Task handle
		0
	);
	Serial.println("setup_task()");
}

void setup() {
	// Serial port for debugging purposes
	Serial.begin(115200);
	delay(2000);

	mutex = xSemaphoreCreateMutex();
	initSPIFFS();
	loadJsonSettings(SettingsPath);

	if (!initWiFi()){
		initAccessPoint();
  	}
	if(check_WiFi_Available()){
		Serial.println(CurrentProbe.Network.SSID.c_str());
		Serial.println(CurrentProbe.Network.Hostname.c_str());
		Serial.println(CurrentProbe.Network.IP.c_str());
		Serial.println(CurrentProbe.Network.CIDR.c_str());
		Serial.println(CurrentProbe.Network.Gateway.c_str());
		Serial.println(CurrentProbe.Network.DNS.c_str());
		Serial.println(CurrentProbe.Network.Strength.toString().c_str());
		Serial.println(CurrentProbe.Network.MAC.c_str());
		// Print local IP address and start web server
		Serial.println("WiFi connected.");
	}

	setup_Routing();
	setup_task();
}

void loop() {

}