#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>


#include "SPIFFS.h"
#include "global.h"
#include "fileSystem.h"
#include "Web.h"

// Create AsyncWebServer object on port 80
	AsyncWebServer server(80);
void setup_RoutingExtend(){

}
void setup_Routing(){

	// Web Server Root URL
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		Serial.println(request->url());
		request->send(SPIFFS, "/index.html", "text/html");
	});
	// display JSON of data
	server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
		Serial.println(request->url());
		request->send(200, "application/json", CurrentProbe.toJson().c_str());
	});
	// remove conf
	server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
		Serial.println(request->url());
		SPIFFS.remove(SettingsPath);
		request->send(200, "application/json", "{\"reset\":\"\"}");
		ESP.restart();
	});
	// Scan available network
	server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
		// WiFi.scanNetworks will return the number of networks found
		int n = WiFi.scanNetworks();
		String scan = "";
		Serial.println(request->url());
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
		Serial.println(request->url());
		Serial.println(paramsNr);
		Serial.println(request->arg("EnableDeepSleep"));
		Serial.println(request->args());
		if (request->arg("DisplayDuringDeepSleep") == "on") {
			CurrentProbe.Settings.DisplayDuringDeepSleep = true;
		} else {}
		if (request->arg("EnableDeepSleep") == "on") {
			CurrentProbe.Settings.EnableDeepSleep = true;
		} else {}
		CurrentProbe.Settings.Hostname 			= request->arg("Hostname").c_str();
		CurrentProbe.Settings.SSID 				= request->arg("SSID").c_str();
		CurrentProbe.Settings.PWD 				= request->arg("PWD").c_str();
		CurrentProbe.Settings.DHCP 				= request->arg("DHCP").c_str();
		CurrentProbe.Settings.IP 				= request->arg("IP").c_str();
		CurrentProbe.Settings.Mask 				= request->arg("Mask").c_str();
		CurrentProbe.Settings.Gateway 			= request->arg("Gateway").c_str();
		CurrentProbe.Settings.DNS1 				= request->arg("DNS1").c_str();
		CurrentProbe.Settings.DNS2 				= request->arg("DNS2").c_str();
		CurrentProbe.Settings.SrvDataBase2Post	= request->arg("SrvDataBase2Post").c_str();

		for(int i=0;i<paramsNr;i++){
			AsyncWebParameter* p = request->getParam(i);
			Serial.print(p->name());
			Serial.print(" = ");
			Serial.println(p->value());
		}
		const char * data = CurrentProbe.Settings.toJson().c_str();
		request->send(200, "application/json", data);
		// request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
		writeFile(SPIFFS, SettingsPath, data);
		delay(100);
		ESP.restart();
	});
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.begin();
	Serial.println("Web Route applyed ! ");
}

