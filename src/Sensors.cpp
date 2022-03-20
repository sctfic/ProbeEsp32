
#include "global.h"
#include "sensors.h"
#include "Web.h"

// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_VEML7700.h"


Adafruit_BMP280 bmp; // I2C

// // PINs
// #define BATTERY_PIN 35
// #define POWER_PIN 32
// #define LD1_PIN 16

//  Heard led as PWM
const int  FREQ = 5000;
const int  LD1CHANNEL = 0;
const int  RESOLUTION = 8;

Adafruit_VEML7700 veml = Adafruit_VEML7700();


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
	int pulse[] = {0,1,5,14,27,45,65,89,114,139,164,188,209,227,241,249,239,224,206,184,160,135,109,85,62,41,25,12,4,1,0,0};
	int N = sizeof(pulse) / sizeof(pulse[0]);

	for (;;) {
		for(int x = 0 ; x < N; x++){
			ledcWrite(LD1CHANNEL, (int)(255-pulse[x]));
			if ((x%3) == 0){
				OnOff = !OnOff;
			}
			vTaskDelay(50-Working*40);
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
bool initVEML7700(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	if (!veml.begin()) {
	    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
	} else {
		// Default settings from datasheet.
		veml.powerSaveEnable(false);
		// veml.setGain(VEML7700_GAIN_1_4);
		// veml.setIntegrationTime(VEML7700_IT_25MS);
		// veml.setPowerSaveMode(VEML7700_POWERSAVE_MODE1); // 1 mesure par 1 sec, reduit la conso
		// veml.setPowerSaveMode(VEML7700_POWERSAVE_MODE4); // 1 mesure par 5 sec, reduit la conso
    	xSemaphoreGive( mutex );
		return true;
	}
	// veml.setLowThreshold(10000); //veml.readALS() value for event event
	// veml.setHighThreshold(50000); //veml.readALS() value for event event
	// veml.interruptEnable(true);
    xSemaphoreGive( mutex );
	return false;
}
bool initBMP280(){
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
	    xSemaphoreGive( mutex );
		return true;
	}
	xSemaphoreGive( mutex );
	return false;
}
bool initBME280(){
	xSemaphoreTake( mutex, portMAX_DELAY );

    xSemaphoreGive( mutex );
	return false;
}
bool initSCD40(){
	xSemaphoreTake( mutex, portMAX_DELAY );

    xSemaphoreGive( mutex );
	return false;
}
// int readVEML7700Flux(int Gain, int IntegrationTime){
// 	int wait, als ;
// 	float Lux=0;
// 	// Serial.print("+---> GAIN_");
// 	switch (Gain) {
// 		case VEML7700_GAIN_1: Serial.print("1"); break;
// 		case VEML7700_GAIN_2: Serial.print("2"); break;
// 		case VEML7700_GAIN_1_4: Serial.print("1/4"); break;
// 		case VEML7700_GAIN_1_8: Serial.print("1/8"); break;
// 	}
// 	Serial.print(" x ");
// 	switch (IntegrationTime) {
// 		case VEML7700_IT_25MS:  Serial.print("25");wait=25; break;
// 		case VEML7700_IT_50MS:  Serial.print("50");wait=50; break;
// 		case VEML7700_IT_100MS: Serial.print("100");wait=100; break;
// 		case VEML7700_IT_200MS: Serial.print("200");wait=200; break;
// 		case VEML7700_IT_400MS: Serial.print("400");wait=400; break;
// 		case VEML7700_IT_800MS: Serial.print("800");wait=800; break;
// 	}
// 	Serial.print(" ms\t: ");
// 	xSemaphoreTake( mutex, portMAX_DELAY );

// 	// veml.powerSaveEnable(false);
// 	veml.setGain(Gain);
// 	veml.setIntegrationTime(IntegrationTime);
// 	vTaskDelay( 2*wait+5 );
// 	Lux = veml.readLuxNormalized();
// 	als = veml.readALS();
// 	Serial.printf("%.2f[%i]\t",Lux,als);

// 	// vTaskDelay( 2*wait+5 );
// 	// 	Lux = veml.readLuxNormalized();
// 	// 	als = veml.readALS();
// 	// Serial.printf("%.2f[%i]\t",Lux,als);

// 	vTaskDelay( 2*wait+5 );
// 		Lux = veml.readLuxNormalized();
// 		als = veml.readALS();
// 	Serial.printf("%.2f[%i]",Lux,als);

//     xSemaphoreGive( mutex );

// 	if (als > 50000 || als < 10000) Serial.print("~");
// 	Serial.println("");
// 	return Lux;
// }
float calibrateVEML7700(bool wait){
	int Gain, IntegrationTime, pause=25;
	float Lux;
	//			2			1			1/4		1/8
	//		+----------------------------------------
	//	800	|	236			472			1887	3775
	//	400	|	472			944			3775.	7550
	//	200	|	944			1887		7550.	15099
	//	100	|	1887		3775		15099.	30199
	//	50 	|	3775		7550		30199.	60398
	//	25 	|	7550		15099		60398.	120796
	xSemaphoreTake( mutex, portMAX_DELAY );
		veml.enable(true);
		// veml.powerSaveEnable(false);
		veml.setGain(VEML7700_GAIN_1_4);
		veml.setIntegrationTime(VEML7700_IT_25MS);
    xSemaphoreGive( mutex );
	vTaskDelay( 2*pause );
	xSemaphoreTake( mutex, portMAX_DELAY );
		Lux = veml.readLux();
    xSemaphoreGive( mutex );

	if (Lux < 3775*0.7){
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_400MS;
		pause = 400;
	} else if (Lux < 7550*0.7){
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_200MS;
		pause = 200;
	} else if (Lux < 15099*0.7){
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_100MS;
		pause = 100;
	} else if (Lux < 30199*0.7){
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_50MS;
		pause = 50;
	} else {
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_25MS;
		pause = 25;
	}
	
	xSemaphoreTake( mutex, portMAX_DELAY );
		veml.setGain(Gain);
		veml.setIntegrationTime(IntegrationTime);
		// veml.powerSaveEnable(true);
    xSemaphoreGive( mutex );
	if (wait){
		vTaskDelay( 2*pause );
		xSemaphoreTake( mutex, portMAX_DELAY );
			Lux = veml.readLux();
		xSemaphoreGive( mutex );
	}
	return Lux;
}
void readVEML7700(){
	int Gain, IntegrationTime, ALS;
	CurrentProbe.Probe.LUX.Set(calibrateVEML7700(true));
	xSemaphoreTake( mutex, portMAX_DELAY );
		veml.enable(false);
    xSemaphoreGive( mutex );
	// vTaskDelay( 100 );
	// xSemaphoreTake( mutex, portMAX_DELAY );
	// 	Lux = veml.readLux();
	// 	ALS = veml.readALS();
	// 	printf("%.3f [%i]\n",Lux,ALS);
	// 	veml.enable(false);
    // xSemaphoreGive( mutex );

    Serial.printf("|   +---> Lux : %s\n",CurrentProbe.Probe.LUX.toString().c_str());

	// readVEML7700Flux(VEML7700_GAIN_1_8 ,VEML7700_IT_25MS);
	// CurrentProbe.Probe.LUX.Set(readVEML7700Flux(VEML7700_GAIN_1_8 ,VEML7700_IT_50MS));
	// readVEML7700Flux(VEML7700_GAIN_1_8 ,VEML7700_IT_100MS);
	// readVEML7700Flux(VEML7700_GAIN_1_8 ,VEML7700_IT_200MS);
	// readVEML7700Flux(VEML7700_GAIN_1_8 ,VEML7700_IT_400MS);
	// // readVEML7700Flux(VEML7700_GAIN_1_8 ,VEML7700_IT_800MS);

	// readVEML7700Flux(VEML7700_GAIN_1, VEML7700_IT_25MS);
	// readVEML7700Flux(VEML7700_GAIN_1, VEML7700_IT_50MS);
	// readVEML7700Flux(VEML7700_GAIN_1, VEML7700_IT_100MS);
	// readVEML7700Flux(VEML7700_GAIN_1, VEML7700_IT_200MS);
	// readVEML7700Flux(VEML7700_GAIN_1, VEML7700_IT_400MS);
	// // readVEML7700Flux(VEML7700_GAIN_1, VEML7700_IT_800MS);

	// readVEML7700Flux(VEML7700_GAIN_1_4, VEML7700_IT_25MS);
	// readVEML7700Flux(VEML7700_GAIN_1_4, VEML7700_IT_50MS);
	// readVEML7700Flux(VEML7700_GAIN_1_4, VEML7700_IT_100MS);
	// readVEML7700Flux(VEML7700_GAIN_1_4, VEML7700_IT_200MS);
	// readVEML7700Flux(VEML7700_GAIN_1_4, VEML7700_IT_400MS);
	// // readVEML7700Flux(VEML7700_GAIN_1_4, VEML7700_IT_800MS);

	// readVEML7700Flux(VEML7700_GAIN_2, VEML7700_IT_25MS);
	// readVEML7700Flux(VEML7700_GAIN_2, VEML7700_IT_50MS);
	// readVEML7700Flux(VEML7700_GAIN_2, VEML7700_IT_100MS);
	// readVEML7700Flux(VEML7700_GAIN_2, VEML7700_IT_200MS);
	// readVEML7700Flux(VEML7700_GAIN_2, VEML7700_IT_400MS);
	// // readVEML7700Flux(VEML7700_GAIN_2, VEML7700_IT_800MS);


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
void readSCD40(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	Serial.println("+---> readSCD40()");

    xSemaphoreGive( mutex );
}
void getSensorData(void * parameter) {
	i2cbus.BMP280 = initBMP280();
	i2cbus.VEML7700 = initVEML7700();
	for (;;) {
		Working = true;
		// Transfert = true;
		Serial.println("+---> readSensors()");
		// CurrentProbe.Probe.CO2.Set( rand() % 100 + 415);
    	// Serial.printf("|   +---> CO2 : %s\n",CurrentProbe.Probe.CO2.toString().c_str());
		// CurrentProbe.Probe.Humidity.Set( rand() % 40 + 30);
    	// Serial.printf("|   +---> Humidity : %s\n",CurrentProbe.Probe.Humidity.toString().c_str());
		CurrentProbe.Energy.PowerSupply.Voltage.Set( getPowerVoltage());
    	Serial.printf("|   +---> PowerSupply.Voltage : %s\n",CurrentProbe.Energy.PowerSupply.Voltage.toString().c_str());
		CurrentProbe.Energy.Battery.Voltage.Set( getBatteryVoltage());
    	Serial.printf("|   +---> Battery.Voltage : %s\n",CurrentProbe.Energy.Battery.Voltage.toString().c_str());
		CurrentProbe.Energy.Battery.Capacity.Set( getBatteryCapacity(CurrentProbe.Energy.Battery.Voltage.Raw, CurrentProbe.Energy.PowerSupply.Voltage.Raw) );
    	Serial.printf("|   +---> Battery.Capacity : %s\n",CurrentProbe.Energy.Battery.Capacity.toString().c_str());
		if(i2cbus.BMP280){
			readBMP280();
		} else {Serial.println("+---> Missing BMP280 sensor!");
		}
		if(i2cbus.VEML7700){
			readVEML7700();
		} else {Serial.println("+---> Missing Lux sensor!");
		}
		if(i2cbus.BME280){
			readBME280();
		} else {Serial.println("+---> Missing BME280 sensor!");
		}
		if(i2cbus.SCD40){
			readSCD40();
		} else {Serial.println("+---> Missing SCD40[CO2] sensor!");
		}
		// Transfert = false;
		Working = false;
		vTaskDelay( 5000 );
		DataReady = true;
	}
}
