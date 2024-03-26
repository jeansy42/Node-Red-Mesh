#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include "auxiliars.h"

extern fs::FS nodeFilesystem;

void handlerNodeServeIndexHTML(AsyncWebServerRequest *request)
{
    request->send(nodeFilesystem, "/index.html", "text/html", false);
}

AsyncCallbackJsonWebHandler *handlerNodeConfigRedMesh = new AsyncCallbackJsonWebHandler("/", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                        {
    JsonObject jsonObj = json.as<JsonObject>();
    if (jsonObj.containsKey("ssid") && jsonObj.containsKey("password") && jsonObj.containsKey("port")){
        String res=configNodeRedMesh(jsonObj);
        request->send(200,"application/json",res);
        }else{
        request->send(400, "application/json", "{\"error\":\"Bad request, ssid, password and port are required.\"}");
    } });