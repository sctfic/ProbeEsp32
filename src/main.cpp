#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <string.h>

// #include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
// #include <Adafruit_NeoPixel.h>
 
const char *SSID = "MartinLopez";
const char *PWD = "0651818124";

#define BATTERY_PIN 35
#define LD1 16

// JSON data buffer
StaticJsonDocument<500> jsonDocument;
char json[500];
 
// env variable
float temperature=1.01;
float humidity=0.01;
float pressure=0.01;
float CO2=0.01;
float BatVoltage=0.01;
int BatCapacity=90;
float PowerVoltage=0.01;

String IP;
String MAC;
String CIDR;
String GW;
String DNS;
int attenuation;
// #include <SPI.h>
// #include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED pins
#define OLED_SDA 5
#define OLED_SCL 4 
#define OLED_RST 15 // 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_BLUE_0 16 // OLED blue area start

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Web server running on port 80
WebServer server(80);
 
// Sensor
// Adafruit_BME280 bme;

// Neopixel LEDs strip
// Adafruit_NeoPixel pixels(NUM_OF_LEDS, PIN, NEO_GRB + NEO_KHZ800);
 // 'logoNB', 25x48px
const unsigned char logoNB [] PROGMEM = {
  0x00, 0x10, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x00,
  0x00, 0x18, 0x00, 0x00,
  0x00, 0x18, 0x00, 0x00, 
	0x00, 0x18, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x00,
  0x00, 0x28, 0x00, 0x00, 
	0x00, 0x28, 0x00, 0x00,
  0x00, 0x08, 0x00, 0x00,
  0x00, 0x50, 0x00, 0x00,
  0x00, 0x90, 0x00, 0x00, 
	0x01, 0xb0, 0x00, 0x00,
  0x01, 0x20, 0x00, 0x00,
  0x02, 0x20, 0x00, 0x00,
  0x04, 0x20, 0x00, 0x00, 
	0x04, 0x00, 0x00, 0x00,
  0x04, 0x20, 0x20, 0x00,
  0x08, 0x20, 0x30, 0x00,
  0x08, 0x10, 0x38, 0x00, 
	0x08, 0x18, 0x48, 0x00,
  0x08, 0x05, 0x88, 0x00,
  0x08, 0x00, 0x08, 0x00,
  0x08, 0x18, 0x08, 0x00, 
	0x00, 0x18, 0x08, 0x00,
  0x04, 0x10, 0x08, 0x00,
  0x04, 0x04, 0x08, 0x00,
  0x02, 0x24, 0x08, 0x00, 
	0x02, 0x24, 0x10, 0x00,
  0x42, 0x02, 0x10, 0x00,
  0x62, 0x42, 0x10, 0x00,
  0x72, 0x41, 0x10, 0x00, 
	0x4c, 0x81, 0x10, 0x00,
  0x01, 0x80, 0x90, 0x00,
  0x41, 0x00, 0xc8, 0x00,
  0x42, 0x00, 0x47, 0x00, 
	0x62, 0x00, 0x01, 0x00,
  0x34, 0x00, 0x21, 0x00,
  0x0c, 0x00, 0x01, 0x00,
  0x08, 0x34, 0x11, 0x00, 
	0x08, 0x22, 0x11, 0x00,
  0x00, 0x02, 0x12, 0x00,
  0x08, 0x26, 0x16, 0x00,
  0x08, 0x18, 0x1c, 0x00, 
	0x04, 0x00, 0x30, 0x00,
  0x06, 0x00, 0x20, 0x00,
  0x02, 0x00, 0x40, 0x00,
  0x00, 0xcf, 0x80, 0x00
};

void progressbar (char level, char x = 14, char y = 28, char lenght = 100){
  display.fillRoundRect(x-3, y-3, lenght+6, 10, 5, BLACK);
  display.drawRoundRect(x-2, y-2, lenght+4, 8, 4, WHITE);
  display.fillRoundRect(x, y, lround(lenght*level/100), 4, 2, WHITE);
  display.setTextSize(1);
  display.fillRect(x+lround(lenght/2)-7,y+8, 14, 7, BLACK);
  if (level < 10){ // underBar
	display.setCursor(x+lround(lenght/2)-4,y+8);
  } else {
	display.setCursor(x+lround(lenght/2)-7,y+8);
  }
  // display.setCursor(x+2+lround(lenght*level/100),y-1); // inBar
  
  display.printf("%i%%",level);
  // display.setCursor(x+lround(lenght/2)+5,y+8);
  // display.println("%");
}
void init_Screen (){
  Serial.println("Enable OLED screen :");
  //initialize OLED
  pinMode(OLED_RST, OUTPUT);
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
	Serial.println(F("SSD1306 allocation failed"));
	for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  Serial.println("Screen Enabled!");
}

