#include "painlessMesh.h"
#include "ArduinoJson.h"

extern painlessMesh meshNode;
extern int actionerPin;
extern uint32_t rootId;

void sendActionerInfo()
{
    JsonDocument doc;
    JsonObject obj=doc["actioner"].to<JsonObject>();
    String msg;
    int status = digitalRead(actionerPin);
    obj["status"] = status;
    serializeJson(doc, msg);
    meshNode.sendSingle(rootId, msg);
    Serial.printf("Enviando informacao ao root %d\n", status);
}
