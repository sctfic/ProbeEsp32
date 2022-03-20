#include <Arduino.h>
#include <WiFi.h>
#include "SPIFFS.h"
#include "network.h"
#include "global.h"


bool check_WiFi_Available(){
	// Serial.printf("\nWiFi.getMode()\n");
	// Serial.print(CurrentProbe.Settings.Lan.toJson().c_str());

    static wifi_mode_t mode = WiFi.getMode();
    CurrentProbe.Network.Strength.Set(WiFi.RSSI());
    CurrentProbe.Network.Hostname = WiFi.getHostname();
    CurrentProbe.Network.MAC      = WiFi.macAddress().c_str();
    if (mode == WIFI_MODE_STA){
		// Serial.printf("WIFI_MODE_STA\n");
        CurrentProbe.Network.IP             = WiFi.localIP().toString().c_str();
        CurrentProbe.Network.Gateway        = WiFi.gatewayIP().toString().c_str();
        CurrentProbe.Network.CIDR           = std::to_string(WiFi.subnetCIDR()).c_str() ;
        CurrentProbe.Network.SSID           = WiFi.SSID().c_str();
        CurrentProbe.Network.DNS1            = WiFi.dnsIP().toString().c_str();
	    WIFI_CONNECTED = !(WiFi.status() != WL_CONNECTED || (CurrentProbe.Network.IP == "0.0.0.0"));
		return WIFI_CONNECTED;
    } else if (mode == WIFI_MODE_AP) {
		// Serial.printf("WIFI_MODE_AP\n");
        CurrentProbe.Network.IP             = WiFi.softAPIP().toString().c_str();
        CurrentProbe.Network.SSID           = WiFi.softAPSSID().c_str();
        CurrentProbe.Network.CIDR           = std::to_string(WiFi.softAPSubnetCIDR()).c_str() ;
        CurrentProbe.Network.Gateway        = "";
        CurrentProbe.Network.DNS1            = WiFi.dnsIP().toString().c_str();
	    WIFI_CONNECTED = false;
		return (CurrentProbe.Network.IP != "0.0.0.0");
    }
}
bool initWiFi() {
	if(CurrentProbe.Settings.Lan.SSID==""){
		Serial.println("Undefined SSID.");
		return false;
	}
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();

	int wait = 20;
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

	if (!CurrentProbe.Settings.Lan.DHCP && CurrentProbe.Settings.Lan.IP != "" && CurrentProbe.Settings.Lan.Mask != ""){
		// il y a une conf IPFixe
		IP.fromString(CurrentProbe.Settings.Lan.IP.c_str());
			// Serial.printf("Set IPFixe: %s",CurrentProbe.Settings.Lan.IP.c_str());
		Mask.fromString(CurrentProbe.Settings.Lan.Mask.c_str());
		Gateway.fromString(CurrentProbe.Settings.Lan.Gateway.c_str());
		DNS1.fromString(CurrentProbe.Settings.Lan.DNS1.c_str());
		DNS2.fromString(CurrentProbe.Settings.Lan.DNS2.c_str());
		if (!WiFi.config(IP, Gateway, Mask, DNS1, DNS2)){
			Serial.println("STA Failed to configure as IPFixe");
			WiFi.disconnect();
		}
	}
	
	if (CurrentProbe.Settings.Lan.Hostname != ""){
		WiFi.setHostname(CurrentProbe.Settings.Lan.Hostname.c_str());
		// Serial.printf("Set Hostname: %s",WiFi.getHostname());
	}
	// check_WiFi_Available();
	// Activation Connexion wifi
	// Serial.printf("Wifi.bigin( %s / %s )\n",CurrentProbe.Settings.Lan.SSID.c_str(), CurrentProbe.Settings.Lan.PWD.c_str());
  	WiFi.begin(CurrentProbe.Settings.Lan.SSID.c_str(), CurrentProbe.Settings.Lan.PWD.c_str());

	while (!check_WiFi_Available()) {
		// Serial.println((int)WiFi.localIP());
		// Serial.print(".");
		delay(200);
		if ( i >= wait ) { // 20 * 200ms = 4sec
			i=0;
			// Serial.println("");

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
	// Serial.println("");
	return true;
}
void initAccessPoint(){
	Serial.println("Setting as AP (Access Point)");
	WiFi.softAP("ESP32-Settings", NULL);

	Serial.print(CurrentProbe.Settings.Lan.toJson().c_str());

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