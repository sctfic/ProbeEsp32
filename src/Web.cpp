#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <HTTPClient.h>


#include "SPIFFS.h"
#include "global.h"
#include "sensors.h"
#include "fileSystem.h"
#include "Web.h"

// Create AsyncWebServer object on port 80
	AsyncWebServer server(80);
	WiFiClient Wclient;  // or WiFiClientSecure for HTTPS
	HTTPClient httpClient;


void setup_Routing(){
	// Web Server Root URL
	// server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
	// 	IgnoreDeepSleep = 600;
	// 	Transfert = true;
	// 	Serial.println(request->url());
	// 	request->send(SPIFFS, "/index.html", "text/html");
	// 	delay(1000);
	// 	Transfert = false;
	// });

	// display JSON of data
	server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
		IgnoreDeepSleep = 60;
		Transfert = true;
		Serial.println(request->url());
		request->send(200, "application/json", CurrentProbe.toJson().c_str());
		Transfert = false;
	});
	// remove conf
	server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
		IgnoreDeepSleep = 10;
		Working = true;
		Serial.println(request->url());
		SPIFFS.remove(SettingsPath);
		request->send(200, "application/json", "{\"reset\":\"\"}");
		ESP.restart();
		// Working = false;
	});
	// Scan available network
	server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
		IgnoreDeepSleep = 60;
		Working = true;
		Transfert = true;
		// WiFi.scanNetworks will return the number of networks found
		Serial.println(request->url());
		int n = WiFi.scanNetworks();
		String scan = "";
		for (int i = 0; i < n; ++i) {
			if (i) {
				scan = scan + ",";
			}		
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
		Working = false;
		Transfert = false;
	});
	// Save and apply new conf
	server.on("/Settings/apply", HTTP_GET, [](AsyncWebServerRequest *request){
		IgnoreDeepSleep = 60;
		Transfert = true;
		// int paramsNr = request->params();
		Serial.println(request->url());
		// Serial.println(paramsNr);
		// Serial.println(request->arg("EnableDeepSleep"));
		// Serial.println(request->args());
		if (request->arg("DisplayDuringDeepSleep") == "on") {
			CurrentProbe.Settings.DisplayDuringDeepSleep = true;
		} else {
			CurrentProbe.Settings.DisplayDuringDeepSleep = false;
		}
		if (request->arg("EnableDeepSleep") == "on") {
			CurrentProbe.Settings.EnableDeepSleep = true;
		} else {
			CurrentProbe.Settings.EnableDeepSleep = false;
		}
		if (request->arg("DHCP") == "on") {
			CurrentProbe.Settings.Lan.DHCP 			= true;
		} else {
			CurrentProbe.Settings.Lan.DHCP 			= false;
		}

		CurrentProbe.Settings.Lan.Hostname 			= request->arg("Hostname").c_str();
		CurrentProbe.Settings.Lan.Wifi.SSID 		= request->arg("SSID").c_str();
		CurrentProbe.Settings.Lan.Wifi.PWD 			= request->arg("PWD").c_str();
		CurrentProbe.Settings.Lan.IP 				= request->arg("IP").c_str();
		CurrentProbe.Settings.Lan.Mask 				= request->arg("Mask").c_str();
		CurrentProbe.Settings.Lan.Gateway 			= request->arg("Gateway").c_str();
		CurrentProbe.Settings.Lan.DNS1 				= request->arg("DNS1").c_str();
		CurrentProbe.Settings.Lan.DNS2 				= request->arg("DNS2").c_str();
		CurrentProbe.Settings.SrvDataBase2Post		= request->arg("SrvDataBase2Post").c_str();
		CurrentProbe.Settings.MeasurementInterval	= str2int(request->arg("MeasurementInterval").c_str());

		CurrentProbe.Settings.area 			= request->arg("area").c_str();
		CurrentProbe.Settings.location 		= request->arg("location").c_str();
		CurrentProbe.Settings.room 			= request->arg("room").c_str();
		CurrentProbe.Settings.type 			= request->arg("type").c_str();

		const char * data = CurrentProbe.Settings.toJson().c_str();
		writeFile(SPIFFS, SettingsPath, data);
		Serial.println(readFile(SPIFFS, SettingsPath).c_str());

		Transfert = false;
		if (!CurrentProbe.Settings.Lan.DHCP){
			request->redirect(("http://" + CurrentProbe.Settings.Lan.IP + "/Sensors").c_str());
		} else {
			request->redirect("/Sensors");
			// request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: ");
		}
		delay(3000);
		ESP.restart();
	});
	// Save and apply new conf
	server.on("/Sensors/apply", HTTP_POST, [](AsyncWebServerRequest *request){
		IgnoreDeepSleep = 60;
		Transfert = true;
		Serial.println(request->url());

		CurrentProbe.Settings.gps.longitude = std::stod(request->arg("Longitude").c_str());
		CurrentProbe.Settings.gps.latitude = std::stod(request->arg("Latitude").c_str());
		CurrentProbe.Settings.gps.altitude = std::stoi(request->arg("Altitude").c_str());
		Serial.println(CurrentProbe.Settings.gps.toJson().c_str());

		CurrentProbe.Settings.Probe.Temperature.Def	= SENSOR_DEF(
			std::stod(request->arg("TemperatureCoef").c_str()),
			std::stod(request->arg("TemperatureOffset").c_str()),
			request->arg("TemperatureUnit").c_str()
		);
		CurrentProbe.Settings.Probe.Pressure.Def	= SENSOR_DEF(
			std::stod(request->arg("PressureCoef").c_str()),
			std::stod(request->arg("PressureOffset").c_str()),
			request->arg("PressureUnit").c_str()
		);
		CurrentProbe.Settings.Probe.Humidity.Def	= SENSOR_DEF(
			std::stod(request->arg("HumidityCoef").c_str()),
			std::stod(request->arg("HumidityOffset").c_str()),
			request->arg("HumidityUnit").c_str()
		);
		CurrentProbe.Settings.Probe.CO2.Def	= SENSOR_DEF(
			std::stod(request->arg("CO2Coef").c_str()),
			std::stod(request->arg("CO2Offset").c_str()),
			request->arg("CO2Unit").c_str()
		);
		CurrentProbe.Settings.Probe.LUX.Def	= SENSOR_DEF(
			std::stod(request->arg("LUXCoef").c_str()),
			std::stod(request->arg("LUXOffset").c_str()),
			request->arg("LUXUnit").c_str()
		);
		CurrentProbe.Settings.Probe.UV.Def	= SENSOR_DEF(
			std::stod(request->arg("UVCoef").c_str()),
			std::stod(request->arg("UVOffset").c_str()),
			request->arg("UVUnit").c_str()
		);

		// Serial.println(CurrentProbe.Settings.Probe.toJson().c_str());
		CurrentProbe.Probe	= CurrentProbe.Settings.Probe;

		const char * data = CurrentProbe.Settings.toJson().c_str();
		// request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
		writeFile(SPIFFS, SettingsPath, data);
		Serial.println(readFile(SPIFFS, SettingsPath).c_str());
		request->redirect("/Sensors");

		// request->send(200, "application/json", data);
		// request->send(SPIFFS, "/Sensors.html", "text/html");
		Transfert = false;
	});
    // Web Server Sensor URL
	server.on("/Sensors", HTTP_GET, [](AsyncWebServerRequest *request){
		IgnoreDeepSleep = 600;
		Transfert = true;
		Serial.println(request->url());
		request->send(SPIFFS, "/Sensors.html", "text/html");
		Transfert = false;
	});
    // Web Server Sensor URL
	server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request){
		IgnoreDeepSleep = 600;	
		Transfert = true;
		Serial.println(request->url());
		// const char *type = request->arg("type").c_str();
		if(request->arg("CO2").c_str()){
			// Temperature
			// Pressure
			// Humidity
			// CO2
			// LUX
			// UV
			calibrateSCD40(
				std::stod(request->arg("Temperature").c_str()) - CurrentProbe.Probe.Temperature.Raw,
				std::stod(request->arg("Pressure").c_str()),
				std::stoi(request->arg("CO2").c_str())
			);

		}
		// case 'Temperature':
		// case 'Pressure':
		// case 'Humidity':
		// case 'LUX':
		// case 'UV':
		request->send(200, "application/json", CurrentProbe.Probe.toJson().c_str());

		Serial.println(request->arg("type").c_str());
		Transfert = false;
	});
    server.serveStatic("/", SPIFFS, "/")
    	.setDefaultFile("index.html");
    
    server.begin();
	Serial.println("> Web Route applyed ! ");
}


bool uploadData(const char * data, const char * url) {
	// Block until we are able to connect to the WiFi access point
	if (WIFI_CONNECTED) {
		Transfert = true;
		// Send request
		httpClient.begin(Wclient, url);
		httpClient.addHeader("Content-Type", "application/json");
		uint httpResponseCode = httpClient.POST(data);

		// Read response
		// Serial.println(httpResponseCode);
		// Serial.println(httpClient.getString());

		// Disconnect
		httpClient.end();
		Serial.printf("+---> Upload(JSON) to %s > %i\n", url,httpResponseCode);
		Transfert = false;
		return httpResponseCode == 201 ? true : false;
	}
	return false;
}
