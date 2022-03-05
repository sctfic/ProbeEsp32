#include <Arduino.h>
#include <WiFi.h>
// #include <ESPAsyncTCP.h>
// #include <ESPAsyncWebServer.h>
#include <WebServer.h>
// #include <string.h>
#include <HTTPClient.h>
// #include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#include "global.h"
#include "display.h"

// #define DELAY_READ_PROBES 30 // in seconds
#define BATTERY_PIN 35
#define POWER_PIN 32
#define LD1_PIN 16
const int  FREQ = 5000;
const int  LD1CHANNEL = 0;
const int  RESOLUTION = 8;

// // JSON data buffer
// StaticJsonDocument<500> jsonDocument;
// char json[500];

// #include <SPI.h>
// #include <Wire.h>

// Web server running on port 80
WebServer server(80);

// Sensor
// Adafruit_BME280 bme;

// Neopixel LEDs strip
// Adafruit_NeoPixel pixels(NUM_OF_LEDS, PIN, NEO_GRB + NEO_KHZ800);


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30      /* Time ESP32 will go to sleep (in seconds) */

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

bool check_WiFi_connected (){
	WIFI_CONNECTED = !(WiFi.status() != WL_CONNECTED || ((int)WiFi.localIP() == 1073550736));
	return WIFI_CONNECTED;
}
void SettingManager(){

}
void SettingSave(){

}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
 
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
 
  server.send(404, "text/plain", message);
}

bool loadFromSPIFFS(String path) {
  String dataType = "text/html";
 
  Serial.print("Requested page -> ");
  Serial.println(path);
  if (SPIFFS.exists(path)){
      File dataFile = SPIFFS.open(path, "r");
      if (!dataFile) {
          handleNotFound();
          return false;
      }
 
      if (server.streamFile(dataFile, dataType) != dataFile.size()) {
        Serial.println("Sent less data than expected!");
      }else{
          Serial.println("Page served!");
      }
 
      dataFile.close();
  }else{
      handleNotFound();
      return false;
  }
  return true;
}
 
// Handle root url (/)
void handle_root() {
  loadFromSPIFFS("/index.html");
}
bool init_WiFi (){
  // Connect to Wi-Fi network with SSID and PWD
	int wait = 100;
	int i = 0;
	int retry = 0;
	Working = true;
  	// delay(2000);

	Serial.print("Connecting to wifi ");
	// https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	WiFi.setHostname(CurrentProbe.Network.Hostname.c_str());
	// WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);

	WiFi.begin(CurrentProbe.Network.SSID.c_str(), CurrentProbe.Settings.PWD.c_str());

	while (!check_WiFi_connected()) {
		// Serial.println((int)WiFi.localIP());
		Serial.print(".");
		delay(200);
		if ( i >= wait ) {
			i=0;
			Serial.println("");

			if (retry == 3) {
				return false;
			} 
			retry++;
			wait = wait * 1.2;
			WiFi.disconnect();
			WiFi.reconnect();
		}
		i++;
	}
	CurrentProbe.Network.Hostname       = WiFi.getHostname();
	CurrentProbe.Network.IP             = WiFi.localIP().toString().c_str();
	CurrentProbe.Network.Gateway        = WiFi.gatewayIP().toString().c_str();
	CurrentProbe.Network.CIDR           = std::to_string(WiFi.subnetCIDR()).c_str() ;
	CurrentProbe.Network.MAC            = WiFi.macAddress().c_str();
	CurrentProbe.Network.DNS            = WiFi.dnsIP().toString().c_str();
	CurrentProbe.Network.Strength       = MEASURE(WiFi.RSSI(), "dBm");

	Serial.println(WiFi.getHostname());
	Serial.println(WiFi.localIP().toString().c_str());
	Serial.println(WiFi.gatewayIP().toString().c_str());
	Serial.println(WiFi.subnetCIDR());
	Serial.println(WiFi.macAddress().c_str());
	Serial.println(WiFi.dnsIP().toString().c_str());
	// Print local IP address and start web server
	Serial.println("WiFi connected.");
	return true;
}
bool init_WiFiManager(){
		// Connect to Wi-Fi network with SSID and password
	Serial.println("Setting AP (Access Point)");
	// NULL sets an open Access Point
	WiFi.softAP("ESP-WIFI-MANAGER", NULL);

	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);

	// Web Server Root URL
	server.on("/", handle_root);

	// server.on("/", HTTP_GET, SettingManager);
	server.on("/", HTTP_POST, SettingSave);

