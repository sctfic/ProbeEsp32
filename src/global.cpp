// #include <FreeRTOS.h>
// #include <semphr.h>
#include "global.h"
#include <iostream>
#include <string>

// Création d'un mutex
	SemaphoreHandle_t mutex = NULL;

// env variable

bool DeepSleepNow = false;
bool Working = true;
bool OnOff = true;
bool WIFI_CONNECTED = false;
bool BMP280_CONNECTED = false;
bool BME280_CONNECTED = false;
bool CO2_CONNECTED = false;

const char * SettingsPath = "/Settings.json";

ESP32BOARD CurrentProbe;

int str2int(std::string s){
    int i = 0;
    for (char c: s) {
        if (c >= '0' && c <= '9') {
            i = i * 10 + (c - '0');
        } else {
            // std::cout << "Bad Input"; // std::cout << c'est un printf
            return 0;
        }
    }
    // std::cout << i << std::endl; // std::cout << c'est un printf
    return i;
}