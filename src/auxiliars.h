#if !defined(AUXILIARS_H)
#define AUXILIARS_H

void createArchiveConfigJSON();
void verifyIfShouldReinit();
bool setArchiveConfigJSON(JsonArray array);
String sendJsonResponseFromFile(String path);
void waitingForConfiguratios();
void startConfiguration();
bool isConfigJsonEmpty();
bool initLittleFS();
void listDir(const char *dirname);
bool deleteFile(String path);

#endif // AUXILIARS_H
