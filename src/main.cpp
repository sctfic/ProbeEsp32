/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include "SPIFFS.h"
#include "global.h"
#include "display.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

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
bool check_WiFi_connected (){
	WIFI_CONNECTED = !(WiFi.status() != WL_CONNECTED || ((int)WiFi.localIP() == 1073550736));
	return WIFI_CONNECTED;
}
// Initialize WiFi
bool initWiFi() {
	if(CurrentProbe.Settings.SSID==""){
		Serial.println("Undefined SSID.");
		return false;
	}
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();

	int wait = 100;
	int i = 0;
	int retry = 0;
	//IPAddress IP(192, 168, 1, 200); // hardcoded
	IPAddress IP;
	// Set your Gateway IP address
	IPAddress Gateway;
	//IPAddress Gateway(192, 168, 1, 1); //hardcoded
	IPAddress Mask(255, 255, 255, 0);
	IPAddress DNS1;
	IPAddress DNS2;


	if (CurrentProbe.Settings.IP != "" && CurrentProbe.Settings.Mask != ""){
		IP.fromString(CurrentProbe.Settings.IP.c_str());
		Mask.fromString(CurrentProbe.Settings.Mask.c_str());
		Gateway.fromString(CurrentProbe.Settings.Gateway.c_str());
		DNS1.fromString(CurrentProbe.Settings.DNS1.c_str());
		DNS2.fromString(CurrentProbe.Settings.DNS2.c_str());
		if (!WiFi.config(IP, Gateway, Mask, DNS1, DNS2)){
			Serial.println("STA Failed to configure as IPFixe");
			WiFi.disconnect();
		}
	}
	Serial.println("Set Hostname");
	
	if (CurrentProbe.Settings.Hostname != ""){
		WiFi.setHostname(CurrentProbe.Settings.Hostname.c_str());
	}

  	WiFi.begin(CurrentProbe.Settings.SSID.c_str(), CurrentProbe.Settings.PWD.c_str());

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
	Serial.println("");

	CurrentProbe.Network.Hostname       = WiFi.getHostname();
	CurrentProbe.Network.IP             = WiFi.localIP().toString().c_str();
	CurrentProbe.Network.Gateway        = WiFi.gatewayIP().toString().c_str();
	CurrentProbe.Network.CIDR           = std::to_string(WiFi.subnetCIDR()).c_str() ;
	CurrentProbe.Network.MAC            = WiFi.macAddress().c_str();
	CurrentProbe.Network.SSID           = WiFi.SSID().c_str();
	CurrentProbe.Network.DNS            = WiFi.dnsIP().toString().c_str();
	CurrentProbe.Network.Strength       = MEASURE(WiFi.RSSI(), "dBm");

	Serial.println(WiFi.getHostname());
	Serial.println(WiFi.SSID().c_str());
	Serial.println(WiFi.localIP().toString().c_str());
	Serial.println(WiFi.gatewayIP().toString().c_str());
	Serial.println(WiFi.subnetCIDR());
	Serial.println(WiFi.macAddress().c_str());
	Serial.println(WiFi.dnsIP().toString().c_str());
	// Print local IP address and start web server
	Serial.println("WiFi connected.");
	return true;
}