void displayBatteryLevel(int batteryLevel){
		display.fillRect(120, 0, 8, 16, BLACK);
		display.drawPixel(123, 0, WHITE);
		display.drawPixel(124, 0, WHITE);
		display.drawRect(120, 1, 8, 15, WHITE);
	if (batteryLevel > 100) {
  		display.drawLine(121, 4, 126, 12, WHITE);
  		display.drawLine(121, 12, 126, 4, WHITE);
	} else if(batteryLevel > 10) {
		display.fillRect(122, 3+lround((100-batteryLevel)*0.11), 4, lround((batteryLevel)*0.11), WHITE);
	} else {
		display.display();
		delay(500);
		display.drawRect(123, 4, 2, 6, WHITE);
		display.drawRect(123, 11, 2, 2, WHITE);
		/* code */
	}
}
float getBatteryCapacity(float VBAT){
	struct batteryCapacity {
		float voltage;
		int capacity;
	};
	const batteryCapacity remainingCapacity[] = {
		4.50,   120,
		4.30,   110,
		4.20,   100,
		4.15,   98,
		4.10,   96,
		4.00,   92,
		3.96,   89,
		3.92,   85,
		3.89,   81,
		3.86,   77,
		3.83,   73,
		3.80,   69,
		3.77,   65,
		3.75,   62,
		3.72,   58,
		3.70,   55,
		3.66,   51,
		3.62,   47,
		3.58,   43,
		3.55,   40,
		3.51,   35,
		3.48,   32,
		3.44,   26,
		3.40,   24,
		3.37,   20,
		3.35,   17,
		3.27,   13,
		3.20,   9,
		3.1,    6,
		3.00,   3,
		2.9,   0,
	};
	const int step = sizeof(remainingCapacity) / sizeof(struct batteryCapacity);
	int i = 0;
	while (VBAT < remainingCapacity[i].voltage && i < step) {
		i++;
	}
	BatCapacity = remainingCapacity[i].capacity;
	Serial.printf("Pourcentage Battery: %i%%\n",BatCapacity);
	return BatCapacity;
}
float getBatteryVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(BATTERY_PIN) / 4096 * 3.3);
	//   Serial.printf("TensionPin(35): %f\n",Vread);
	BatVoltage =  Vread*(20120+9670)/20120;
	Serial.printf("Tension Battery: %fV\n",BatVoltage);
	return BatVoltage;
}
void displayNetwork(){
	display.setTextSize(0);
	display.setCursor(0,0);
	display.printf("%s\n",SSID);
	// display.setCursor(0,8);
	display.print(IP);

	attenuation = WiFi.RSSI();
	Serial.print("RSSI: ");
	Serial.println(attenuation);
	display.fillRect(99, 0, 20, 16, BLACK);
	if (attenuation >= -80){
		display.fillRect(99, 12, 4, 4, WHITE);
	} else {
		display.drawRect(99, 12, 4, 4, WHITE);
	}
	if (attenuation >= -73){
		display.fillRect(104, 8, 4, 8, WHITE);
	} else {
		display.drawRect(104, 8, 4, 8, WHITE);
	}
	if (attenuation >= -67){
		display.fillRect(109, 4, 4, 12, WHITE);
	} else {
		display.drawRect(109, 4, 4, 12, WHITE);
	}
	if (attenuation >= -60){
		display.fillRect(114, 0, 4, 16, WHITE);
	} else {
		display.drawRect(114, 0, 4, 16, WHITE);
	}
	if (attenuation <= -80){
		display.drawLine(99, 0, 118, 14, WHITE);
		display.drawLine(99, 1, 118, 15, WHITE);
		display.drawLine(99, 2, 118, 16, BLACK);
		display.drawLine(100, 0, 119, 14, BLACK);
	}
  	// progressbar (attenuation*-1);

}

