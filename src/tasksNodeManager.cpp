#include "Arduino.h"
#include "ArduinoJson.h"
#include "painlessMesh.h"
#include "tasksNodeManager.h"

extern painlessMesh meshNode;
extern int actionerPin;
extern int doorSensor;
extern int doorSensorState;
extern uint32_t rootId;
extern bool shouldReinit;

void waitingForConfiguratios()
{
    Serial.println("Aguardando configuracoes....");
};

void sendActionerInfo()
{
    JsonDocument doc;
    JsonObject obj = doc["actioner"].to<JsonObject>();
    String msg;
    int status = digitalRead(actionerPin);
    obj["status"] = status;
    serializeJson(doc, msg);
    meshNode.sendSingle(rootId, msg);
    Serial.printf("Enviando informacao ao root %d\n", status);
}

void verifyIfShouldReinit()
{
    if (shouldReinit)
    {
        Serial.println("Reiniciando no....");
        ESP.restart();
    }
}

void getDoorSensorInfo()
{
    int currentState = digitalRead(doorSensor);
    if (currentState != doorSensorState)
    {
        doorSensorState = currentState;
        taskSendDoorSensorInfo.enable();
    }
}
void sendDoorSensorInfo()
{
    JsonDocument msg;
    JsonObject obj = msg["doorSensor"].to<JsonObject>();
    obj["status"] = doorSensorState;
    meshNode.sendSingle(rootId, msg.as<String>());
    Serial.printf("Enviando informacao do sensor de porta ao root %d\n", doorSensorState);
}

Task taskWaitingForConfigurations(TASK_SECOND * 5, TASK_FOREVER, &waitingForConfiguratios);
Task taskSendActionerInformation(TASK_SECOND, TASK_FOREVER, &sendActionerInfo);
Task taskVerifyIfShouldReinit(TASK_SECOND * 3, TASK_FOREVER, &verifyIfShouldReinit);

// Sensor de porta
Task taskGetDoorSensorInfo(TASK_SECOND, TASK_FOREVER, &getDoorSensorInfo);
Task taskSendDoorSensorInfo(TASK_SECOND, TASK_FOREVER, &sendDoorSensorInfo);
