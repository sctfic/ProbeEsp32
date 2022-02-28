#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <freertos/semphr.h>
// Création d'un mutex
	SemaphoreHandle_t mutex = NULL;

#include <string.h>
#include <HTTPClient.h>
// #include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

const char *SSID = "MartinLopez";
const char *PWD = "0651818124";
const char *hostname = "ESP32NodeSensor_1";
const char* SrvPostData = "http://rpimaster/probe";
unsigned long previousMillis = 0;
unsigned long interval = 30000;

#define DELAY_READ_PROBES 10 // in seconds
#define BATTERY_PIN 35
#define LD1 16
const int  FREQ = 5000;
const int  LD1CHANNEL = 0;
const int  RESOLUTION = 8;

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

	String HostName;
	String IP;
	String MAC;
	String CIDR;
	String GW;
	String DNS;
	int attenuation;
	boolean Working = true;
	boolean OnOff = true;

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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); // , 120000UL, 120000UL
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

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define NONE 0

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
void progressbar (char level, char x = 14, char y = 28, char lenght = 100, char DisplayValue = NONE){
	display.fillRoundRect(x-3, y-3, lenght+6, 10, 5, BLACK);
	display.drawRoundRect(x-2, y-2, lenght+4, 8, 4, WHITE);
	display.fillRoundRect(x, y, lround(lenght*level/100), 4, 2, WHITE);
	display.setTextSize(1);
	display.fillRect(x+lround(lenght/2)-8,y+7, 16, 9, BLACK);
	int X;
	if (level < 10){ // underBar
		X=x+lround(lenght/2)-4;
	} else {
		X=x+lround(lenght/2)-7;
	}
	display.setCursor(X,y+8);
	// display.setCursor(x+2+lround(lenght*level/100),y-1); // inBar

	display.printf("%i%%",level);
	// display.setCursor(x+lround(lenght/2)+5,y+8);
	// display.println("%");
}
void displayStatus(boolean color){
	int x = 113;
	int y = 53;
	display.fillRect(x, y, 28, 11, BLACK);
	if (Working){
		// Left Top arrow
		display.drawRect(x+1, y+2, 9, 3, WHITE);
		display.drawPixel(x, y+3, WHITE);
		display.drawPixel(x+2, y+1, WHITE);
		display.drawPixel(x+2, y+5, WHITE);
		display.drawPixel(x+3, y, WHITE);
		display.drawPixel(x+3, y+1, WHITE);
		display.drawPixel(x+3, y+5, WHITE);
		display.drawPixel(x+3, y+6, WHITE);

		// Right Bottom arrow
		display.drawRect(x+5, y+6, 9, 3, WHITE);
		display.drawPixel(x+14, y+7, WHITE);
		display.drawPixel(x+11, y+4, WHITE);
		display.drawPixel(x+11, y+5, WHITE);
		display.drawPixel(x+12, y+5, WHITE);
		display.drawPixel(x+12, y+9, WHITE);
		display.drawPixel(x+11, y+9, WHITE);
		display.drawPixel(x+11, y+10, WHITE);

		display.drawFastHLine(x+2,y+3,7,color ? WHITE : BLACK);
		display.drawFastHLine(x+6,y+7,7,color ? BLACK : WHITE);

		Serial.printf("Status(%d)\n",color);
	}
	// display.setCursor(x,y+13);
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
		display.drawRect(123, 4, 2, 6, WHITE);
		display.drawRect(123, 11, 2, 2, WHITE);
		/* code */
	}
}
void displayNetwork(){
	display.setTextSize(0);
	display.setCursor(0,0);
	display.printf("%s\n",SSID);
	display.print(IP);
	boolean connected = WiFi.status() == WL_CONNECTED ? true : false;
	display.fillRect(99, 0, 20, 16, BLACK);
	if (attenuation >= -80 && connected){
		display.fillRect(99, 12, 4, 4, WHITE);
	} else {
		display.drawRect(99, 12, 4, 4, WHITE);
	}
	if (attenuation >= -73 && connected){
		display.fillRect(104, 8, 4, 8, WHITE);
	} else {
		display.drawRect(104, 8, 4, 8, WHITE);
	}
	if (attenuation >= -67 && connected){
		display.fillRect(109, 4, 4, 12, WHITE);
	} else {
		display.drawRect(109, 4, 4, 12, WHITE);
	}
	if (attenuation >= -60 && connected){
		display.fillRect(114, 0, 4, 16, WHITE);
	} else {
		display.drawRect(114, 0, 4, 16, WHITE);
	}
	if (attenuation <= -80 || !connected){
		display.drawLine(99, 0, 118, 14, WHITE);
		display.drawLine(99, 1, 118, 15, WHITE);
		display.drawLine(99, 2, 118, 16, BLACK);
		display.drawLine(100, 0, 119, 14, BLACK);
	}
}
void displaySensor(){
	display.setCursor(30,SCREEN_BLUE_0+4);
	display.printf("Temp:%.1f`C\n",temperature);
	display.setCursor(30,SCREEN_BLUE_0+4+9);
	display.printf("Press:%.1fhPa\n",pressure);
	display.setCursor(30,SCREEN_BLUE_0+4+18);
	display.printf("Hum:%.1f%%",humidity);
	display.setCursor(30,SCREEN_BLUE_0+4+27);
	display.printf("CO2:%.0fppm",CO2);
}

