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
extern bool Transfert;
extern bool OnOff;
extern bool WIFI_CONNECTED;
extern bool DataReady;
extern int IgnoreDeepSleep;
extern char charging;


extern const char * SettingsPath;

struct I2CBUS {
// define the wiring settings for I2C interface connection
	int SCL;// 4
	int SDA;// 5 // 21 // SCK
	int RST;// 15

	bool OLED;
	bool BMP280;
	bool BME280;
	bool SCD40;
	bool VEML7700;
};

const struct PIN {
	int BATTERY; // 35
	int POWER; // 32
	int LD1; // 16
	int UV; // 21
} Pin = {35,32,16,21};

class SENSOR_DEF{
    public:
		inline SENSOR_DEF() {}
		// CurrentProbe.Energy.Battery.Voltage.Def = SENSOR_DEF(4.2,"V");
		// inline SENSOR_DEF(double value, std::string unit) : Value(round(value*100)/100), Unit(unit) {}
		inline SENSOR_DEF(double coef,double offset,std::string unit) : Offset(offset),Coef(coef),Unit(unit) {}

		double Offset=0;
		double Coef=1;
		std::string Unit;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(SENSOR_DEF, Offset, Coef, Unit)
		//"{...}"" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// SENSOR_DEF maMesure = SENSOR_DEF::fromJson("{...}");
		inline static SENSOR_DEF fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<SENSOR_DEF>();
		};
	};
class MEASURE {
	public:
		// inline MEASURE() {}
		// CurrentProbe.Energy.Battery.Voltage = MEASURE(4.2,"V");
		// inline MEASURE(double value, std::string unit) : Value(round(value*100)/100), Unit(unit) {}

		double Raw;
		SENSOR_DEF Def;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(MEASURE, Raw, Def)
		//"{...}"" = maMesure.toJson();
		inline void Set(double value) {Raw=round(value*100)/100;}
		inline double Value() {
			// Valeur corrigee et arrondi a 2 decimals
			return (round((this->Def.Coef * this->Raw + this->Def.Offset) * 100) / 100) ;
		}
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// MEASURE maMesure = MEASURE::fromJson("{...}");
		inline static MEASURE fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<MEASURE>();
		};
		inline std::string toString(){
			std::string s = std::to_string(this->Value());
			return s.substr(0, s.find_first_of(".")+2) + this->Def.Unit;
			// char s[32];
			// snprintf(s,32,"%.1f %s",Value,Unit);
			// sprintf(s,"%.1f %s",Value,Unit);
			// return s;
		};
	};
class PROBE {
	public:
		MEASURE Temperature;
		MEASURE Pressure;
		MEASURE Humidity;
		// MEASURE DewPoint;
		// MEASURE BoilingPoint;
		MEASURE CO2;
		MEASURE LUX;
		MEASURE UV;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(PROBE, Temperature, Pressure, Humidity, CO2, LUX, UV)
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
class WIFI {
    public:
    	std::string SSID; //'MartinLopez',
    	std::string PWD; //'Wifi Key',
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(WIFI, SSID, PWD)
	};
class NETWORK {
    public:
    	std::string Hostname; //'ESP32NodeSensor_1',
    	WIFI Wifi;
    	std::string MAC; //'C8:C9:A3:D2:F3:54',
    	bool DHCP = true;
    	std::string IP; //'192.168.1.16',
    	std::string CIDR; //'25',
    	std::string Mask; //'255.255.255.128',
    	std::string Gateway; //'192.168.1.1',
    	std::string DNS1; //'192.168.1.1',
    	std::string DNS2; //'192.168.1.1',
    	MEASURE Strength; // { value: -67, unit: 'dBm' }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(NETWORK, Hostname, Wifi, MAC, DHCP, IP, CIDR, Mask, Gateway, DNS1, DNS2, Strength)
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
class GPS{
    public:
		double longitude;
		double latitude;
		int altitude;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(GPS, longitude, latitude, altitude)
		//"{...}" = maMesure.toJson();
		inline std::string toJson(){
			nlohmann::json json = *this;
			return json.dump();
		};
		// GPS maMesure = GPS::fromJson("{...}");
		inline static GPS fromJson(const std::string& jsonString){
			return nlohmann::json::parse(jsonString, NULL, false, true).get<GPS>();
		};
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
		int MeasurementInterval;
		std::string SrvDataBase2Post;
		std::string area;
		std::string location;
		std::string room;
		std::string type;
		NETWORK Lan;
		WIFI OtherWifi[8];
		GPS gps;
		PROBE Probe;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(SETTINGS, EnableDeepSleep,DisplayDuringDeepSleep,MeasurementInterval,SrvDataBase2Post,gps,Lan,OtherWifi,Probe)
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
extern I2CBUS i2cbus;
extern const PIN Pin;
int str2int(std::string s);
