#pragma once

void refreshScreen();
void init_Screen ();
void displayProgressbar (char level, char x = 14, char y = 28, char lenght = 100, char DisplayValue = NULL);
void displayTransfert(bool color);
// void displayBatteryLevel(int batteryLevel);
// void displaySignal(int x);
void displayNetwork();
// void displaySensor();
void displayDeepSleep();
void redrawScreen();
void manageScreen(void * parameter);
