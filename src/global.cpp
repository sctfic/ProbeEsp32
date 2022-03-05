#include "global.h"


// Création d'un mutex
	SemaphoreHandle_t mutex = NULL;

// env variable

bool DeepSleepNow = false;
bool Working = true;
bool OnOff = true;
bool WIFI_CONNECTED = false;

ESP32BOARD CurrentProbe;