// 		// HTTP POST gateway value
// 		if (p->name() == PARAM_INPUT_4) {
// 			gateway = p->value().c_str();
// 			Serial.print("Gateway set to: ");
// 			Serial.println(gateway);
// 			// Write file to save value
// 			writeFile(SPIFFS, gatewayPath, gateway.c_str());
// 		}
// 		//Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
// 		}
// 	request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
// 	delay(3000);
// 	ESP.restart();
// 	});
// 	server.begin();
}

void reconnect_WiFi(){
	while (!check_WiFi_connected()) {
		Working = true;
		WiFi.disconnect();
		WiFi.reconnect();
		CurrentProbe.Network.Hostname       = WiFi.getHostname();
		CurrentProbe.Network.IP             = WiFi.localIP().toString().c_str();
		CurrentProbe.Network.Gateway        = WiFi.gatewayIP().toString().c_str();
		CurrentProbe.Network.CIDR           = std::to_string(WiFi.subnetCIDR()).c_str() ;
		CurrentProbe.Network.MAC            = WiFi.macAddress().c_str();
		CurrentProbe.Network.DNS            = WiFi.dnsIP().toString().c_str();
		CurrentProbe.Network.Strength       = MEASURE(WiFi.RSSI(), "dBm");
	}
	Serial.print(CurrentProbe.Network.toJson().c_str());
	Working = false;
}
// Initialize SPIFFS
void initSPIFFS() {
	if (!SPIFFS.begin(true)) {
		Serial.println("An error has occurred while mounting SPIFFS");
	}
	Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
char* readFile(fs::FS &fs, const char * path){
	Serial.printf("Reading file: %s\r\n", path);

	File file = fs.open(path);
	if(!file || file.isDirectory()){
		Serial.println("- failed to open file for reading");
		return "";
	}
	char NL = 10;
	char * fileContent;
	while(file.available()){
		fileContent = (char*)file.readStringUntil(NL).c_str();
		break;     
	}
	return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
	Serial.printf("Writing file: %s\r\n", path);

	File file = fs.open(path, FILE_WRITE);
	if(!file){
		Serial.println("- failed to open file for writing");
		return;
	}
	if(file.print(message)){
		Serial.println("- file written");
	} else {
		Serial.println("- frite failed");
	}
}

void ApiJson(){
	Working = true;
	Serial.printf("API Reply JSON %d\n",Working);
	server.send(200, "application/json", CurrentProbe.toJson().c_str());
	delay(2000);
	Working = false;
}
bool postDataToServer() {
	// Block until we are able to connect to the WiFi access point
	if (WIFI_CONNECTED && DeepSleepNow == false) {
		WiFiClient Wclient;  // or WiFiClientSecure for HTTPS
		HTTPClient httpClient;

		// Send request
		httpClient.begin(Wclient, CurrentProbe.Settings.SrvDataBase2Post.c_str());
		Serial.printf("POST(JSON) to %s > ",CurrentProbe.Settings.SrvDataBase2Post.c_str());
		httpClient.addHeader("Content-Type", "application/json");
		int httpResponseCode = httpClient.POST(CurrentProbe.toJson().c_str());

		// Read response
		Serial.println(httpResponseCode);
		Serial.println(httpClient.getString());

		// Disconnect
		httpClient.end();
		return httpResponseCode == 201 ? true : false;
	}
	return false;
}
void getBatteryCapacity(){
	//  equation d'une Li-Ion sous faible charge a 20C
	if (CurrentProbe.Energy.PowerSupply.Voltage.Value > CurrentProbe.Energy.Battery.Voltage.Value && CurrentProbe.Energy.PowerSupply.Voltage.Value > 4.2){
		CurrentProbe.Energy.Battery.Capacity = MEASURE(101,"Charging!");
	// } else if (CurrentProbe.Energy.Battery.Voltage.Value < 2.76){
	// 	CurrentProbe.Energy.Battery.Capacity = MEASURE(-1,"No Battery");
	} else {
		CurrentProbe.Energy.Battery.Capacity = MEASURE((int)lround(-47391.44 + 68111.053*CurrentProbe.Energy.Battery.Voltage.Value -38668.575*pow(CurrentProbe.Energy.Battery.Voltage.Value,2) + 10828.2011*pow(CurrentProbe.Energy.Battery.Voltage.Value,3) + -1494.5436*pow(CurrentProbe.Energy.Battery.Voltage.Value,4) + 81.37904*pow(CurrentProbe.Energy.Battery.Voltage.Value,5)),"%");
	}
}
void getBatteryVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(BATTERY_PIN) / 4096 * 3.3);
	//   Serial.printf("TensionPin(35): %f\n",Vread);
	CurrentProbe.Energy.Battery.Voltage =  MEASURE(Vread*(20120+9670)/20120,"V");
	// Serial.printf("Tension Battery: %fV\n",BatVoltage);
}
void getPowerVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(POWER_PIN) / 4096 * 3.3);
	//   Serial.printf("TensionPin(35): %f\n",Vread);
	CurrentProbe.Energy.PowerSupply.Voltage = MEASURE(Vread*2,"V");
	// Serial.printf("Tension Battery: %fV\n",BatVoltage);
}
void getSensorData(void * parameter) {
	for (;;) {
		if (WIFI_CONNECTED) {
			Working = true;
			CurrentProbe.Probe.Temperature = MEASURE(8.3,"°C"); // bme.readTemperature();
			CurrentProbe.Probe.Humidity = MEASURE(56.2,"%"); // bme.readHumidity();
			CurrentProbe.Probe.Pressure = MEASURE(1003.5,"hPa"); // bme.readPressure() / 100;
			CurrentProbe.Probe.CO2 = MEASURE(415.5,"ppm");
			getPowerVoltage();
			getBatteryVoltage();
			getBatteryCapacity();
			xSemaphoreTake( mutex, portMAX_DELAY );
				// read CO2 SPi or I2C
			xSemaphoreGive( mutex );
			DeepSleepNow = postDataToServer();
			vTaskDelay( pdMS_TO_TICKS( 2000 ) );
			Working = false;
		} else {
			vTaskDelay( pdMS_TO_TICKS( 200 ) );
		}
	}
}

