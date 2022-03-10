#pragma once

#include <Arduino.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <iomanip>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t mutex;

// #include <WString.h>
#include <string>
// #include "SPIFFS.h"

#undef B1
#include "json.h"

extern bool DeepSleepNow;
extern bool Working;
extern bool OnOff;
extern bool WIFI_CONNECTED;
extern const char * SettingsPath;


class MEASURE {
	public:
		inline MEASURE() {}
		// CurrentProbe.Energy.Battery.Voltage = MEASURE(4.2,"V");
		inline MEASURE(double value, std::string unit) : Value(round(value*100)/100), Unit(unit) {}

		double Value;
		std::string Unit;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(MEASURE, Value, Unit)
		//"{...}"" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// MEASURE maMesure = MEASURE::fromJson("{...}");
		inline static MEASURE fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<MEASURE>();
		};
		inline std::string toString(){
			std::string s = std::to_string(Value);
			return s.substr(0, s.find_first_of(".")+2) + Unit;
		};
	};
class PROBE {
	public:
		MEASURE Temperature;
		MEASURE Pressure;
		MEASURE Humidity;
		MEASURE CO2;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(PROBE, Temperature, Pressure, Humidity, CO2)
		//"{...}"" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// PROBE maMesure = PROBE::fromJson("{...}");
		inline static PROBE fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<PROBE>();
		};
	};
class NETWORK {
    public:
    	std::string Hostname; //'ESP32NodeSensor_1',
    	std::string SSID; //'MartinLopez',
    	// std::string PWD; //'Wifi Key',
    	std::string MAC; //'C8:C9:A3:D2:F3:54',
    	std::string IP; //'192.168.1.16',
    	std::string CIDR; //'25',
    	std::string Gateway; //'192.168.1.1',
    	std::string DNS; //'192.168.1.1',
    	MEASURE Strength; // { value: -67, unit: 'dBm' }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NETWORK, Hostname, SSID, MAC, IP, CIDR, Gateway, DNS, Strength)
		//"{...}"" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// NETWORK maMesure = NETWORK::fromJson("{...}");
		inline static NETWORK fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<NETWORK>();
		};
	};
class POWERSUPPLY {
    public:
		MEASURE Voltage;
		MEASURE Current;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(POWERSUPPLY, Voltage,Current)

	};
class BATTERY{
    public:
		MEASURE Voltage;
		MEASURE Capacity;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(BATTERY, Voltage,Capacity)

	};
class ENERGY{
    public:
		POWERSUPPLY PowerSupply;
		BATTERY Battery;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(ENERGY, PowerSupply,Battery)
		//"{...}"" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// ENERGY maMesure = ENERGY::fromJson("{...}");
		inline static ENERGY fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<ENERGY>();
		};
	};
class SETTINGS{
    public:
		bool EnableDeepSleep;
		bool DisplayDuringDeepSleep;
		std::string SrvDataBase2Post;
    	std::string SSID; //'MartinLopez',
    	std::string PWD; //'Wifi Key',
    	std::string Hostname; //'ESP32NodeSensor_1',
    	std::string IP; //'192.168.1.32',
    	bool DHCP;
    	std::string Mask; //'255.255.255.0',
    	std::string Gateway; //'192.168.1.1',
    	std::string DNS1; //'8.8.8.8',
    	std::string DNS2; //'8.8.4.4',
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(SETTINGS, EnableDeepSleep,DisplayDuringDeepSleep,SrvDataBase2Post,SSID,PWD,Hostname,DHCP,IP,Mask,Gateway,DNS1,DNS2)
		//"{...}" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// SETTINGS maMesure = SETTINGS::fromJson("{...}");
		inline static SETTINGS fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<SETTINGS>();
		};
	};
class ESP32BOARD{
    public:
		NETWORK Network;
		PROBE Probe;
		ENERGY Energy;
		SETTINGS Settings;
		// String toJson();
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(ESP32BOARD, Network,Probe,Energy,Settings)
		//"{...}"" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// ESP32BOARD maMesure = ESP32BOARD::fromJson("{...}");
		inline static ESP32BOARD fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<ESP32BOARD>();
		};
	};

extern ESP32BOARD CurrentProbe;
