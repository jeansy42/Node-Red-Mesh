#include <Arduino.h>
#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "LittleFS.h"
#include "auxiliars.h"
#include "meshManager.h"

#define MESH_PREFIX "Soluforce"
#define MESH_PASSWORD "soluforcesenha"
#define MESH_PORT 5555

uint32_t rootId;
int actionerPin = D4;
bool shouldReinit = false;

painlessMesh meshNode;
Scheduler userScheduler;

Task taskWaitingForConfigurations(TASK_SECOND * 5, TASK_FOREVER, &waitingForConfiguratios);
Task taskSendActionerInformation(TASK_SECOND, TASK_FOREVER, &sendActionerInfo);
Task taskVerifyIfShouldReinit(TASK_SECOND * 3, TASK_FOREVER, &verifyIfShouldReinit);

void receivedCallback(uint32_t from, String &msg);

void setup()
{

  Serial.begin(115200);
  if (!initLittleFS())
    return;
  /* bool state = LittleFS.format();
  if (state)
    Serial.println("LittleFS formatado corretamente");
  else
    return; */
  createArchiveConfigJSON();
  meshNode.setDebugMsgTypes(ERROR | STARTUP);
  meshNode.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  meshNode.onReceive(&receivedCallback);
  meshNode.setContainsRoot(true);

  // Agregando tarefas
  userScheduler.addTask(taskWaitingForConfigurations);
  userScheduler.addTask(taskSendActionerInformation);
  userScheduler.addTask(taskVerifyIfShouldReinit);

  taskVerifyIfShouldReinit.enable();

  if (isConfigJsonEmpty())
  {
    taskWaitingForConfigurations.enable();
  }
  else
  {
    startConfiguration();
  };
}

void loop()
{
  meshNode.update();
}

void receivedCallback(uint32_t from, String &msg)
{
  if (msg == "root")
    rootId = from;
  else
  {
    JsonDocument doc;
    deserializeJson(doc, msg);
    if (doc.as<JsonObject>().containsKey("config"))
    {
      Serial.println("Recebendo configuracoes");
      JsonArray config = doc["config"].as<JsonArray>();
      setArchiveConfigJSON(config);
    }
    else if (doc.containsKey("actioner"))
    {
      JsonObject obj = doc["actioner"];
      if (obj.containsKey("setStatus"))
      {
        int status = obj["setStatus"];
        digitalWrite(actionerPin, status);
      }
    }
  }
};