void handlePost() {
  if (server.hasArg("plain") == false) {
	  Serial.println("");
  }

  String body = server.arg("plain");
  Serial.println(body);
//   deserializeJson(jsonDocument, body);
//   Serial.println(jsonDocument);

  // Respond to the client
  server.send(200, "application/json", "{\"OK\"}");
}
// setup API resources
void setup_routing() {
	if (WIFI_CONNECTED){
		server.on("/", ApiJson);
		server.on("/msg", HTTP_POST, handlePost);
		Serial.println("setup_routing()");
		// start server
		server.begin();
	} else {
		Serial.println("Network is Missing!");
	}
}
void isWorking(void * parameter){
	for (;;) {
			// digitalWrite(LD1_PIN, OnOff);
			OnOff = !OnOff;
			// Max LED brightness 0, Min 255
			for(int dutyCycle = 253; dutyCycle >= 140; dutyCycle-=2+Working*2){
				ledcWrite(LD1CHANNEL, dutyCycle);   
				vTaskDelay(4-Working*2);
			}
			OnOff = !OnOff;
			for(int dutyCycle = 140; dutyCycle <= 250; dutyCycle+=2+Working*2){   
				ledcWrite(LD1CHANNEL, dutyCycle);
				vTaskDelay(7-Working*2);
			}
			OnOff = !OnOff;
			for(int dutyCycle = 250; dutyCycle >= 100; dutyCycle-=2+Working*2){
				ledcWrite(LD1CHANNEL, dutyCycle);   
				vTaskDelay(4-Working*2);
			}
			OnOff = !OnOff;
			for(int dutyCycle = 100; dutyCycle <= 253; dutyCycle+=2+Working*2){   
				ledcWrite(LD1CHANNEL, dutyCycle);
				vTaskDelay(7-Working*2);
			}
			OnOff = !OnOff;
		vTaskDelay( (100));
	}
}
// void waitHttpRequest(void * parameter){
// 	setup_routing();
// 	for (;;) {
// 		server.handleClient();
// 		vTaskDelay(500 / portTICK_PERIOD_MS);
// 	}
// }
void enableDeepSleep(){
	Serial.println("GO deep_sleep_start()");
	displayDeepSleep();
	if (CurrentProbe.Settings.EnableDeepSleep){
		esp_deep_sleep_start();
	}
}

