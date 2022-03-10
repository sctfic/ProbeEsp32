#include <Arduino.h>
#include <WiFi.h>
#include "SPIFFS.h"
#include "network.h"
#include "global.h"



bool check_WiFi_Available(){
	// Serial.printf("\nWiFi.getMode()\n");
    static wifi_mode_t mode = WiFi.getMode();
    CurrentProbe.Network.Hostname       = WiFi.getHostname();
    CurrentProbe.Network.MAC            = WiFi.macAddress().c_str();
    CurrentProbe.Network.Strength       = MEASURE(WiFi.RSSI(), "dBm");
    if (mode == WIFI_MODE_STA){
		// Serial.printf("WIFI_MODE_STA\n");
        CurrentProbe.Network.IP             = WiFi.localIP().toString().c_str();
        CurrentProbe.Network.Gateway        = WiFi.gatewayIP().toString().c_str();
        CurrentProbe.Network.CIDR           = std::to_string(WiFi.subnetCIDR()).c_str() ;
        CurrentProbe.Network.SSID           = WiFi.SSID().c_str();
        CurrentProbe.Network.DNS            = WiFi.dnsIP().toString().c_str();
	    WIFI_CONNECTED = !(WiFi.status() != WL_CONNECTED || (CurrentProbe.Network.IP == "0.0.0.0"));
		return WIFI_CONNECTED;
    } else if (mode == WIFI_MODE_AP) {
		// Serial.printf("WIFI_MODE_AP\n");
        CurrentProbe.Network.IP             = WiFi.softAPIP().toString().c_str();
        CurrentProbe.Network.SSID           = WiFi.softAPSSID().c_str();
        CurrentProbe.Network.CIDR           = std::to_string(WiFi.softAPSubnetCIDR()).c_str() ;
        CurrentProbe.Network.Gateway        = "";
        CurrentProbe.Network.DNS            = WiFi.dnsIP().toString().c_str();
        // CurrentProbe.Network.Strength       = MEASURE(-120, "dBm");
	    WIFI_CONNECTED = false;
		return (CurrentProbe.Network.IP != "0.0.0.0");
    }
}
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


	if (!CurrentProbe.Settings.DHCP && CurrentProbe.Settings.IP != "" && CurrentProbe.Settings.Mask != ""){
		// il y a une conf IPFixe
		IP.fromString(CurrentProbe.Settings.IP.c_str());
			Serial.printf("Set IPFixe: %s",CurrentProbe.Settings.IP.c_str());
		Mask.fromString(CurrentProbe.Settings.Mask.c_str());
		Gateway.fromString(CurrentProbe.Settings.Gateway.c_str());
		DNS1.fromString(CurrentProbe.Settings.DNS1.c_str());
		DNS2.fromString(CurrentProbe.Settings.DNS2.c_str());
		if (!WiFi.config(IP, Gateway, Mask, DNS1, DNS2)){
			Serial.println("STA Failed to configure as IPFixe");
			WiFi.disconnect();
		}
	}
	
	if (CurrentProbe.Settings.Hostname != ""){
		WiFi.setHostname(CurrentProbe.Settings.Hostname.c_str());
			Serial.printf("Set Hostname: %s",WiFi.getHostname());
	}
	check_WiFi_Available();
	// Activation Connexion wifi
	Serial.printf("Wifi.bigin( %s / %s )\n",CurrentProbe.Settings.SSID.c_str(), CurrentProbe.Settings.PWD.c_str());
  	WiFi.begin(CurrentProbe.Settings.SSID.c_str(), CurrentProbe.Settings.PWD.c_str());

	while (!check_WiFi_Available()) {
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

	return true;
}
void initAccessPoint(){
	Serial.println("Setting as AP (Access Point)");
	WiFi.softAP("ESP32-Settings", NULL);
    while (!check_WiFi_Available()) {
		// Serial.println((int)WiFi.localIP());
		Serial.print("*");
		// Serial.println(WiFi.status() != WL_CONNECTED);
		// Serial.println(CurrentProbe.Network.IP == "0.0.0.0");
		// Serial.println(CurrentProbe.Network.SSID.c_str());
        // Serial.println(CurrentProbe.Network.Hostname.c_str());
		// Serial.println(CurrentProbe.Network.IP.c_str());
		// Serial.println(CurrentProbe.Network.CIDR.c_str());
		// Serial.println(CurrentProbe.Network.Gateway.c_str());
		// Serial.println(CurrentProbe.Network.DNS.c_str());
		// Serial.println(CurrentProbe.Network.Strength.toString().c_str());
		// Serial.println(CurrentProbe.Network.MAC.c_str());
		delay(500);
    }
}