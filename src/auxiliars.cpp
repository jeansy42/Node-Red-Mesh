#include "LittleFS.h"
#include "FS.h"
#include "ArduinoJson.h"
#include "painlessMesh.h"
#include "tasksNodeManager.h"

extern bool shouldReinit;
extern uint32_t rootId;
extern int actionerPin;
extern int doorSensor;
extern int doorSensorState;
extern painlessMesh meshNode;
extern fs::FS nodeFilesystem;
extern String nodeSsid;
extern String nodePassword;
extern uint16_t nodePort;

void createArchiveConfigJSON(fs::FS *filesystem)

{
    const char *path = "/config.json";
    if (!filesystem->exists(path))
    {

        Serial.printf("Criando arquivo: %s", path);
        File file = filesystem->open(path, "w");
        if (!file)
        {
            Serial.println("Erro ao criar o arquivo");
        }
        else
        {
            Serial.printf("%s criado com sucesso \n", path);
        }
        file.close();
    }
}
void createNodeConfigMeshIfNotExists(fs::FS *filesystem)
{
    const char *path = "/configMesh.json";
    if (!filesystem->exists(path))
    {
        Serial.printf("Criando arquivo: %s\n", path);
        File file = filesystem->open(path, "w");
        if (!file)
        {
            Serial.printf("Erro ao criar o arquivo %s\n", path);
            return;
        }
        JsonDocument doc;
        doc["ssid"] = "";
        doc["password"] = "";
        doc["port"] = "";
        serializeJson(doc, file);
        Serial.printf("%s criado com sucesso \n", path);
        file.close();
    }
    else
        Serial.printf("O arquivo %s ja existe\n", path);
}
bool isNodeRedMeshConfig()
{
    const char *path = "/configMesh.json";
    createNodeConfigMeshIfNotExists(&nodeFilesystem);
    File file = nodeFilesystem.open(path, "r");
    if (!file)
    {
        Serial.printf("Erro ao abrir o arquivo %s\n", path);
        return false;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error)
    {
        Serial.printf("Erro ao deserializar o arquivo %s\n", path);
        return false;
    }
    if (doc["ssid"].as<String>() == "" || doc["password"].as<String>() == "" || doc["port"].as<String>() == "")
    {
        Serial.println("Configuracao da red mesh nao achada \n");
        return false;
    }
    nodeSsid = doc["ssid"].as<String>();
    nodePassword = doc["password"].as<String>();
    nodePort = doc["port"].as<uint16_t>();
    Serial.println("Configuracao da red mesh estabelecida com sucesso \n");
    return true;
}
String configNodeRedMesh(JsonObject obj)
{
    const char *path = "/configMesh.json";
    JsonDocument docRes;
    createNodeConfigMeshIfNotExists(&nodeFilesystem);
    File file = nodeFilesystem.open(path, "w");
    if (!file)
    {
        Serial.printf("Erro ao abrir o arquivo %s\n", path);
        docRes["status"] = "error";
        docRes["msg"] = "Erro ao abrir o arquivo";
    }
    else
    {
        JsonDocument doc;
        doc["ssid"] = obj["ssid"];
        doc["password"] = obj["password"];
        doc["port"] = obj["port"];
        serializeJson(doc, file);
        file.close();
        docRes["status"] = "success";
        docRes["msg"] = "Configuracao da red mesh feita com sucesso";
        Serial.println("Reiniciando dispositivo....");
        shouldReinit = true;
    }
    return docRes.as<String>();
}

String sendJsonResponseFromFile(String path, fs::FS *filesystem)
{
    if (!filesystem->exists(path))
    {
        JsonDocument doc;
        doc.to<JsonArray>();
        String emptyArray = doc.as<String>();
        return emptyArray;
    }
    JsonDocument docRes;
    File file = filesystem->open(path, "r");
    deserializeJson(docRes, file);
    String response = docRes.as<String>();
    file.close();
    return response;
}