void manageScreen(void * parameter){
	// init_Screen();
	for (;;) {
		redrawScreen();
		if (Working || (!Working && OnOff)){
			displayStatus(OnOff);
			// Serial.printf("Working = %d\n",Working);
			refreshScreen();
			vTaskDelay( pdMS_TO_TICKS(100));
		} else {
			// Serial.print(".");
			refreshScreen();
			vTaskDelay( pdMS_TO_TICKS(100));
		}
		if (DeepSleepNow){
			vTaskDelay( pdMS_TO_TICKS(3000));
			enableDeepSleep();
		}
	}
}
void setup_task() {
	xTaskCreatePinnedToCore(
		manageScreen,
		"xTask acces to screen",	 // Name of the task (for debugging)
		20000,             // Stack size (bytes)
		NULL,            // Parameter to pass
		6,               // Task priority
		NULL,             // Task handle
		1
	);
	xTaskCreatePinnedToCore(
		getSensorData,
		"Read sensor data and put on server.",   // Name of the task (for debugging)
		20000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		5,               // Task priority
		NULL,             // Task handle
		1
	);
	// xTaskCreate(
	// 	waitHttpRequest,
	// 	"Attend les requetes API",    // Name of the task (for debugging)
	// 	10000,             // Stack size (bytes)
	// 	NULL,            // Parameter to pass
	// 	1,               // Task priority
	// 	NULL             // Task handle
	// );
	xTaskCreatePinnedToCore(
		isWorking,
		"Toogle LD1 on working",    // Name of the task (for debugging)
		1000,             // Stack size (bytes)
		NULL,            // Parameter to pass
		5,               // Task priority
		NULL,             // Task handle
		0
	);
	Serial.println("setup_task()");
}

void setup() {
	Serial.begin(115200);
	Working = true;
	DeepSleepNow = false;
	mutex = xSemaphoreCreateMutex();
	initSPIFFS();
	std::string SettingsJson = readFile(SPIFFS, "/Settings.json");
	if(SettingsJson.length() > 2){
		CurrentProbe.Settings = SETTINGS::fromJson(SettingsJson.c_str());
	} else {
		init_WiFiManager();
	}

	// CurrentProbe.Settings.SSID = "MartinLopez";
	// CurrentProbe.Settings.PWD = "0651818124";
	// CurrentProbe.Settings.SrvDataBase2Post = "http://rpimaster/probe";
	// CurrentProbe.Settings.Hostname = "ESP32NodeSensor_1";
	// CurrentProbe.Settings.EnableDeepSleep = true;
	// CurrentProbe.Settings.DisplayDuringDeepSleep = true;

	pinMode(BATTERY_PIN, INPUT);
	pinMode(POWER_PIN, INPUT);
	// pinMode(LD1_PIN, OUTPUT);
	// configure LED PWM functionalitites
	ledcSetup(LD1CHANNEL, FREQ, RESOLUTION);
	// attach the channel to the GPIO to be controlled
	ledcAttachPin(LD1_PIN, LD1CHANNEL);

	// Sensor setup
	// if (!bme.begin(0x76)) {
	//   Serial.println("Problem connecting to BME280");
	// }
	print_wakeup_reason();
	esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
	Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

	init_Screen();
	setup_task();
	init_WiFi();
	setup_routing();

	Serial.println("setup()");
	Working = false;
	Serial.flush(); 
}

void loop() {
	reconnect_WiFi();
	server.handleClient();
	delay(5000);
	// Serial.println(CurrentProbe.toJson().c_str());
}


