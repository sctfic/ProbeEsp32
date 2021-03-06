
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "global.h"
#include "sensors.h"

//OLED pins
// #define I2C_SDA 5
// #define I2C_SCL 4
// #define OLED_RST 15 // 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_BLUE_0 16 // OLED blue area start

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, i2cbus.RST); // , 120000UL, 120000UL


const unsigned char degree [] PROGMEM = {0xc0,0xc0};
const unsigned char pow2 [] PROGMEM = {0xc0,0x20,0x40,0x80,0xe0};
const unsigned char pow3 [] PROGMEM = {0xc0,0x20,0x40,0x20,0xc0};
 // 'logoNB', 23x48px
const unsigned char logoNB [] PROGMEM = {
	0x00, 0x20, 0x00,
	0x00, 0x20, 0x00,
	0x00, 0x30, 0x00,
	0x00, 0x30, 0x00,
	0x00, 0x30, 0x00,
	0x00, 0x30, 0x00,
	0x00, 0x70, 0x00, 
	0x00, 0x70, 0x00, 
	0x00, 0x50, 0x00, 
	0x00, 0x50, 0x00, 
	0x00, 0xb0, 0x00, 
	0x01, 0xa0, 0x00, 
	0x01, 0x20, 0x00, 
	0x02, 0x40, 0x00, 
	0x06, 0x40, 0x00, 
	0x04, 0x40, 0x80, 
	0x0c, 0x40, 0x80,
	0x08, 0x40, 0xc0,
	0x08, 0x40, 0xe0,
	0x08, 0x60, 0xa0,
	0x10, 0x30, 0xb0,
	0x10, 0x1f, 0x10,
	0x18, 0x10, 0x10, 
	0x08, 0x30, 0x10, 
	0x08, 0x38, 0x10, 
	0x0c, 0x28, 0x20, 
	0x04, 0x28, 0x20, 
	0x04, 0x48, 0x20, 
	0x06, 0x4c, 0x20, 
	0x42, 0x44, 0x20, 
	0x62, 0x84, 0x40, 
	0xf2, 0x82, 0x40, 
	0x9d, 0x02, 0x40,
	0x81, 0x01, 0x60,
	0xc2, 0x01, 0x32,
	0x42, 0x00, 0x8e,
	0x64, 0x00, 0x82,
	0x34, 0x00, 0x42,
	0x18, 0x38, 0x42, 
	0x08, 0x6c, 0x24, 
	0x08, 0x44, 0x24, 
	0x08, 0x44, 0x28, 
	0x08, 0x44, 0x28, 
	0x0c, 0x38, 0x70, 
	0x04, 0x00, 0x40, 
	0x06, 0x00, 0xc0, 
	0x03, 0x83, 0x80, 
	0x00, 0xfe, 0x00
};

#define BLACK 0
#define WHITE 1

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define NONE 0

