#include "Settings.h"
#include "FS.h"
#ifndef ESP8266
  #include "SPIFFS.h"
#endif
#include <ArduinoJson.h>

Settings::Settings()
{
    readSettingsFile();
}
  
void Settings::setWifiLogin(String iSSID, String iPwd)
{
    wifiSSID = iSSID;
    wifiPWD = iPwd;
}

void Settings::setmqttServer(String iServer, int iPort, String iLogin, String iPwd)
{
    mqttServer = iServer;
    mqttPort = iPort;
    mqttLogin = iLogin;
    mqttPwd = iPwd;
}

void Settings::setOTA(String iOTAPWD)
{
    otaPWD = iOTAPWD;
}

void Settings::setWifiList(String iList)
{
    listWIFInets = iList;
}

bool Settings::saveSettings()
{
    String SettingsJson="";
    StaticJsonDocument<384> doc;

    doc["wifiSSID"] = wifiSSID;
    doc["wifiPWD"] = wifiPWD;
    doc["mqttServer"] = mqttServer;
    doc["mqttPort"] = mqttPort;
    doc["mqttLogin"] = mqttLogin;
    doc["mqttPwd"] = mqttPwd;
    doc["otaPwd"] = otaPWD;

    serializeJson(doc, SettingsJson);

    File SettingsFile = SPIFFS.open(Settings_FILE_PATH, "w");
    SettingsFile.println(SettingsJson);
    SettingsFile.close();
}

bool Settings::isWifiSetup()
{
    if(wifiSSID.length()==0 || wifiPWD.length()==0) return false;
    return true;
}

String Settings::getWifiSSID()
{
    return wifiSSID;
}

String Settings::getWifiPWD()
{
    return wifiPWD;
}

bool Settings::isMqttSetup()
{
    if(mqttServer.length()==0) return false;
    return true;
}

String Settings::getMqttServer()
{
    return mqttServer;
}

int Settings::getMqttPort()
{
    return mqttPort;
}

String Settings::getMqttLogin()
{
    return mqttLogin;
}

String Settings::getMqttPWD()
{
    return mqttPwd;
}

bool Settings::isOTASettings()
{
    if(otaPWD.length()==0) return false;
    return true;
}

String Settings::getOTAPWD()
{
    return otaPWD;
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
    
    wifiSSID = (const char*) doc["wifiSSID"];
    wifiPWD = (const char*) doc["wifiPWD"];
    mqttServer = (const char*) doc["mqttServer"];
    mqttPort = (int) doc["mqttPort"]; 
    mqttLogin = (const char*) doc["mqttLogin"];
    mqttPwd = (const char*) doc["mqttPwd"];
    otaPWD = (const char*) doc["otaPwd"]; 

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
        if(wifiName==wifiSSID) newOption.replace("#SSID_SELECTED#","selected");
        else newOption.replace("#SSID_SELECTED#","");
        wifiOptions += newOption;
    }

    //Generate the full HTML
    webPage.replace("#OPTION_SSID#",wifiOptions);
    webPage.replace("#WIFI_PWD#",wifiPWD);
    webPage.replace("#MQTT_SERVER#",mqttServer);
    webPage.replace("#MQTT_PORT#",String(mqttPort));
    webPage.replace("#MQTT_LOGIN#",mqttLogin);
    webPage.replace("#MQTT_PWD#",mqttPwd);
    webPage.replace("#OTA_PWD#",otaPWD);

    return webPage;
}