void setup_Routing(){

	// Web Server Root URL
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/index.html", "text/html");
	});
	// display JSON of data
	server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "application/json", CurrentProbe.toJson().c_str());
	});
	
	// display JSON of data
	server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
		// WiFi.scanNetworks will return the number of networks found
		int n = WiFi.scanNetworks();
		String scan = "";
		for (int i = 0; i < n; ++i) {
			if (i>0){scan = scan + ",";}
			scan = scan + "\"" +WiFi.SSID(i)+ "\"";
			// Print SSID and RSSI for each network found
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" " : "*");
		}
		request->send(200, "application/json", ("["+scan+"]"));
	});
	// Save and apply new conf
	server.on("/applySettings", HTTP_GET, [](AsyncWebServerRequest *request){
		int paramsNr = request->params();
		Serial.println(paramsNr);
		Serial.println(request->arg("EnableDeepSleep"));
		Serial.println(request->args());
		if (request->arg("DisplayDuringDeepSleep") == "on") {
			CurrentProbe.Settings.DisplayDuringDeepSleep = true;
		} else {}
		if (request->arg("EnableDeepSleep") == "on") {
			CurrentProbe.Settings.EnableDeepSleep = true;
		} else {}
		CurrentProbe.Settings.Hostname = request->arg("Hostname").c_str();
		CurrentProbe.Settings.SSID = request->arg("SSID").c_str();
		CurrentProbe.Settings.PWD = request->arg("PWD").c_str();
		CurrentProbe.Settings.IP = request->arg("IP").c_str();
		CurrentProbe.Settings.Mask = request->arg("Mask").c_str();
		CurrentProbe.Settings.Gateway = request->arg("Gateway").c_str();
		CurrentProbe.Settings.DNS1 = request->arg("DNS1").c_str();
		CurrentProbe.Settings.DNS2 = request->arg("DNS2").c_str();
		CurrentProbe.Settings.SrvDataBase2Post = request->arg("SrvDataBase2Post").c_str();

		for(int i=0;i<paramsNr;i++){
			AsyncWebParameter* p = request->getParam(i);
			Serial.print(p->name());
			Serial.print(" = ");
			Serial.println(p->value());
		}
		const char * data = CurrentProbe.Settings.toJson().c_str();
		request->send(200, "application/json", data);
		// request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
		writeFile(SPIFFS, "/Settings.json", data);
		delay(100);
		ESP.restart();
	});
    
    server.serveStatic("/", SPIFFS, "/");
	Serial.print("Route loaded ! ");
    
    server.begin();
	Serial.print("Route applyed ! ");

}
void manageScreen(void * parameter){
	init_Screen();
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
			// enableDeepSleep();
		}
	}
}
void setup_task() {
	xTaskCreatePinnedToCore(
		manageScreen,
		"xTask acces to screen",	 // Name of the task (for debugging)
		20000,             // Stack size (bytes)
		NULL,              // Parameter to pass
		6,                 // Task priority
		NULL,              // Task handle
		1                  // force CPU 1
	);
	// xTaskCreatePinnedToCore(
	// 	getSensorData,
	// 	"Read sensor data and put on server.",   // Name of the task (for debugging)
	// 	20000,            // Stack size (bytes)
	// 	NULL,            // Parameter to pass
	// 	5,               // Task priority
	// 	NULL,             // Task handle
	// 	1
	// );
	// xTaskCreatePinnedToCore(
	// 	isWorking,
	// 	"Toogle LD1 on working",    // Name of the task (for debugging)
	// 	1000,             // Stack size (bytes)
	// 	NULL,            // Parameter to pass
	// 	5,               // Task priority
	// 	NULL,             // Task handle
	// 	0
	// );
	Serial.println("setup_task()");
}
void setup() {
	// Serial port for debugging purposes
	Serial.begin(115200);
	mutex = xSemaphoreCreateMutex();

	initSPIFFS();

	std::string SettingsJson = readFile(SPIFFS, "/Settings.json");
	Serial.println(CurrentProbe.Settings.toJson().c_str());
	if(SettingsJson.length() > 2){
		/Serial.println("Loading SettingsJson");
		CurrentProbe.Settings = SETTINGS::fromJson(SettingsJson.c_str());
		Serial.println(CurrentProbe.Settings.toJson().c_str());
	}

	if (!initWiFi()){
		Serial.println("Wrong File : /Settings.json");
		// Connect to Wi-Fi network with SSID and password
		Serial.println("Setting as AP (Access Point)");
		// NULL sets an open Access Point
		WiFi.softAP("ESP32-Settings", NULL);

		IPAddress IP = WiFi.softAPIP();
		Serial.print("AP IP address: ");
		Serial.println(IP); 
  	}
	setup_Routing();
	delay(2000);
	// Serial.print("DISPLAY ! ");
	// init_Screen();
	setup_task();

}

void loop() {

}