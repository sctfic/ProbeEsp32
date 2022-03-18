
#include "global.h"
#include "sensors.h"
#include "Web.h"

// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C

// // PINs
// #define BATTERY_PIN 35
// #define POWER_PIN 32
// #define LD1_PIN 16

//  Heard led as PWM
const int  FREQ = 5000;
const int  LD1CHANNEL = 0;
const int  RESOLUTION = 8;

void heartPulse(void * parameter){
	// configure LED PWM functionalitites
	ledcSetup(LD1CHANNEL, FREQ, RESOLUTION);
	// attach the channel to the GPIO to be controlled
	ledcAttachPin(Pin.LD1, LD1CHANNEL);
		Serial.println("init PIN");

	// ECG of earth, use 10ms
	// double pulse[]={0,0,0,0,0,0,7.2,15.5,22.9,29.0,33.4,35.9,35.5,29.5,18.8,0,0,0,0,0,-18.2,-36.4,182,182,-72,-36.4,0,0,0,0,0,0,10.1,21.7,32.1,40.6,46.8,50.3,50.9,45.5,28.9,0,0,0,0,0,0,0,0,0,0};

	// wave of earth, use 20ms
	// int pulse[] = {3,4,47,78,112,144,166,180,179,150,98,44,118,169,210,239,254,255,224,137,58,44,36,33,29,26,22,18,15,11,7,4};

	// wave of sin()
	int pulse[] = {0,1,5,14,27,45,65,89,114,139,164,188,209,227,241,250,255,255,249,239,224,206,184,160,135,109,85,62,41,25,12,4,1,0,0,0,0,0,0,0,0,0};
	int N = sizeof(pulse) / sizeof(pulse[0]);

	for (;;) {
		for(int x = 0 ; x < N; x++){
			ledcWrite(LD1CHANNEL, (int)(255-pulse[x]));
			if ((x%3) == 0){
				OnOff = !OnOff;
			}
			vTaskDelay(50-Working*30);
		}
		// vTaskDelay(250);
	}
}
double getBatteryCapacity(double VBat, double VPower){
	//  equation d'une Li-Ion sous faible charge a 20C
	if (VPower > VBat && VPower > 4.2){
		// Charging !
		return 101;
	} else {
		return (int)lround(-47391.44 + 68111.053*VBat -38668.575*pow(VBat,2) + 10828.2011*pow(VBat,3) + -1494.5436*pow(VBat,4) + 81.37904*pow(VBat,5));
	}
}
double getBatteryVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(Pin.BATTERY) / 4096 * 3.3);
	//   Serial.printf("TensionPin(35): %f\n",Vread);
	// Serial.printf("Tension Battery: %fV\n",BatVoltage);
	return Vread*(20120+9670)/20120;
}
double getPowerVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(Pin.POWER) / 4096 * 3.3);
	// Serial.printf("TensionPin(35): %f\n",Vread);
	// Serial.printf("Tension Battery: %fV\n",BatVoltage);
	return Vread*2;
}
void initBMP280(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
	    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));

	} else {
		// Default settings from datasheet.
  		bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
						Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
						Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
						Adafruit_BMP280::FILTER_X16,      /* Filtering. */
						Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
		i2cbus.BMP280 = true;
	}
    xSemaphoreGive( mutex );
}
void initBME280(){
	xSemaphoreTake( mutex, portMAX_DELAY );

    xSemaphoreGive( mutex );
}
void readBMP280(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	Serial.println("+---> readBMP280()");
	if (bmp.takeForcedMeasurement()) {
		// Serial.println(CurrentProbe.Probe.Pressure.toJson().c_str());
		CurrentProbe.Probe.Temperature.Set( bmp.readTemperature()); // bme.readTemperature();
		CurrentProbe.Probe.Pressure.Set(bmp.readPressure()); // bme.readPressure() / 100;
		// Serial.println(CurrentProbe.Probe.Pressure.toJson().c_str());
    	Serial.printf("|   +---> Temperature : %s\n",CurrentProbe.Probe.Temperature.toString().c_str());
    	Serial.printf("|   +---> Pressure : %s\n",CurrentProbe.Probe.Pressure.toString().c_str());
	} else {
		Serial.println("Forced measurement failed!");
	}
    xSemaphoreGive( mutex );
}
void readBME280(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	Serial.println("+---> readBME280()");

    xSemaphoreGive( mutex );
}
void readCO2(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	Serial.println("+---> readCO2()");

    xSemaphoreGive( mutex );
}
void getSensorData(void * parameter) {
	initBMP280();
	DeepSleepNow = false;
	for (;;) {
		Serial.println("> getSensorData()");
		Working = true;
		CurrentProbe.Probe.CO2.Set( rand() % 100 + 415);
    	Serial.printf("|   +---> CO2 : %s\n",CurrentProbe.Probe.CO2.toString().c_str());
		CurrentProbe.Probe.Humidity.Set( rand() % 40 + 30);
    	Serial.printf("|   +---> Humidity : %s\n",CurrentProbe.Probe.Humidity.toString().c_str());
		CurrentProbe.Energy.PowerSupply.Voltage.Set( getPowerVoltage());
    	Serial.printf("|   +---> PowerSupply.Voltage : %s\n",CurrentProbe.Energy.PowerSupply.Voltage.toString().c_str());
		CurrentProbe.Energy.Battery.Voltage.Set( getBatteryVoltage());
    	Serial.printf("|   +---> Battery.Voltage : %s\n",CurrentProbe.Energy.Battery.Voltage.toString().c_str());
		CurrentProbe.Energy.Battery.Capacity.Set( getBatteryCapacity(CurrentProbe.Energy.Battery.Voltage.Raw, CurrentProbe.Energy.PowerSupply.Voltage.Raw) );
    	Serial.printf("|   +---> Battery.Capacity : %s\n",CurrentProbe.Energy.Battery.Capacity.toString().c_str());
		if(i2cbus.BMP280){
			readBMP280();
		} else {Serial.println("+---> Missing BMP280 sensor!");}
		if(i2cbus.BME280){
			readBME280();
		} else {Serial.println("+---> Missing BME280 sensor!");
		}
		if(i2cbus.CO2){
			readCO2();
		} else {Serial.println("+---> Missing CO2 sensor!");
		}
		if (WIFI_CONNECTED) {
			// read CO2 SPi or I2C
			DeepSleepNow = uploadData(CurrentProbe.toJson().c_str(), CurrentProbe.Settings.SrvDataBase2Post.c_str());
			if(!CurrentProbe.Settings.EnableDeepSleep){
				vTaskDelay( CurrentProbe.Settings.MeasurementInterval * 1000);
			} else{
				delay( 1000 );
			}
			Working = false;
		} else {
			int i = 600; // 600 = 1 min
			while (!WIFI_CONNECTED && i>0) {
				vTaskDelay( 100 );
				i--;
			}
			
		}
	}
}