void refreshScreen(){
    xSemaphoreTake( mutex, portMAX_DELAY );
    display.display();
	// Serial.println("# refreshScreen()! ############################################################################### ");
    xSemaphoreGive( mutex );
}
void init_Screen (){
  //initialize OLED
  pinMode(i2cbus.OLED, OUTPUT);
  Wire.begin(i2cbus.SDA, i2cbus.SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
	Serial.println(F("SSD1306 allocation failed"));
	for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  Serial.println("> Screen Enabled!");
}
void displayProgressbar (char level, char x = 14, char y = 28, char lenght = 100, char DisplayValue = NONE){
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

void displayTransfert(bool color){
	// les 2 petites fleches en bas
	int x = 113;
	int y = 53;
	if (Transfert){
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
	} else {
		display.fillRect(x, y, 28, 11, BLACK);
	}
}
void displayBatteryLevel(int batteryLevel){
	display.fillRect(120, 0, 8, 16, BLACK);
	display.drawPixel(123, 0, WHITE);
	display.drawPixel(124, 0, WHITE);
	display.drawRect(120, 1, 8, 15, WHITE);
	double vPow = getPowerVoltage();
	double vBat = getBatteryVoltage();
	if (vPow > vBat && vPow < 4.7) {
	// if (CurrentProbe.Energy.PowerSupply.Voltage.Value() > CurrentProbe.Energy.Battery.Voltage.Value() && CurrentProbe.Energy.Battery.Voltage.Value() > 2) {
		charging = (charging+4)%100;
		display.fillRect(122, 3+lround((100-charging)*0.11), 4, lround((charging)*0.11), WHITE);
	} else if(batteryLevel > 0) {
		// trace le rectangle interieur du niveau de batterie
		display.fillRect(122, 3+lround((100-batteryLevel)*0.11), 4, lround((batteryLevel)*0.11), WHITE);
	} else {
		// trace une croix
  		display.drawLine(121, 4, 126, 12, WHITE);
  		display.drawLine(121, 12, 126, 4, WHITE);
	}
}
void displaySignal(int x){
	display.fillRect(99, 0, 20, 16, BLACK);
	if (x >= -80 && WIFI_CONNECTED){
		display.fillRect(99, 12, 4, 4, WHITE);
	} else {
		display.drawRect(99, 12, 4, 4, WHITE);
	}
	if (x >= -73 && WIFI_CONNECTED){
		display.fillRect(104, 8, 4, 8, WHITE);
	} else {
		display.drawRect(104, 8, 4, 8, WHITE);
	}
	if (x >= -67 && WIFI_CONNECTED){
		display.fillRect(109, 4, 4, 12, WHITE);
	} else {
		display.drawRect(109, 4, 4, 12, WHITE);
	}
	if (x >= -60 && WIFI_CONNECTED){
		display.fillRect(114, 0, 4, 16, WHITE);
	} else {
		display.drawRect(114, 0, 4, 16, WHITE);
	}
	if (x <= -80 || !WIFI_CONNECTED){
		display.drawLine(99, 0, 118, 14, WHITE);
		display.drawLine(99, 1, 118, 15, WHITE);
		display.drawLine(99, 2, 118, 16, BLACK);
		display.drawLine(100, 0, 119, 14, BLACK);
	}
}
void displayNetwork(){
	display.setTextSize(0);
	display.setCursor(0,0);
	display.println(CurrentProbe.Network.Wifi.SSID.c_str());
	display.print(CurrentProbe.Network.IP.c_str());
	displaySignal(CurrentProbe.Network.Strength.Value());
}
void displayASCII(char * str){
	// Serial.printf("%s = ",str);
	for (int i = 0; i < strlen(str); i++){
		switch ((int)str[i]) {
		case 194:
		case 195:
		// Serial.print("utf8");
			break;
		case 176:
		// Serial.printf("%i*",(int)str[i]);
			display.drawBitmap(display.getCursorX(), display.getCursorY(),  degree, 2, 2, true);
			display.setCursor(display.getCursorX()+3, display.getCursorY());
			break;
		case 178: // ^2
		// Serial.printf("%i*",(int)str[i]);
			display.drawBitmap(display.getCursorX(), display.getCursorY(),  pow2, 3, 5, true);
			display.setCursor(display.getCursorX()+4, display.getCursorY());
			break;
		case 179: // ^3
		// Serial.printf("%i*",(int)str[i]);
			display.drawBitmap(display.getCursorX(), display.getCursorY(),  pow3, 3, 5, true);
			display.setCursor(display.getCursorX()+4, display.getCursorY());
			break;
		default:
		// Serial.printf("%i.",(int)str[i]);
			display.write((int)str[i]);
			break;
		}
	// Serial.print(" ");
	}
	// Serial.println("");
}
void displaySensor(){
	display.setCursor(28,SCREEN_BLUE_0+4);
		display.print("Temp:");
	display.setCursor(64,SCREEN_BLUE_0+4);
		displayASCII((char*)CurrentProbe.Probe.Temperature.toString().c_str());
	display.setCursor(28,SCREEN_BLUE_0+4+9);
		display.print("Press:");
	display.setCursor(64,SCREEN_BLUE_0+4+9);
		displayASCII((char*)CurrentProbe.Probe.Pressure.toString().c_str());
	display.setCursor(28,SCREEN_BLUE_0+4+9*2);
		display.print("Humid:");
	display.setCursor(64,SCREEN_BLUE_0+4+9*2);
		displayASCII((char*)CurrentProbe.Probe.Humidity.toString().c_str());
	display.setCursor(28,SCREEN_BLUE_0+4+9*3);
		display.print("CO2:");
	display.setCursor(64,SCREEN_BLUE_0+4+9*3);
		displayASCII((char*)CurrentProbe.Probe.CO2.toString().c_str());
	display.setCursor(28,SCREEN_BLUE_0+4+9*4);
		display.print("Lux:");
	display.setCursor(64,SCREEN_BLUE_0+4+9*4);
		displayASCII((char*)CurrentProbe.Probe.LUX.toString().c_str());

	// display.setCursor(30,SCREEN_BLUE_0+4+27);
	// display.printf("Hum:%.1f%%",CurrentProbe.Probe.Humidity.Raw);
	// display.printf("CO2:%.1fppm",CurrentProbe.Probe.CO2.Raw);
	// display.setCursor(30,SCREEN_BLUE_0+4+36);
	// display.printf("VBat:%.2fV",CurrentProbe.Energy.Battery.Voltage.Raw);
}
void displayDeepSleep(){
	if (CurrentProbe.Settings.DisplayDuringDeepSleep){
		display.fillRect(0, 0, 98, 16, BLACK);
		display.setCursor(0, 0);
		display.println("DeepSleep");
		display.print(" Low Power...");
		displaySignal(-100);
		display.fillRect(100, 53, 28, 11, BLACK);
		display.setCursor(116, 56);
		display.print("z");
		display.setCursor(122, 54);
		display.print("Z");
	} else {
		display.clearDisplay();
	}
	refreshScreen();
}
void redrawScreen(){
    display.clearDisplay();
    displayNetwork();
    displaySensor();
    display.drawBitmap(0, SCREEN_BLUE_0,  logoNB, 23, 48, true);
}
void manageScreen(void * parameter){
	init_Screen();
	char i = 100;
	if (esp_sleep_get_wakeup_cause()!=ESP_SLEEP_WAKEUP_TIMER){
		display.print("Booting...");
	} else {
		display.print("WakeUp...");
	}
	while (i-=2) {
		displayProgressbar(100-i); // 10 fois / sec
		refreshScreen();
		vTaskDelay(19);
	}
	for (;;) {
		i = 10;
		redrawScreen(); // 1000ms
		// Serial.printf("OnOff = %d - ",OnOff);
		// Serial.printf("Working = %d - ",Working);
		// Serial.printf("DeepSleepNow = %d\n",DeepSleepNow); // DeepSleepNow = uploadData
		// Serial.printf("%d || (%d && %d)\n",Working, !Working, OnOff);
		while (i--) {
		    displayBatteryLevel(CurrentProbe.Energy.Battery.Capacity.Value());
			displayTransfert(OnOff); // 100ms
			refreshScreen();
			vTaskDelay(100);
		}
	}
}