void init_ToWiFi (){
  // Connect to Wi-Fi network with SSID and PWD
	Serial.print("Connecting to wifi ");
	Serial.print(SSID);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	WiFi.begin(SSID, PWD);
	while (WiFi.status() != WL_CONNECTED) {
		digitalWrite(LD1, HIGH);
		delay(200);
		digitalWrite(LD1, LOW);
		Serial.print(".");
		delay(200);
	}
	// Print local IP address and start web server
	Serial.println("WiFi connected.");
	Serial.printf("IP address: ");
	IP = WiFi.localIP().toString();
	Serial.print(IP);
	GW = WiFi.gatewayIP().toString();
	Serial.print(GW);
	CIDR = (String)(WiFi.subnetCIDR());
	Serial.print(CIDR);
	MAC = WiFi.macAddress();
	Serial.print(MAC);
	DNS = WiFi.dnsIP().toString();
	Serial.println(DNS);
}
 

void read_sensor_data(void * parameter) {
	for (;;) {
		temperature = 8.3; // bme.readTemperature();
		humidity = 56.2; // bme.readHumidity();
		pressure = 1003.5; // bme.readPressure() / 100;
		Serial.println("Read sensor data !!");
		// delay the task
		vTaskDelay(60000 / portTICK_PERIOD_MS);
	}
}

void getJSON() {
	Serial.println("Build Json");
	display.setCursor(30,SCREEN_BLUE_0+4);
	display.printf("Temp:%.1f`C\n",temperature);
	display.setCursor(30,SCREEN_BLUE_0+4+9);
	display.printf("Press:%.1fhPa\n",pressure);
	display.setCursor(30,SCREEN_BLUE_0+4+18);
	display.printf("Hum:%.1f%%",humidity);
	display.setCursor(30,SCREEN_BLUE_0+4+27);
	display.printf("CO2:%.0f%%",CO2);

	String json ="{";
		json += "\"Temperature\": {\"value\":"+String(temperature)+",\"unit\": \"°C\"},";
		json += "\"Pressure\": {\"value\":"+String(pressure)+",\"unit\": \"hPa\"},";
		json += "\"Humidity\": {\"value\":"+String(humidity)+",\"unit\": \"%\"},";
		json += "\"CO2\": {\"value\":"+String(CO2)+",\"unit\": \"ppm\"},";
		json += "\"Network\": {\"SSID\": \""+String(SSID)+"\",\"MAC\": \""+String(MAC)+"\",\"IP\": \""+String(IP)+"\",\"CIDR\": \""+String(CIDR)+"\",\"Gateway\": \""+String(GW)+"\",\"DNS\": \""+String(DNS)+"\",\"Strength\": {\"value\":"+String(attenuation)+",\"unit\": \"dBm\"}},";
		json += "\"Battery\": {\"Voltage\": {\"value\":"+String(BatVoltage)+",\"unit\": \"V\"},\"Capacity\": {\"value\":"+String(BatCapacity)+",\"unit\": \"%\"}},";
		json += "\"Power\": {\"value\":"+String(PowerVoltage)+",\"unit\": \"V\"}}";

	server.send(200, "application/json", json);


}

void handlePost() {
  if (server.hasArg("plain") == false) {
	//handle error here
  }

  String body = server.arg("plain");
  Serial.println(body);
  deserializeJson(jsonDocument, body);

  // Respond to the client
  server.send(200, "application/json", "{}");
}
 
// setup API resources
void setup_routing() {
	server.on("/", getJSON);
	server.on("/led", HTTP_POST, handlePost);
	Serial.println("setup_routing()");
	// start server
	server.begin();
}


void setup_task() {
	xTaskCreate(
		read_sensor_data,    
		"Read sensor data",   // Name of the task (for debugging)
		1000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL             // Task handle
	);
	Serial.println("setup_task()");
}
 
void setup() {
	Serial.begin(115200);
	pinMode(BATTERY_PIN, INPUT);
	pinMode(LD1, OUTPUT);

	// Sensor setup
	// if (!bme.begin(0x76)) {
	//   Serial.println("Problem connecting to BME280");
	// }
	init_Screen();
	init_ToWiFi();
	setup_task();
	setup_routing();  

	display.drawBitmap(0, SCREEN_BLUE_0,  logoNB, 25, 48, WHITE);

}
 
void loop() {
	float VBAT = getBatteryVoltage();
	int LevelBAT = getBatteryCapacity(VBAT);
	displayBatteryLevel(LevelBAT);
	displayNetwork();
	display.display();
	server.handleClient();
	delay(200);
    digitalWrite(LD1, LOW);
	delay(200);
    digitalWrite(LD1, HIGH);
}