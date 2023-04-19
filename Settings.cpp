#include "Settings.h"
#include "FS.h"
#ifndef ESP8266
  #include "SPIFFS.h"
#endif
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <string.h>

Settings::Settings()
{
#ifdef ESP8266
  bool result = SPIFFS.begin();
#else
  bool result = SPIFFS.begin(true);
#endif
    //SPIFFS.remove(Settings_FILE_PATH);
    readSettingsFile();
}
  
void Settings::setWifiLogin(String iSSID, String iPwd)
{
    if(strWifiSSID) free(strWifiSSID);
    strWifiSSID = (char *) malloc(sizeof(char)*(iSSID.length()+1));
    iSSID.toCharArray(strWifiSSID,iSSID.length()+1);

    if(strWifiPWD) free(strWifiPWD);
    strWifiPWD = (char *) malloc(sizeof(char)*(iPwd.length()+1));
    iPwd.toCharArray(strWifiPWD,iPwd.length()+1);
}

void Settings::setmqttServer(String iServer, int iPort, String iLogin, String iPwd)
{
    if(strMqttServer) free(strMqttServer);
    strMqttServer = (char *) malloc(sizeof(char)*(iServer.length()+1));
    iServer.toCharArray(strMqttServer,iServer.length()+1);
    
    mqttPort = iPort;

    if(strMqttLogin) free(strMqttLogin);
    strMqttLogin = (char *) malloc(sizeof(char)*(iLogin.length()+1));
    iLogin.toCharArray(strMqttLogin,iLogin.length()+1);

    if(strMqttPWD) free(strMqttPWD);
    strMqttPWD = (char *) malloc(sizeof(char)*(iPwd.length()+1));
    iPwd.toCharArray(strMqttPWD,iPwd.length()+1);
}

void Settings::setOTA(String iOTAPWD)
{
    if(strOtaPWD) free(strOtaPWD);
    strOtaPWD = (char *) malloc(sizeof(char)*(iOTAPWD.length()+1));
    iOTAPWD.toCharArray(strOtaPWD,iOTAPWD.length()+1);
}

void Settings::setWifiList(String iList)
{
    listWIFInets = iList;
}

bool Settings::saveSettings()
{
    String SettingsJson="";
    StaticJsonDocument<384> doc;

    doc["wifiSSID"] = strWifiSSID;
    doc["wifiPWD"] = strWifiPWD;
    doc["mqttServer"] = strMqttServer;
    doc["mqttPort"] = mqttPort;
    doc["mqttLogin"] = strMqttLogin;
    doc["mqttPwd"] = strMqttPWD;
    doc["otaPwd"] = strOtaPWD;

    serializeJson(doc, SettingsJson);

    File SettingsFile = SPIFFS.open(Settings_FILE_PATH, "w");
    SettingsFile.println(SettingsJson);
    SettingsFile.close();
    return true;
}

bool Settings::isWifiSetup()
{
    if(!strWifiSSID) return false;
    return true;
}

const char* Settings::getWifiSSID()
{
    return strWifiSSID;
}

const char* Settings::getWifiPWD()
{
    return strWifiPWD;
}

bool Settings::isMqttSetup()
{
    if(!strMqttServer) return false;
    return true;
}

const char* Settings::getMqttServer()
{
    return strMqttServer;
}

int Settings::getMqttPort()
{
    return mqttPort;
}

const char* Settings::getMqttLogin()
{
    return strMqttLogin;
}

const char* Settings::getMqttPWD()
{
    return strMqttPWD;
}

bool Settings::isOTASettings()
{
    if(!strOtaPWD) return false;
    return true;
}

const char* Settings::getOTAPWD()
{
    return strOtaPWD;
}
  
void Settings::readSettingsFile()
{
    String SettingsJson="";
    File SettingsFile = SPIFFS.open(Settings_FILE_PATH, "r");
    while(SettingsFile.available() && SettingsJson.length()==0) 
    {
       SettingsJson = SettingsFile.readStringUntil('\n');
    }
    SettingsFile.close();

    if(SettingsJson.length()==0) return;

    StaticJsonDocument<384> doc;
    DeserializationError error = deserializeJson(doc, SettingsJson);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        return;
    }
    
    mqttPort = (int) doc["mqttPort"]; 

    if(strWifiSSID) free(strWifiSSID);
    strWifiSSID = (char *) malloc(sizeof(char)*(strlen(doc["wifiSSID"])+1));
    strcpy(strWifiSSID, (const char* )doc["wifiSSID"]);

    if(strWifiPWD) free(strWifiPWD);
    strWifiPWD = (char *) malloc(sizeof(char)*(strlen(doc["wifiPWD"])+1));
    strcpy(strWifiPWD, (const char* )doc["wifiPWD"]);

    if(strMqttServer) free(strMqttServer);
    strMqttServer = (char *) malloc(sizeof(char)*(strlen(doc["mqttServer"])+1));
    strcpy(strMqttServer, (const char* )doc["mqttServer"]);

    if(strMqttLogin) free(strMqttLogin);
    strMqttLogin = (char *) malloc(sizeof(char)*(strlen(doc["mqttLogin"])+1));
    strcpy(strMqttLogin, (const char* )doc["mqttLogin"]);

    if(strMqttPWD) free(strMqttPWD);
    strMqttPWD = (char *) malloc(sizeof(char)*(strlen(doc["mqttPwd"])+1));
    strcpy(strMqttPWD, (const char* )doc["mqttPwd"]);

    if(strOtaPWD) free(strOtaPWD);
    strOtaPWD = (char *) malloc(sizeof(char)*(strlen(doc["otaPwd"])+1));
    strcpy(strOtaPWD, (const char* )doc["otaPwd"]);

}

String Settings::getSettingsHtml()
{
    String webPage = FPSTR(Settings_html);
    
    //Create the list of options
    String wifiOptions = "";
    String slistWifi = listWIFInets;
    int nbOptions = 0;
    while(slistWifi.indexOf(';')>=0)
    {
        nbOptions++;
        String wifiName = slistWifi.substring(0, slistWifi.indexOf(';'));
        slistWifi = slistWifi.substring(slistWifi.indexOf(';')+1);
        String newOption = FPSTR(Wifi_options);
        newOption.replace("#SSID#",wifiName);
        newOption.replace("#I#",String(nbOptions));
        if(wifiName==String(strWifiSSID)) newOption.replace("#SSID_SELECTED#","selected");
        else newOption.replace("#SSID_SELECTED#","");
        wifiOptions += newOption;
    }

    //Generate the full HTML
    webPage.replace("#OPTION_SSID#",wifiOptions);
    webPage.replace("#WIFI_PWD#",strWifiPWD);
    webPage.replace("#MQTT_SERVER#",strMqttServer);
    webPage.replace("#MQTT_PORT#",String(mqttPort));
    webPage.replace("#MQTT_LOGIN#",strMqttLogin);
    webPage.replace("#MQTT_PWD#",strMqttPWD);
    webPage.replace("#OTA_PWD#",strOtaPWD);

    return webPage;
}