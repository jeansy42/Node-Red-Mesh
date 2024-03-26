#if !defined(AUXILIARS_H)
#define AUXILIARS_H

void createArchiveConfigJSON(fs::FS *filesystem);
String readArchiveConfigJSON();
bool setArchiveConfigJSON(JsonArray array, fs::FS *filesystem);
bool isConfigJsonEmpty(fs::FS *filesystem);

void createNodeConfigMeshIfNotExists(fs::FS *filesystem);
bool isNodeRedMeshConfig();
String configNodeRedMesh(JsonObject obj);

String sendJsonResponseFromFile(String path, fs::FS *filesystem);
void startConfiguration(fs::FS *filesystem);
bool initLittleFS();
void listDir(const char *dirname);
bool deleteFile(String path);

#endif // AUXILIARS_H
