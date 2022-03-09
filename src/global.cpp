// #include <FreeRTOS.h>
// #include <semphr.h>
#include "global.h"


// Création d'un mutex
	SemaphoreHandle_t mutex = NULL;

// env variable

bool DeepSleepNow = false;
bool Working = true;
bool OnOff = true;
bool WIFI_CONNECTED = false;
const char * SettingsPath = "/Settings.json";

ESP32BOARD CurrentProbe;
