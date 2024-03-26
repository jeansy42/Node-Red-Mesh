#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include "painlessMesh.h"
#include "ArduinoJson.h"
#include "LittleFS.h"
#include "auxiliars.h"
#include "meshManager.h"
#include "tasksNodeManager.h"
#include "routesNodeHandler.h"

String nodeSsid;
String nodePassword;
uint16_t nodePort;

uint32_t rootId;
int actionerPin = D4;
int doorSensor = D1;
int doorSensorState;
bool shouldReinit = false;
bool nodeRedMeshConfigState;
fs::FS nodeFilesystem = LittleFS;

painlessMesh meshNode;
Scheduler userScheduler;
AsyncWebServer nodeServer(80);

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
  createNodeConfigMeshIfNotExists(&nodeFilesystem);
  nodeRedMeshConfigState = isNodeRedMeshConfig();
  createArchiveConfigJSON(&nodeFilesystem);

  if (nodeRedMeshConfigState)
  {
    meshNode.setDebugMsgTypes(ERROR | STARTUP);
    meshNode.init(nodeSsid, nodePassword, &userScheduler, nodePort);
    meshNode.onReceive(&receivedCallback);

    meshNode.setContainsRoot(true);

    // Agregando tarefas

    IPAddress myAPIP = meshNode.getAPIP();
    Serial.println("My AP IP is " + myAPIP.toString());

    if (isConfigJsonEmpty(&nodeFilesystem))
    {
      userScheduler.addTask(taskWaitingForConfigurations);
      taskWaitingForConfigurations.enable();
    }
    else
    {
      userScheduler.addTask(taskSendActionerInformation);
      userScheduler.addTask(taskGetDoorSensorInfo);
      userScheduler.addTask(taskSendDoorSensorInfo);
      startConfiguration(&nodeFilesystem);
    };
  }
  else
  {
    Serial.println("Setting AP (Access Point)");
    WiFi.softAP("ESP-NodeRedMesh-Manager", NULL);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }

  // Ruta para configuracão da red mesh no nó
  nodeServer.on("/", HTTP_GET, handlerNodeServeIndexHTML);
  nodeServer.addHandler(handlerNodeConfigRedMesh);
  nodeServer.serveStatic("/", nodeFilesystem, "/");
  nodeServer.begin();

  userScheduler.addTask(taskVerifyIfShouldReinit);
  taskVerifyIfShouldReinit.enable();
}

void loop()
{
  if (nodeRedMeshConfigState)
  {
    meshNode.update();
  }
  else
  {
    userScheduler.execute();
  }
}
