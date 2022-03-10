
#include "global.h"
#include "sensors.h"
#include "Web.h"

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
	int pulse[] = {3,4,47,78,112,144,166,180,179,150,98,44,118,169,210,239,254,255,224,137,58,44,36,33,29,26,22,18,15,11,7,4};

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
void getSensorData(void * parameter) {
	for (;;) {
		if (WIFI_CONNECTED) {
			Working = true;
			CurrentProbe.Probe.Temperature = MEASURE( rand() % 20, "°C"); // bme.readTemperature();
			CurrentProbe.Probe.Humidity = MEASURE( rand() % 40 + 30, "%"); // bme.readHumidity();
			CurrentProbe.Probe.Pressure = MEASURE( rand() % 100 + 950, "hPa"); // bme.readPressure() / 100;
			CurrentProbe.Probe.CO2 = MEASURE( rand() % 100 + 415, "ppm");
			CurrentProbe.Energy.PowerSupply.Voltage = MEASURE( getPowerVoltage(), "V");
			CurrentProbe.Energy.Battery.Voltage =  MEASURE( getBatteryVoltage(), "V");
			CurrentProbe.Energy.Battery.Capacity = MEASURE( getBatteryCapacity(CurrentProbe.Energy.Battery.Voltage.Value, CurrentProbe.Energy.PowerSupply.Voltage.Value), "%");
			xSemaphoreTake( mutex, portMAX_DELAY );
				// read CO2 SPi or I2C
			xSemaphoreGive( mutex );
			DeepSleepNow = uploadData(CurrentProbe.toJson().c_str(), CurrentProbe.Settings.SrvDataBase2Post.c_str());
			vTaskDelay( pdMS_TO_TICKS( 20000 ) );
			Working = false;
		} else {
			vTaskDelay( pdMS_TO_TICKS( 200 ) );
		}
	}
}
