
#include "global.h"
#include "sensors.h"
#include "Web.h"

// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

// define the wiring settings for I2C interface connection
// #define I2C_SDA    5 // 21 // SCK
// #define I2C_SCL    4 // 22
Adafruit_BMP280 bmp; // I2C


// PINs
#define BATTERY_PIN 35
#define POWER_PIN 32
#define LD1_PIN 16

//  Heard led as PWM
const int  FREQ = 5000;
const int  LD1CHANNEL = 0;
const int  RESOLUTION = 8;

void heartPulse(void * parameter){
	// configure LED PWM functionalitites
	ledcSetup(LD1CHANNEL, FREQ, RESOLUTION);
	// attach the channel to the GPIO to be controlled
	ledcAttachPin(LD1_PIN, LD1CHANNEL);
		Serial.println("init PIN");

	// ECG of earth, use 10ms
	// double pulse[]={0,0,0,0,0,0,7.2,15.5,22.9,29.0,33.4,35.9,35.5,29.5,18.8,0,0,0,0,0,-18.2,-36.4,182,182,-72,-36.4,0,0,0,0,0,0,10.1,21.7,32.1,40.6,46.8,50.3,50.9,45.5,28.9,0,0,0,0,0,0,0,0,0,0};

	// wave of earth, use 20ms
	// int pulse[] = {3,4,47,78,112,144,166,180,179,150,98,44,118,169,210,239,254,255,224,137,58,44,36,33,29,26,22,18,15,11,7,4};

	// wave of sin()
	int pulse[] = {0,1,5,14,27,45,65,89,114,139,164,188,209,227,241,250,255,255,249,239,224,206,184,160,135,109,85,62,41,25,12,4,1,0};
	int N = sizeof(pulse) / sizeof(pulse[0]);

	for (;;) {
		for(int x = 0 ; x < N; x++){
			ledcWrite(LD1CHANNEL, (int)(255-pulse[x]));
			if ((x%3) == 0){
				OnOff = !OnOff;
			}
			vTaskDelay(50-Working);
		}
		vTaskDelay(250);
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
	float Vread = ((float)analogRead(BATTERY_PIN) / 4096 * 3.3);
	//   Serial.printf("TensionPin(35): %f\n",Vread);
	// Serial.printf("Tension Battery: %fV\n",BatVoltage);
	return Vread*(20120+9670)/20120;
}
double getPowerVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(POWER_PIN) / 4096 * 3.3);
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
		BMP280_CONNECTED = true;
	}
    xSemaphoreGive( mutex );
}
void readBMP280(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	Serial.println("+---> readBMP280()");
	if (bmp.takeForcedMeasurement()) {
		CurrentProbe.Probe.Temperature = MEASURE( bmp.readTemperature(), "°C"); // bme.readTemperature();
		CurrentProbe.Probe.Pressure = MEASURE( bmp.readPressure()/100, "hPa"); // bme.readPressure() / 100;
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
		Serial.println("> getSensorData() ===============================================================================");
		Working = true;
		CurrentProbe.Probe.CO2 = MEASURE( rand() % 100 + 415, "ppm");
		CurrentProbe.Probe.Humidity = MEASURE( rand() % 40 + 30, "ppm");
		CurrentProbe.Energy.PowerSupply.Voltage = MEASURE( getPowerVoltage(), "V");
		CurrentProbe.Energy.Battery.Voltage =  MEASURE( getBatteryVoltage(), "V");
		CurrentProbe.Energy.Battery.Capacity = MEASURE( getBatteryCapacity(CurrentProbe.Energy.Battery.Voltage.Value, CurrentProbe.Energy.PowerSupply.Voltage.Value), "%");
		if(BMP280_CONNECTED){
			readBMP280();
		} else {Serial.println("+---> Missing BMP280 sensor!");}
		if(BME280_CONNECTED){
			readBME280();
		// } else {Serial.println("+---> Missing BME280 sensor!");
		}
		if(CO2_CONNECTED){
			readCO2();
		// } else {Serial.println("+---> Missing CO2 sensor!");
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
			vTaskDelay( 200 );
		}
	}
}
