
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
bool initVEML7700(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	if (!veml.begin()) {
	    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
	} else {
		// Default settings from datasheet.
		veml.powerSaveEnable(true);
		veml.setPowerSaveMode(VEML7700_POWERSAVE_MODE1); // 1 mesure par 1 sec, reduit la conso
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
void readVEML7700(){
	xSemaphoreTake( mutex, portMAX_DELAY );
	// veml.enable(true);
	int Gain = VEML7700_GAIN_1_8;
	int IntegrationTime = VEML7700_IT_25MS;
	int wait= 25;
	int Lux=0;

	veml.powerSaveEnable(false);
	veml.setGain(Gain);
	veml.setIntegrationTime(IntegrationTime);

	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);
	vTaskDelay(2);

	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);
	vTaskDelay(2);

	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);
	vTaskDelay(2);

	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);
	vTaskDelay(2);

	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);
	vTaskDelay(2);

	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);

	vTaskDelay( 2*wait );
	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);

	vTaskDelay( wait );
	Lux = veml.readLux();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);

	vTaskDelay( wait );
	Lux = veml.readLuxNormalized();
	Serial.printf("+---> readVEML7700(%i)\n",Lux);

	//			2		1		1/4		1/8
	//		+---------------------------------
	//	800	|	236		472		1887	3775
	//	400	|	472		944		3775	7550
	//	200	|	944		1887	7550	15099
	//	100	|	1887	3775	15099	30199
	//	50 	|	3775	7550	30199	60398
	//	25 	|	7550	15099	60398	120796

	if (Lux < 236*0.9){
		Gain = VEML7700_GAIN_2;
		IntegrationTime = VEML7700_IT_800MS;
		wait= 800;
	    Serial.println("|   +---> GAIN_2 x 800MS");
	} else if (Lux < 472*0.9){
		Gain = VEML7700_GAIN_2;
		IntegrationTime = VEML7700_IT_400MS;
		wait= 400;
	    Serial.println("|   +---> GAIN_2 x 400MS");
	} else if (Lux < 944*0.9){
		Gain = VEML7700_GAIN_2;
		IntegrationTime = VEML7700_IT_200MS;
		wait= 200;
	    Serial.println("|   +---> GAIN_2 x 200MS");
	} else if (Lux < 1887*0.9){
		Gain = VEML7700_GAIN_1;
		IntegrationTime = VEML7700_IT_200MS;
		wait= 200;
	    Serial.println("|   +---> GAIN_1 x 200MS");
	} else if (Lux < 3775*0.9){
		Gain = VEML7700_GAIN_1;
		IntegrationTime = VEML7700_IT_100MS;
		wait= 100;
	    Serial.println("|   +---> GAIN_1 x 100MS");
	} else if (Lux < 7550*0.9){
		Gain = VEML7700_GAIN_1;
		IntegrationTime = VEML7700_IT_50MS;
		wait = 50;
	    Serial.println("|   +---> GAIN_1 x _50MS");
	} else if (Lux < 15099*0.9){
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_100MS;
		wait= 100;
	    Serial.println("|   +---> GAIN_1/4 x 100MS");
	} else if (Lux < 30199*0.9){
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_50MS;
		wait = 50;
	    Serial.println("|   +---> GAIN_1/4 x _50MS");
	} else if (Lux < 60398*0.9){
		Gain = VEML7700_GAIN_1_4;
		IntegrationTime = VEML7700_IT_25MS;
		wait = 25;
	    Serial.println("|   +---> GAIN_1/4 x _25MS");
	} else {
		Gain = VEML7700_GAIN_1_8;
		IntegrationTime = VEML7700_IT_25MS;
		wait = 25;
	    Serial.println("|   +---> GAIN_1/8 x _25MS");
	}
	
	veml.setGain(Gain);
	veml.setIntegrationTime(IntegrationTime);

	vTaskDelay( 2*wait );
	if (Gain == VEML7700_GAIN_1_8 && IntegrationTime == VEML7700_IT_25MS){
		CurrentProbe.Probe.LUX.Set(veml.readLuxNormalized());
	}else{
		CurrentProbe.Probe.LUX.Set(veml.readLux());
	}
    Serial.printf("|   +---> Lux : %s (2*%ims)\n",CurrentProbe.Probe.LUX.toString().c_str(),wait);

	vTaskDelay( wait );
	if (Gain == VEML7700_GAIN_1_8 && IntegrationTime == VEML7700_IT_25MS){
		CurrentProbe.Probe.LUX.Set(veml.readLuxNormalized());
	}else{
		CurrentProbe.Probe.LUX.Set(veml.readLux());
	}
    Serial.printf("|   +---> Lux : %s (%ims)\n",CurrentProbe.Probe.LUX.toString().c_str(),wait);

	vTaskDelay( wait );
	if (Gain == VEML7700_GAIN_1_8 && IntegrationTime == VEML7700_IT_25MS){
		CurrentProbe.Probe.LUX.Set(veml.readLuxNormalized());
	}else{
		CurrentProbe.Probe.LUX.Set(veml.readLux());
	}
    Serial.printf("|   +---> Lux : %s (%ims)\n",CurrentProbe.Probe.LUX.toString().c_str(),wait);
	
	// veml.enable(false);
	veml.powerSaveEnable(true);
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
		// Serial.println("> getSensorData()");
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
		} else {Serial.println("+---> Missing BMP280 sensor!");}
		if(i2cbus.BME280){
			readBME280();
		} else {Serial.println("+---> Missing BME280 sensor!");
		}
		if(i2cbus.SCD40){
			readSCD40();
		} else {Serial.println("+---> Missing CO2 sensor!");
		}
		if(i2cbus.VEML7700){
			readVEML7700();
		} else {Serial.println("+---> Missing Lux sensor!");
		}
		Working = false;
		vTaskDelay( 5000 );
	}
}