void init_WiFi (){
	Working = true;
  // Connect to Wi-Fi network with SSID and PWD
	Serial.print("Connecting to wifi ");
	Serial.print(SSID);
	// https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
	WiFi.mode(WIFI_STA);
	WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
	WiFi.setHostname(hostname);
	WiFi.disconnect();
	WiFi.begin(SSID, PWD);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(200);
	}
	// Print local IP address and start web server
	Serial.println("WiFi connected.");
	Working = false;
}
void check_WiFi (boolean log = false){
	unsigned long currentMillis = millis();
// if WiFi is down, try reconnecting
	HostName = WiFi.getHostname();
	IP = WiFi.localIP().toString();
	GW = WiFi.gatewayIP().toString();
	CIDR = (String)(WiFi.subnetCIDR());
	MAC = WiFi.macAddress();
	DNS = WiFi.dnsIP().toString();
	attenuation = WiFi.RSSI();
	if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
		Working = true;
		// IP = "";
		// GW = "";
		// CIDR = "";
		// DNS = "";
		// attenuation = "";

		Serial.print(millis());
		Serial.println("Reconnecting to WiFi...");
		WiFi.disconnect();
		WiFi.reconnect();
		previousMillis = currentMillis;

		HostName = WiFi.getHostname();
		IP = WiFi.localIP().toString();
		GW = WiFi.gatewayIP().toString();
		CIDR = (String)(WiFi.subnetCIDR());
		MAC = WiFi.macAddress();
		DNS = WiFi.dnsIP().toString();
		attenuation = WiFi.RSSI();

		Working = false;
	}

	if (log){
		Serial.printf("Network:");
		Serial.print(" ");Serial.print(HostName);
		Serial.print(" ");Serial.print(IP);
		Serial.print(" ");Serial.print(GW);
		Serial.print(" ");Serial.print(CIDR);
		Serial.print(" ");Serial.print(MAC);
		Serial.print(" ");Serial.print(DNS);
		Serial.printf(" %i dBm\n",attenuation);
	}
}
String getJSON() {
	String json ="{";
		json += "\"Network\": {\"hostname\": \""+String(HostName)+"\",\"SSID\": \""+String(SSID)+"\",\"MAC\": \""+String(MAC)+"\",\"IP\": \""+String(IP)+"\",\"CIDR\": \""+String(CIDR)+"\",\"Gateway\": \""+String(GW)+"\",\"DNS\": \""+String(DNS)+"\",\"Strength\": {\"value\":"+String(attenuation)+",\"unit\": \"dBm\"}},";
		json += "\"Temperature\": {\"value\":"+String(temperature)+",\"unit\": \"°C\"},";
		json += "\"Pressure\": {\"value\":"+String(pressure)+",\"unit\": \"hPa\"},";
		json += "\"Humidity\": {\"value\":"+String(humidity)+",\"unit\": \"%\"},";
		json += "\"CO2\": {\"value\":"+String(CO2)+",\"unit\": \"ppm\"},";
		json += "\"Battery\": {\"Voltage\": {\"value\":"+String(BatVoltage)+",\"unit\": \"V\"},\"Capacity\": {\"value\":"+String(BatCapacity)+",\"unit\": \"%\"}},";
		json += "\"Power\": {\"value\":"+String(PowerVoltage)+",\"unit\": \"V\"}}";
	return json;
}
void ApiJson(){
	Working = true;
	Serial.printf("API Reply JSON %d\n",Working);
	delay(2000);
	server.send(200, "application/json", getJSON());
	Working = false;
	Serial.printf("API Reply JSON %d\n",Working);
}
void postDataToServer() {
	// Block until we are able to connect to the WiFi access point
	if (WiFi.status() == WL_CONNECTED) {
		Serial.println("Posting JSON data to server http://rpimaster/probe");

		WiFiClient Wclient;  // or WiFiClientSecure for HTTPS
		HTTPClient httpClient;

		// Send request
		httpClient.begin(Wclient, SrvPostData);
		Serial.printf("POST(JSON) to %s > ",SrvPostData);
		httpClient.addHeader("Content-Type", "application/json");
		int httpResponseCode = httpClient.POST(getJSON()); // 

		// Read response
		Serial.println(httpResponseCode);
		Serial.println(httpClient.getString());

		// Disconnect
		httpClient.end();
		// delay(1000);
	}
}

