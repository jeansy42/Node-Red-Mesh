#include "LittleFS.h"
#include "FS.h"
#include "ArduinoJson.h"
#include "painlessMesh.h"

extern uint32_t rootId;
extern bool shouldReinit;
extern int actionerPin;
extern Task taskSendActionerInformation;
extern painlessMesh meshNode;

void createArchiveConfigJSON()

{
    String path = "/config.json";
    if (!LittleFS.exists(path))
    {

        Serial.printf("Criando arquivo: %s", path.c_str());
        File file = LittleFS.open(path, "w");
        if (!file)
        {
            Serial.println("Erro ao criar o arquivo");
        }
        else
        {
            Serial.printf("%s criado com sucesso \n", path.c_str());
        }
        file.close();
    }
}

String sendJsonResponseFromFile(String path)
{
    if (!LittleFS.exists(path))
    {
        JsonDocument doc;
        doc.to<JsonArray>();
        String emptyArray = doc.as<String>();
        return emptyArray;
    }
    JsonDocument docRes;
    File file = LittleFS.open(path, "r");
    deserializeJson(docRes, file);
    String response = docRes.as<String>();
    file.close();
    return response;
}

void setArchiveConfigJSON(JsonArray array)
{
    String path = "/config.json";
    if (!LittleFS.exists(path))
    {
        Serial.printf("O arquivo na ruta %s no existe", path.c_str());
        createArchiveConfigJSON();
    };
    File file = LittleFS.open(path, "w");
    if (!file)
    {
        Serial.printf("Error ao abrir o arquivo na ruta %s", path.c_str());
    }
    else
    {
        JsonDocument doc;
        for (JsonObject obj : array)
        {
            doc.to<JsonArray>().add(obj);
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

void waitingForConfiguratios()
{
    Serial.println("Aguardando configuracoes....");
}

void startConfiguration()
{
    File file = LittleFS.open("/config.json", "r");
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
            }
        }
    };
}

void verifyIfShouldReinit()
{
    if (shouldReinit)
    {
        Serial.println("Reiniciando no....");
        ESP.restart();
    }
}

// Confire se o arquivo config.json está vacio
bool isConfigJsonEmpty()
{
    File file = LittleFS.open("/config.json", "r");
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