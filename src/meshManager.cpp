#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "auxiliars.h"
#include "tasksNodeManager.h"

extern painlessMesh meshNode;
extern int actionerPin;
extern uint32_t rootId;
extern fs::FS nodeFilesystem;

void receivedCallback(uint32_t from, String &msg)
{

    if (msg == "root")
        rootId = from;
    else if (msg == "validateConfigurations")
    {
        JsonDocument docConfig;
        JsonDocument docRes;
        DeserializationError error = deserializeJson(docConfig, readArchiveConfigJSON());
        if (error)
            Serial.println("Erro ao deserializar o arquivo config.json");
        else
        {
            JsonArray array = docRes["validateConfigOk"].to<JsonArray>();
            for (JsonObject obj : docConfig.as<JsonArray>())
            {
                array.add(obj);
            };
            Serial.println("Enviando configuracao ao hub para verificacao.");
            meshNode.sendSingle(from, docRes.as<String>());
        }
    }
    else
    {
        JsonDocument doc;
        deserializeJson(doc, msg);
        if (doc.containsKey("config"))
        {
            Serial.println("Recebendo configuracoes");
            JsonArray config = doc["config"].as<JsonArray>();
            setArchiveConfigJSON(config, &nodeFilesystem);
        }
        else if (doc.containsKey("actioner"))
        {
            JsonObject obj = doc["actioner"];
            if (obj.containsKey("setStatus"))
            {
                int status = obj["setStatus"];
                digitalWrite(actionerPin, status);
                taskSendActionerInformation.enable();
            }
            else if (obj.containsKey("getInfo"))
            {
                obj["getInfo"].as<String>() == "off" && taskSendActionerInformation.disable();
            }
        }
        else if (doc.containsKey("doorSensor"))
        {
            JsonObject obj = doc["doorSensor"];
            obj["getInfo"].as<String>() == "off" && taskSendDoorSensorInfo.disable();
        }
    }
};