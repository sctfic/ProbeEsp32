#include <Arduino.h>

// #include "SPIFFS.h"
#include "global.h"
#include "network.h"
#include "fileSystem.h"
#include "Web.h"
#include "display.h"
#include "sensors.h"
// #include <SoftwareSerial.h>                               //  Remove if using HardwareSerial or non-uno compatabile device    
unsigned long startTime;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void uploaderDeepSleep(void * parameter){
	for (;;) {
		int i = 150; // 600 = 60 sec
		DeepSleepNow = false;
		while (!WIFI_CONNECTED && i>0 || !DataReady) {
			vTaskDelay( 100 );
			i--;
		}
		if (WIFI_CONNECTED) {
			// read CO2 SPi or I2C
			DeepSleepNow = uploadData(CurrentProbe.toJson().c_str(), CurrentProbe.Settings.SrvDataBase2Post.c_str());
		} else {
			Serial.println("+---> [ERROR] uploadData require wifi !");
		}
		if(CurrentProbe.Settings.EnableDeepSleep && IgnoreDeepSleep <= 0){
			Serial.printf("+---> GO deep_sleep_start(%i) ",CurrentProbe.Settings.MeasurementInterval);
			while (Working || Transfert){
				vTaskDelay(100);
				// Serial.print("-");
			}
			// Serial.println(" !");
			// DataReady = false;
			// Serial.printf("Duration %i Sec",((millis()-startTime)));

			esp_sleep_enable_timer_wakeup(((CurrentProbe.Settings.MeasurementInterval) * 1000 - (millis()-startTime)) * 1000);
			displayDeepSleep();
			esp_deep_sleep_start();
		} else {
			// il n'y a pas de mise en veille donc on pattiente avant les prochaines mesures
			vTaskDelay(((CurrentProbe.Settings.MeasurementInterval) * 1000 - (millis()-startTime)) * 1000);
			IgnoreDeepSleep -= CurrentProbe.Settings.MeasurementInterval;
		}
	}
}
void setup_displayTask() {
	xTaskCreatePinnedToCore(
		manageScreen,
		"refresh screen",		// Name of the task (for debugging)
		20000,							// Stack size (bytes)
		NULL,							// Parameter to pass
		4,								// Task priority
		NULL,							// Task handle
		1 								// force CPU 1
	);
}
void setup_sensorTask() {

	CurrentProbe.Network.Strength.Def = {1,0,std::string("dBm")};
	CurrentProbe.Energy.Battery.Capacity.Def = {1,0,std::string("%")};
	CurrentProbe.Energy.Battery.Voltage.Def = {1,0,std::string("V")};
	CurrentProbe.Energy.PowerSupply.Current.Def = {1,0,std::string("A")};
	CurrentProbe.Energy.PowerSupply.Voltage.Def = {1,0,std::string("V")};

	xTaskCreatePinnedToCore(
		getSensorData,
		"Read sensor data and put on server.",	// Name of the task (for debugging)
		20000,									// Stack size (bytes)
		NULL,									// Parameter to pass
		5,										// Task priority
		NULL,									// Task handle
		0
	);
}
void setup_hearthTask() {
	xTaskCreatePinnedToCore(
		heartPulse,
		"Toogle LD1 on working",    // Name of the task (for debugging)
		2000, 						// Stack size (bytes)
		NULL,    				    // Parameter to pass
		5,     				        // Task priority
		NULL,						// Task handle
		0
	);
}
void setup_uploaderTask() {
	xTaskCreatePinnedToCore(
		uploaderDeepSleep,
		"Upload Data to serveur every [MeasurementInterval] sec",    // Name of the task (for debugging)
		5000, 						// Stack size (bytes)
		NULL,    				    // Parameter to pass
		5,     				        // Task priority
		NULL,						// Task handle
		0
	);
}
void setup() {
	startTime = millis();
	// Serial port for debugging purposes
	Serial.begin(115200);
	// i2cbus = {4,5,15,false,false,false,false,false};
	// print_wakeup_reason();
	setup_hearthTask();
	mutex = xSemaphoreCreateMutex();
	setup_displayTask();
	delay(100);
	setup_sensorTask();
	setup_uploaderTask();
	initSPIFFS();
	loadJsonSettings(SettingsPath);

	if (!initWiFi()){
		initAccessPoint();
  	}
	
	if(check_WiFi_Available()){
		Serial.println("> Wifi");
		Serial.print("+---> ");
		Serial.print(CurrentProbe.Network.Hostname.c_str());
		Serial.print(" <---> ");
		Serial.print(CurrentProbe.Network.SSID.c_str());
		Serial.print(" [");
		Serial.print(CurrentProbe.Network.Strength.toString().c_str());
		Serial.println("]");
		Serial.print("+---> ");
		Serial.print(CurrentProbe.Network.IP.c_str());
		Serial.print("/");
		Serial.print(CurrentProbe.Network.CIDR.c_str());
		Serial.print(" - ");
		Serial.println(CurrentProbe.Network.Gateway.c_str());
	}
	setup_Routing();
}

void loop() {
	int n;
	if (!check_WiFi_Available()){
		Working = true;
		WiFi.disconnect();
		n = WiFi.scanNetworks();
		for (int i = 0; i < n; ++i) {
			// Print SSID and RSSI for each network found
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" " : "*");
			Serial.printf("%s == %s (%i)", WiFi.SSID(i).c_str(), CurrentProbe.Settings.Lan.SSID.c_str(), strcmp(WiFi.SSID(i).c_str(), CurrentProbe.Settings.Lan.SSID.c_str()));

			if (strcmp(WiFi.SSID(i).c_str(), CurrentProbe.Settings.Lan.SSID.c_str()) == 0){
				WiFi.reconnect();
				Serial.println("> Wifi");
				Serial.print("+---> ");
				Serial.print(CurrentProbe.Network.Hostname.c_str());
				Serial.print(" <---> ");
				Serial.print(CurrentProbe.Network.SSID.c_str());
				Serial.print(" [");
				Serial.print(CurrentProbe.Network.Strength.toString().c_str());
				Serial.println("]");
				Serial.print("+---> ");
				Serial.print(CurrentProbe.Network.IP.c_str());
				Serial.print("/");
				Serial.print(CurrentProbe.Network.CIDR.c_str());
				Serial.print(" <---> ");
				Serial.print(CurrentProbe.Network.Gateway.c_str());
				Serial.println(" <---> www");
				
			}
			
		}
		delay(10000);
		Working = false;
	}
	delay(1000);
}