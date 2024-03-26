#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#if !defined ROUTESNODEHANDLER_H
#define ROUTESNODEHANDLER_H
void handlerNodeServeIndexHTML(AsyncWebServerRequest *request);

extern AsyncCallbackJsonWebHandler *handlerNodeConfigRedMesh;
#endif