void getBatteryCapacity(){
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
	while (BatVoltage < remainingCapacity[i].voltage && i < step) {
		i++;
	}
	BatCapacity = remainingCapacity[i].capacity;
	// Serial.printf("Pourcentage Battery: %i%%\n",BatCapacity);
}
void getBatteryVoltage(){
	// Vbat = Vread * (R1 + R2) / R2
	float Vread = ((float)analogRead(BATTERY_PIN) / 4096 * 3.3);
	//   Serial.printf("TensionPin(35): %f\n",Vread);
	BatVoltage =  Vread*(20120+9670)/20120;
	// Serial.printf("Tension Battery: %fV\n",BatVoltage);
}
void getSensorData(void * parameter) {
	for (;;) {
		Working = true;
		Serial.printf("Reading Sensors %d\n",Working);
		temperature = 8.3; // bme.readTemperature();
		humidity = 56.2; // bme.readHumidity();
		pressure = 1003.5; // bme.readPressure() / 100;
		CO2 = 415.5;
		getBatteryVoltage();
		getBatteryCapacity();
		xSemaphoreTake( mutex, portMAX_DELAY );
			// read CO2 SPi or I2C
		xSemaphoreGive( mutex );
		postDataToServer();
		Working = false;
		
		vTaskDelay( pdMS_TO_TICKS( DELAY_READ_PROBES*1000 ) );
		Serial.printf("Reading Sensors. %d\n",Working);
	}
}

