#include <Arduino.h>

#include "SPIFFS.h"

#include "global.h"

#include "fileSystem.h"

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}


// Read File from SPIFFS
// char *readFile(fs::FS &fs, const char * path){
// 	Serial.printf("Reading file: %s\r\n", path);
// 	File file = fs.open(path);
// 	if(!file || file.isDirectory()){
// 		Serial.println("- failed to open file for reading");
// 		return "";
// 	}
// 	char NL = 10;
// 	char * fileContent;
// 	while(file.available()){
// 		fileContent = (char*)file.readStringUntil(NL);
// 		break;     
// 	}
// 	Serial.println("File content :");
// 	Serial.println(fileContent);// 	return fileContent;
// }
// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
	Serial.printf("Reading file: %s\r\n", path);

	File file = fs.open(path);
	if(!file || file.isDirectory()){
		Serial.println("> failed to open file for reading");
		return String();
	}
	
	String fileContent;
	while(file.available()){
		fileContent = file.readStringUntil('\n');
		break;     
	}
    // Serial.println("File content :");
    // Serial.println(fileContent.c_str());
	return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("> failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("> file written");
    } else {
        Serial.println("> file write failed");
    }
}
void loadJsonSettings(const char *path){
	std::string SettingsJson = readFile(SPIFFS, path).c_str();

	if(SettingsJson.length() > 2 ){
		if((SettingsJson[0]=='{' && SettingsJson[SettingsJson.length()-1]=='}') || (SettingsJson[0]=='[' && SettingsJson[SettingsJson.length()-1]==']')){
			Serial.printf("Loading SettingsJson [%i]\n",SettingsJson.length());
			CurrentProbe.Settings = SETTINGS::fromJson(SettingsJson);
			// Serial.println(CurrentProbe.Settings.toJson().c_str());
		} else {
			Serial.println(SettingsJson.c_str());
			Serial.println("====== WRONG JSON, Removing file ! ======");
			SPIFFS.remove(path);
		}
	} else {
		Serial.println("====== MISSING JSON ! ======");
	}
}

