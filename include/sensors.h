#pragma once


void getSensorData(void * parameter);
void heartPulse(void * parameter);
double getPowerVoltage();
double getBatteryVoltage();
/**
 * calibrateSCD40() - Etalonage selon les valeur reele du moment
 *
 * @note Available during measurements.
 *
 * @param TempOffset Temperature offset in °C
 *
 * @param realPressure Ambient pressure in hPa
 *
 * @param realCO2 valeur connu de CO2 atmospherique en ppm
 *
 * @return 0 on success, an error code otherwise
 */
void calibrateSCD40(double TempOffset, double realPressure, int realCO2);