void setArchiveConfigJSON(JsonArray array, fs::FS *filesystem)
{
    String path = "/config.json";
    if (!filesystem->exists(path))
    {
        Serial.printf("O arquivo na ruta %s no existe", path.c_str());
        createArchiveConfigJSON(&nodeFilesystem);
    };
    File file = filesystem->open(path, "w");
    if (!file)
    {
        Serial.printf("Error ao abrir o arquivo na ruta %s", path.c_str());
    }
    else
    {
        JsonDocument doc;
        for (JsonObject obj : array)
        {
            doc.add(obj);
        };
        serializeJson(doc, file);
        file.close();
        Serial.println("Configuracao atualizada com sucesso.");
        Serial.println("Enviando confirmação ao hub...");

        shouldReinit = meshNode.sendSingle(rootId, "configOk");
        if (shouldReinit)
            Serial.println("--->Confirmacao ao hub enviada com sucesso");
        else
            Serial.println("--->Ocurreu algum erro ao enviar a confirmacao ao hub");
    };
};

String readArchiveConfigJSON()
{
    const char *path = "/config.json";
    File file = nodeFilesystem.open(path, "r");
    if (!file)
    {
        Serial.println("Falho ao abrir o arquivo de configuracao.");
        return "";
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error)
    {
        Serial.println("Falho ao deserializar o arquivo config.json");
        return "";
    }
    else
    {
        return doc.as<String>();
    }
}
void startConfiguration(fs::FS *filesystem)
{
    File file = filesystem->open("/config.json", "r");
    if (!file)
    {
        Serial.println("Falho ao abrir o arquivo de configuracao.");
        return;
    }
    else
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        if (error)
        {
            Serial.println("Falho ao deserializar o arquivo config.json");
            return;
        }
        else
        {
            JsonArray array = doc.as<JsonArray>();
            for (JsonObject module : array)
            {
                if (module["type"] == "actioner")
                {
                    pinMode(actionerPin, OUTPUT);
                    digitalWrite(actionerPin, LOW);
                    Serial.println("Configuracao de atuador estabelecida");
                    taskSendActionerInformation.enable();
                }
                else if (module["type"] == "doorSensor")
                {
                    pinMode(doorSensor, INPUT);
                    doorSensorState = digitalRead(doorSensor);
                    Serial.println("Configuracao do sensor de porta estabelecida");
                    taskGetDoorSensorInfo.enable();
                    taskSendDoorSensorInfo.enable();
                }
            }
        }
    };
}

// Confire se o arquivo config.json está vacio
bool isConfigJsonEmpty(fs::FS *filesystem)
{
    File file = filesystem->open("/config.json", "r");
    if (!file)
    {
        Serial.println("Erro ao abrir o arquivo config.json");
        return true;
    }
    else
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        if (error)
        {
            Serial.println("Erro ao deserializar  o arquivo config.json");
            return true;
        }
        else
        {
            if (doc.as<JsonArray>().size() == 0)
                return true;
        }
    }
    return false;
}

bool initLittleFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return false;
    }
    return true;
}

void listDir(const char *dirname)
{
    Serial.printf("Listing directory: %s\n", dirname);

    Dir root = LittleFS.openDir(dirname);

    while (root.next())
    {
        File file = root.openFile("r");
        Serial.print("  FILE: ");
        Serial.print(root.fileName());
        Serial.print("  SIZE: ");
        Serial.print(file.size());
        time_t cr = file.getCreationTime();
        time_t lw = file.getLastWrite();
        file.close();
        struct tm *tmstruct = localtime(&cr);
        Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        tmstruct = localtime(&lw);
        Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
}

bool deleteFile(String path)
{
    Serial.print("Deletando arquivo ");
    Serial.print(path);
    Serial.println(" : ");

    if (LittleFS.remove(path)) // exclui o arquivo passando o
                               //                          caminho/nome (path)
                               //                        Se a exclusão der certo, ...
    {
        // informa ao usuário que deu certo
        Serial.println(" - arquivo excluído");
    }
    else
    {
        // informa ao usuário que deu erros e sai da função retornando false.
        Serial.println(" - falha na exclusão");
        return false;
    }
    return true; // retorna true se não houver nenhum erro
}