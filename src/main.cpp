#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
#include <HTTPClient.h>
// #include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#include "global.h"
#include "display.h"

const char *hostname = "ESP32NodeSensor_1";
const char* SrvPostData = "http://rpimaster/probe";
unsigned long previousMillis = 0;
unsigned long interval = 30000;

// #define DELAY_READ_PROBES 30 // in seconds
#define BATTERY_PIN 35
#define LD1_PIN 16
const int  FREQ = 5000;
const int  LD1CHANNEL = 0;
const int  RESOLUTION = 8;

// JSON data buffer
StaticJsonDocument<500> jsonDocument;
char json[500];

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



void init_WiFi (){
	int wait = 100;
	Working = true;
  // Connect to Wi-Fi network with SSID and PWD
	Serial.print("Connecting to wifi ");
	Serial.print(SSID);
	// https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
	WiFi.mode(WIFI_STA);
	WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
	WiFi.setHostname(hostname);
	WiFi.disconnect();
	WiFi.begin(SSID, PWD);
	while (WiFi.status() != WL_CONNECTED || ((int)WiFi.localIP() == 1073550736)) {
		Serial.print(".");
		delay(200);
		if (wait == 0) {
			wait = 200;
			WiFi.disconnect();
			WiFi.reconnect();
		}
		wait -= 1;
	}
	// Print local IP address and start web server
	Serial.println("WiFi connected.");
	WIFI_CONNECTED = true;
	Working = false;
}
void check_WiFi (bool log = false){
	unsigned long currentMillis = millis();
// if WiFi is down, try reconnecting
	HostName = WiFi.getHostname();
	IP = WiFi.localIP().toString();
	GW = WiFi.gatewayIP().toString();
	CIDR = (String)(WiFi.subnetCIDR());
	MAC = WiFi.macAddress();
	DNS = WiFi.dnsIP().toString();
	attenuation = WiFi.RSSI();
	if ((WiFi.status() != WL_CONNECTED || ((int)WiFi.localIP() == 1073550736)) && (currentMillis - previousMillis >= interval)) {
		WIFI_CONNECTED = false;
		Working = true;
		// IP = "";
		// GW = "";
		// CIDR = "";
		// DNS = "";
		// attenuation = "";

		Serial.print(millis());
		Serial.println("Reconnecting to WiFi...");
		WiFi.disconnect();
		WiFi.reconnect();
		previousMillis = currentMillis;

		HostName = WiFi.getHostname();
		IP = WiFi.localIP().toString();
		GW = WiFi.gatewayIP().toString();
		CIDR = (String)(WiFi.subnetCIDR());
		MAC = WiFi.macAddress();
		DNS = WiFi.dnsIP().toString();
		attenuation = WiFi.RSSI();

		Working = false;
	}

	if (log){
		Serial.printf("Network:");
		Serial.print(" ");Serial.print(HostName);
		Serial.print(" ");Serial.print(IP);
		Serial.print(" ");Serial.print(GW);
		Serial.print(" ");Serial.print(CIDR);
		Serial.print(" ");Serial.print(MAC);
		Serial.print(" ");Serial.print(DNS);
		Serial.printf(" %i dBm\n",attenuation);
	}
}
String getJSON() {
	String json ="{";
		json += "\"Network\": {\"hostname\": \""+String(HostName)+"\",\"SSID\": \""+String(SSID)+"\",\"MAC\": \""+String(MAC)+"\",\"IP\": \""+String(IP)+"\",\"CIDR\": \""+String(CIDR)+"\",\"Gateway\": \""+String(GW)+"\",\"DNS\": \""+String(DNS)+"\",\"Strength\": {\"value\":"+String(attenuation)+",\"unit\": \"dBm\"}},";
		json += "\"Temperature\": {\"value\":"+String(temperature)+",\"unit\": \"°C\"},";
		json += "\"Pressure\": {\"value\":"+String(pressure)+",\"unit\": \"hPa\"},";
		json += "\"Humidity\": {\"value\":"+String(humidity)+",\"unit\": \"%\"},";
		json += "\"CO2\": {\"value\":"+String(CO2)+",\"unit\": \"ppm\"},";
		json += "\"Battery\": {\"Voltage\": {\"value\":"+String(BatVoltage)+",\"unit\": \"V\"},\"Capacity\": {\"value\":"+String(BatCapacity)+",\"unit\": \"%\"}},";
		json += "\"Power\": {\"value\":"+String(PowerVoltage)+",\"unit\": \"V\"}}";
	return json;
}
void ApiJson(){
	Working = true;
	Serial.printf("API Reply JSON %d\n",Working);
	server.send(200, "application/json", getJSON());
	delay(2000);
	Working = false;
}
bool postDataToServer() {
	// Block until we are able to connect to the WiFi access point
	if (WIFI_CONNECTED && DeepSleep == false) {
		WiFiClient Wclient;  // or WiFiClientSecure for HTTPS
		HTTPClient httpClient;

		// Send request
		httpClient.begin(Wclient, SrvPostData);
		Serial.printf("POST(JSON) to %s > ",SrvPostData);
		httpClient.addHeader("Content-Type", "application/json");
		int httpResponseCode = httpClient.POST(getJSON()); // 

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
	if (BatVoltage < 2.76){
		BatCapacity = -1;
	} else if (BatVoltage > 4.2){
		BatCapacity = 101;
	} else {
		BatCapacity = (int)lround(-47391.44 + 68111.053*BatVoltage -38668.575*pow(BatVoltage,2) + 10828.2011*pow(BatVoltage,3) + -1494.5436*pow(BatVoltage,4) + 81.37904*pow(BatVoltage,5));
	}

	// const batteryCapacity remainingCapacity[] = {
	// 4.20,	100.00,
	// 4.18,	99.25,
	// 4.16,	98.58,
	// 4.14,	97.81,
	// 4.12,	96.93,
	// 4.10,	95.94,
	// 4.08,	94.85,
	// 4.06,	93.65,
	// 4.04,	92.34,
	// 4.02,	90.93,
	// 4.00,	89.42,
	// 3.98,	87.81,
	// 3.96,	86.10,
	// 3.94,	84.30,
	// 3.92,	82.42,
	// 3.90,	80.45,
	// 3.88,	78.40,
	// 3.86,	76.28,
	// 3.84,	74.09,
	// 3.82,	71.84,
	// 3.80,	69.54,
	// 3.78,	67.19,
	// 3.76,	64.80,
	// 3.74,	62.38,
	// 3.72,	59.92,
	// 3.70,	57.45,
	// 3.68,	54.96,
	// 3.66,	52.47,
	// 3.64,	49.98,
	// 3.62,	47.50,
	// 3.60,	45.03,
	// 3.58,	42.59,
	// 3.56,	40.17,
	// 3.54,	37.79,
	// 3.52,	35.46,
	// 3.50,	33.17,
	// 3.48,	30.94,
	// 3.46,	28.78,
	// 3.44,	26.68,
	// 3.42,	24.65,
	// 3.40,	22.70,
	// 3.38,	20.83,
	// 3.36,	19.05,
	// 3.34,	17.36,
	// 3.32,	15.76,
	// 3.30,	14.26,
	// 3.28,	12.85,
	// 3.26,	11.55,
	// 3.24,	10.34,
	// 3.22,	9.24,
	// 3.20,	8.23,
	// 3.18,	7.33,
	// 3.16,	6.52,
	// 3.14,	5.81,
	// 3.12,	5.18,
	// 3.10,	4.65,
	// 3.08,	4.19,
	// 3.06,	3.81,
	// 3.04,	3.50,
	// 3.02,	3.25,
	//.3.00,	3.05,
	// 2.98,	2.89,
	// 2.96,	2.76,
	// 2.94,	2.66,
	// 2.92,	2.55,
	// 2.90,	2.44,
	// 2.88,	2.30,
	// 2.86,	2.13,
	// 2.84,	1.89,
	// 2.82,	1.58,
	// 2.80,	1.16,
	// 2.78,	0.63,
	// 2.76,	0.00,
	// };
	// const int step = sizeof(remainingCapacity) / sizeof(struct batteryCapacity);
	// int i = 0;
	// while (BatVoltage < remainingCapacity[i].voltage && i < step) {
	// 	i++;
	// }
	// BatCapacity = remainingCapacity[i].capacity;
	// Serial.printf("Pourcentage Battery: %i%%\n",BatCapacity);
}
void getBatteryVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(BATTERY_PIN) / 4096 * 3.3);
	//   Serial.printf("TensionPin(35): %f\n",Vread);
	BatVoltage =  Vread*(20120+9670)/20120;
	// Serial.printf("Tension Battery: %fV\n",BatVoltage);
}
void getSensorData(void * parameter) {
	for (;;) {
		if (WIFI_CONNECTED) {
			Working = true;
			temperature = 8.3; // bme.readTemperature();
			humidity = 56.2; // bme.readHumidity();
			pressure = 1003.5; // bme.readPressure() / 100;
			CO2 = 415.5;
			getBatteryVoltage();
			getBatteryCapacity();
			xSemaphoreTake( mutex, portMAX_DELAY );
				// read CO2 SPi or I2C
			xSemaphoreGive( mutex );
			DeepSleep = postDataToServer();
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
  deserializeJson(jsonDocument, body);
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
	xSemaphoreTake( mutex, portMAX_DELAY );
		displayDeepSleep();
	xSemaphoreGive( mutex );
	esp_deep_sleep_start();
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
		if (DeepSleep){
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
	DeepSleep = false;
	mutex = xSemaphoreCreateMutex();

	pinMode(BATTERY_PIN, INPUT);
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
	// Serial.println("loop(start)");
	check_WiFi();
	server.handleClient();
	// manageScreen(NULL);
	delay(200);
}