void handlePost() {
  if (server.hasArg("plain") == false) {
	  Serial.println("");
  }

  String body = server.arg("plain");
  Serial.println(body);
  deserializeJson(jsonDocument, body);
//   Serial.println(jsonDocument);

  // Respond to the client
  server.send(200, "application/json", "{\"OK\"}");
}
// setup API resources
void setup_routing() {
	if (WiFi.status() == WL_CONNECTED){
		server.on("/", ApiJson);
		server.on("/msg", HTTP_POST, handlePost);
		Serial.println("setup_routing()");
		// start server
		server.begin();
	} else {
		Serial.println("Network is Missing!");
	}
}
void isWorking(void * parameter){
	for (;;) {
			// digitalWrite(LD1, OnOff);
			OnOff = !OnOff;
			// Max LED brightness 0, Min 255
			for(int dutyCycle = 253; dutyCycle >= 140; dutyCycle-=2+Working*2){
				ledcWrite(LD1CHANNEL, dutyCycle);   
				vTaskDelay(3);
			}
			OnOff = !OnOff;
			for(int dutyCycle = 140; dutyCycle <= 250; dutyCycle+=2+Working*2){   
				ledcWrite(LD1CHANNEL, dutyCycle);
				vTaskDelay(6);
			}
			OnOff = !OnOff;
			for(int dutyCycle = 250; dutyCycle >= 100; dutyCycle-=2+Working*2){
				ledcWrite(LD1CHANNEL, dutyCycle);   
				vTaskDelay(3);
			}
			OnOff = !OnOff;
			for(int dutyCycle = 100; dutyCycle <= 253; dutyCycle+=2+Working*2){   
				ledcWrite(LD1CHANNEL, dutyCycle);
				vTaskDelay(6);
			}
			OnOff = !OnOff;
		vTaskDelay( (100));
	}
}
// void waitHttpRequest(void * parameter){
// 	setup_routing();
// 	for (;;) {
// 		server.handleClient();
// 		vTaskDelay(500 / portTICK_PERIOD_MS);
// 	}
// }

void manageScreen(void * parameter){
	// init_Screen();
	for (;;) {
		display.clearDisplay();
		displayBatteryLevel(BatCapacity);
		displayNetwork();
		displaySensor();
		display.drawBitmap(0, SCREEN_BLUE_0,  logoNB, 25, 48, true);
		if (Working || (!Working && OnOff)){
			displayStatus(OnOff);
			// Serial.printf("Working = %d\n",Working);
			xSemaphoreTake( mutex, portMAX_DELAY );
				display.display();
			xSemaphoreGive( mutex );
			vTaskDelay( pdMS_TO_TICKS(25));
		} else {
			// Serial.print(".");
			xSemaphoreTake( mutex, portMAX_DELAY );
				display.display();
			xSemaphoreGive( mutex );
			vTaskDelay( pdMS_TO_TICKS(25));
		}
		// delay the task
	}
}
void setup_task() {
	xTaskCreatePinnedToCore(
		manageScreen,
		"xTask acces to screen",    // Name of the task (for debugging)
		20000,             // Stack size (bytes)
		NULL,            // Parameter to pass
		6,               // Task priority
		NULL,             // Task handle
		1
	);
	xTaskCreatePinnedToCore(
		getSensorData,
		"Read sensor data and put on server.",   // Name of the task (for debugging)
		20000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		5,               // Task priority
		NULL,             // Task handle
		1
	);
	// xTaskCreate(
	// 	waitHttpRequest,
	// 	"Attend les requetes API",    // Name of the task (for debugging)
	// 	10000,             // Stack size (bytes)
	// 	NULL,            // Parameter to pass
	// 	1,               // Task priority
	// 	NULL             // Task handle
	// );
	xTaskCreatePinnedToCore(
		isWorking,
		"Toogle LD1 on working",    // Name of the task (for debugging)
		1000,             // Stack size (bytes)
		NULL,            // Parameter to pass
		5,               // Task priority
		NULL,             // Task handle
		0
	);

	Serial.println("setup_task()");
}

void setup() {
	Serial.begin(115200);
	Working = true;
	mutex = xSemaphoreCreateMutex();

	pinMode(BATTERY_PIN, INPUT);
	// pinMode(LD1, OUTPUT);
	// configure LED PWM functionalitites
	ledcSetup(LD1CHANNEL, FREQ, RESOLUTION);
	// attach the channel to the GPIO to be controlled
	ledcAttachPin(LD1, LD1CHANNEL);

	// Sensor setup
	// if (!bme.begin(0x76)) {
	//   Serial.println("Problem connecting to BME280");
	// }
	init_Screen();
	setup_task();
	init_WiFi();
	setup_routing();

	Serial.println("setup()");
	Working = false;
}

void loop() {
	// Serial.println("loop(start)");
	check_WiFi();
	server.handleClient();
	// manageScreen(NULL);
	delay(500);
}