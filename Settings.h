/************************************************
* Class to manage App settings;
* wifi SSID
* wifi PassWord
* Mqtt Server address
* Mqtt Port
* Mqtt user name
* Mqtt Password
* OTA Password
************************************************/

#ifndef Settings_h
#define Settings_h

#include <WString.h>

//log file path
#define Settings_FILE_PATH "/MyHAS_Settings.txt"

class Settings
{
public:
  Settings();
  void setWifiList(String iList);
  void setWifiLogin(String iSSID, String iPwd);
  void setmqttServer(String iServer, int iPort, String iLogin, String iPwd);
  void setOTA(String iOTAPWD);
  bool saveSettings();

  bool isWifiSetup();
  const char* getWifiSSID();
  const char* getWifiPWD();
  
  bool isMqttSetup();
  int getMqttPort();
  const char* getMqttServer();
  const char* getMqttLogin();
  const char* getMqttPWD();

  bool isOTASettings();
  const char* getOTAPWD();
  
  String getSettingsHtml();

private:
  void readSettingsFile();
  
  int mqttPort = 0;
  //List of available wifi networks, separated by ';'
  String listWIFInets = "";
  char *strWifiSSID = NULL;
  char *strWifiPWD = NULL;
  char *strMqttServer = NULL;
  char *strMqttLogin = NULL;
  char *strMqttPWD = NULL;
  char *strOtaPWD = NULL;
};

const char Settings_html[] PROGMEM = R"html(
<!DOCTYPE HTML><html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1" />
<link rel="stylesheet" type="text/css" href="http://cedric.millard.free.fr/MyHAS.css">
</head>
<body>
<h1>Settings</h1>
<form id="FormR" action="/updateSettings" method="post">
<label for="wifiSSID">Wifi Network:</label> 
<select id="wifiSSID" name="wifiSSID">
#OPTION_SSID#
</select>
<br>
<label for="wifiPWD">Wifi Password:</label> 
<input type="password" id="wifiPWD" name="wifiPWD" value="#WIFI_PWD#" required>
<br>
<label for="mqttServer">MQTT Server:</label> 
<input type="text" id="mqttServer" name="mqttServer" value="#MQTT_SERVER#" required> 
<label for="mqttPort">Port:</label> 
<input type="number" id="mqttPort" name="mqttPort" value="#MQTT_PORT#" required> 
<br>
<label for="mqttLogin">MQTT Login:</label> 
<input type="text" id="mqttLogin" name="mqttLogin" value="#MQTT_LOGIN#" required>
<br>
<label for="mqttPWD">MQTT Password:</label> 
<input type="password" id="mqttPWD" name="mqttPWD" value="#MQTT_PWD#" required>
<br>
<label for="otaPWD">OTA Password:</label> 
<input type="password" id="otaPWD" name="otaPWD" value="#OTA_PWD#" required>
<br>
<input id="sub" type="submit" value="Save">
</form>
</body>
</html>
)html";

const char Wifi_options[] PROGMEM = R"Option(
<option value="#SSID#" id="WIFI_#I#" #SSID_SELECTED#>#SSID#</option>
)Option";

#endif
