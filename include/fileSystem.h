#pragma once

void initSPIFFS();
// char *readFile(fs::FS &fs, const char * path);
String readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void loadJsonSettings(const char *path);
