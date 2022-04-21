#pragma once


void getSensorData(void * parameter);
void heartPulse(void * parameter);

/**
 * calibrateSCD40() - The set_ambient_pressure command can be sent
 * during periodic measurements to enable continuous pressure compensation.
 * Note that setting an ambient pressure to the sensor overrides any
 * pressure compensation based on a previously set sensor altitude.
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
