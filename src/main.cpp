#include <Arduino.h>

// #include "SPIFFS.h"
#include "global.h"
#include "network.h"
#include "fileSystem.h"
#include "Web.h"
#include "display.h"
#include "sensors.h"
// #include <SoftwareSerial.h>                               //  Remove if using HardwareSerial or non-uno compatabile device    
#include "Adafruit_VEML7700.h"

Adafruit_VEML7700 veml = Adafruit_VEML7700();

void enableDeepSleep(){
	if (CurrentProbe.Settings.EnableDeepSleep){
		Serial.printf("GO deep_sleep_start(%i) ",CurrentProbe.Settings.MeasurementInterval);
		while (Working || Transfert){
			vTaskDelay(100);
			Serial.print("-");
		}
		Serial.println(" !");
		displayDeepSleep();
		esp_deep_sleep_start();
	}
}
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
void manageScreen(void * parameter){
	init_Screen();
	for (;;) {
		redrawScreen();

		// Serial.printf("OnOff = %d - ",OnOff);
		// Serial.printf("Working = %d - ",Working);
		// Serial.printf("DeepSleepNow = %d\n",DeepSleepNow); // DeepSleepNow = uploadData
		// Serial.printf("%d || (%d && %d)\n",Working, !Working, OnOff);

		if (Working || (!Working && OnOff)){
			displayStatus(OnOff);
			refreshScreen();
			vTaskDelay(300);
		} else {
			refreshScreen();
			vTaskDelay(300);
		}
		if (DeepSleepNow){
			// vTaskDelay( 3000);
			enableDeepSleep();
		}
		delay(1000);
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


void setup() {
	// Serial port for debugging purposes
	Serial.begin(115200);
	// i2cbus = {4,5,15,false,false,false,false,false};
	print_wakeup_reason();
	setup_hearthTask();
	mutex = xSemaphoreCreateMutex();
	setup_displayTask();
	delay(100);

	setup_sensorTask();

	initSPIFFS();
	loadJsonSettings(SettingsPath);
	esp_sleep_enable_timer_wakeup((CurrentProbe.Settings.MeasurementInterval-2) * 1000000);

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


	  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");

  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_800MS);

  Serial.print(F("Gain: "));
  switch (veml.getGain()) {
    case VEML7700_GAIN_1: Serial.println("1"); break;
    case VEML7700_GAIN_2: Serial.println("2"); break;
    case VEML7700_GAIN_1_4: Serial.println("1/4"); break;
    case VEML7700_GAIN_1_8: Serial.println("1/8"); break;
  }

  Serial.print(F("Integration Time (ms): "));
  switch (veml.getIntegrationTime()) {
    case VEML7700_IT_25MS: Serial.println("25"); break;
    case VEML7700_IT_50MS: Serial.println("50"); break;
    case VEML7700_IT_100MS: Serial.println("100"); break;
    case VEML7700_IT_200MS: Serial.println("200"); break;
    case VEML7700_IT_400MS: Serial.println("400"); break;
    case VEML7700_IT_800MS: Serial.println("800"); break;
  }

  //veml.powerSaveEnable(true);
  //veml.setPowerSaveMode(VEML7700_POWERSAVE_MODE4);

  veml.setLowThreshold(10000); //veml.readALS() value for event event
  veml.setHighThreshold(20000); //veml.readALS() value for event event
  veml.interruptEnable(true);
  veml.setGain(VEML7700_GAIN_1_4);
  veml.setIntegrationTime(VEML7700_IT_100MS);
}

void loop() {
	if (!check_WiFi_Available()){
		Working = true;
		WiFi.disconnect();
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
		Serial.print(" - ");
		Serial.println(CurrentProbe.Network.Gateway.c_str());
		Working = false;
		delay(10000);
	}
	// Serial.print("+");
	delay(1000);
  Serial.print("Lux: "); Serial.println(veml.readLux());
  Serial.print("White: "); Serial.println(veml.readWhite());
  Serial.print("Raw ALS: "); Serial.println(veml.readALS());

  uint16_t irq = veml.interruptStatus();
  if (irq & VEML7700_INTERRUPT_LOW) {
    Serial.println("** Low threshold"); 
  }
  if (irq & VEML7700_INTERRUPT_HIGH) {
    Serial.println("** High threshold"); 
  }
	// MEASURE x = MEASURE(1209.09732489);
	// Serial.println(CurrentProbe.Probe.Pressure.toJson().c_str());
	// x.Def = SENSOR_DEF(0.01,1,"$");
	// Serial.println(CurrentProbe.Probe.Pressure.toJson().c_str());
	// Serial.println(CurrentProbe.Probe.Pressure.toString().c_str());
}