#include "ConnectedObjects.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <EEPROM.h>

Prise::Prise(short iID, objType iType, int iEepromStartAddr) : Objet(iID, iType)
{
  eepromStartAddr = iEepromStartAddr;
}

PriseIOT::PriseIOT(PubSubClient *iMqttClient, short iID, objType iType, int iEepromStartAddr) : MyMQTTClient(iMqttClient, String(iID)+"_prise"), Prise(iID, iType, iEepromStartAddr)
{
  topicList.add("/prise/"+String(Id)+"/name");
  topicList.add("/prise/"+String(Id)+"/status");
  topicList.add("/prise/"+String(Id)+"/rules");
}

bool PriseIOT::publishName()
{
  bool bResult = true;

  String pubTopic = "/net/prise/" + String(Id) + "/name";
  bResult &= publishMsg(pubTopic, name, true);
  //Save to EEPROM
  if(eepromStartAddr!=-1)
  {
    char tmpName[23];
    EEPROM.get(ADDRESS_NAME + eepromStartAddr, tmpName);
    String sName(tmpName);
    if(sName!=name)
      bResult &= saveNameInEeprom();
  }
  return bResult;
}

bool PriseIOT::publishStatus()
{
  bool bResult = true;

  String pubTopic = "/net/prise/" + String(Id) + "/status";
  bResult &= publishMsg(pubTopic, String(status), true);

  if(eepromStartAddr!=-1)
  {
    bool bStat =(bool) EEPROM.read(ADDRESS_STATUS + eepromStartAddr);
    if(bStat!=status)
    {
      bResult &= saveStatusInEeprom();
    }
  }
  return bResult;
}

bool PriseIOT::publishRules()
{
  bool bResult = true;
  String pubTopic = "/net/prise/" + String(Id) + "/rules";
  String sRules = rulesToJson();
  bResult &= publishMsg(pubTopic, sRules, true);

  if(eepromStartAddr!=-1)
  {
    //Save in EEPROM if possible
    char tmpRules[EEPROM_RULES_SIZE+1];
    EEPROM.get(ADDRESS_RULES + eepromStartAddr, tmpRules);
    String sTmpRules(tmpRules);
    if(sTmpRules!=sRules)
    {
      bResult &= saveRulesInEeprom();
    }
  }
  return bResult;
}

bool PriseIOT::saveStatusInEeprom()
{
  bool bResult = false;
  if(eepromStartAddr!=-1)
  {
    EEPROM.put(ADDRESS_STATUS + eepromStartAddr, status);
    if(!EEPROM.commit()) 
    {
      Serial.printf("Write Status in EEPROM FAILED\n");
      pLog->addLogEntry("ERROR: Write Status in EEPROM FAILED");
    }
    else
    {
      Serial.printf("Write Status in EEPROM OK\n");
      bResult = true;
    }
  }
  return bResult;
}

bool PriseIOT::saveRulesInEeprom()
{
  bool bResult = false;
  if(eepromStartAddr!=-1)
  {
    String sRules = rulesToJson();

    if (sRules.length()>EEPROM_RULES_SIZE) sRules = rulesToJson(true); //save only active rules if too much
    if (sRules.length()<=EEPROM_RULES_SIZE)
    {
      char tmpRules[EEPROM_RULES_SIZE+1];
      memcpy(tmpRules, sRules.substring(0, EEPROM_RULES_SIZE).c_str(), sRules.substring(0, EEPROM_RULES_SIZE).length());
      tmpRules[sRules.substring(0, EEPROM_RULES_SIZE).length()]  ='\0';

      EEPROM.put(ADDRESS_RULES + eepromStartAddr, tmpRules);
      if(!EEPROM.commit())
      {
        Serial.printf("Write Rules in EEPROM FAILED\n");
        pLog->addLogEntry("ERROR: Write Rules in EEPROM FAILED");
      }
      else
      {
        Serial.printf("Write Rules in EEPROM OK %s\n", tmpRules);
        bResult = true;
      }
    }
    else
    {
      Serial.printf("Too many active rules to be published in EEPROM %s\n", sRules.c_str());
      pLog->addLogEntry("ERROR: Too many active rules to be published in EEPROM");
    }
  }
  return bResult;
}

bool PriseIOT::saveNameInEeprom()
{
  bool bResult = false;
  if(eepromStartAddr!=-1)
  {
    char tmpName[23];
    memcpy(tmpName, name.substring(0, 22).c_str(), name.substring(0, 22).length());
    tmpName[name.substring(0, 22).length()]  ='\0';
    EEPROM.put(ADDRESS_NAME + eepromStartAddr, tmpName);
    if(!EEPROM.commit()) 
    {
      Serial.printf("Write Name in EEPROM FAILED\n");
      pLog->addLogEntry("ERROR: Write Name in EEPROM FAILED");
    }
    else
    {
      Serial.printf("Write Name in EEPROM OK\n");
      bResult = true;
    }
  }
  return bResult;
}

bool PriseIOT::blink()
{
  return false;
}

bool PriseIOT::stopBlink()
{
  return false;
}

bool PriseIOT::update(bool iForce)
{
  MyMQTTClient::update(iForce);
  //Evaluate rules
  if(bBlink) blink();

  for(int i=0; i<aRules.size(); i++)
  {
    if( aRules[i].active && (((uint8_t)(pow(2,getDay())+128) & aRules[i].frequency) ||  aRules[i].frequency==128) )
    {
      bool bEvalRule = evalRule(aRules[i].condition, aRules[i].lastEvalResult);

      //Rules triggered from false to true => turn on
      if(bEvalRule > aRules[i].lastEvalResult)
      {
        switch(aRules[i].action)
        {
          case turnOn: turnON(); break;
          case Action::blink: bBlink = true; blink(); pLog->addLogEntry("START BLINK");
          break;
        }
        aRules[i].lastEvalResult = true;
      }
      //Rules triggered from true to false => Turn off
      else if(bEvalRule < aRules[i].lastEvalResult)
      {
        switch(aRules[i].action)
        {
          case turnOn: turnOFF(); break;
          case Action::blink: bBlink = false; stopBlink(); 
          break;
        }
        if(aRules[i].frequency==128)
        {
          aRules[i].active = false;
          publishRules();
        }
        aRules[i].lastEvalResult = false;
      }
    }
  }
  return true;
}

void PriseIOT::handleMqttCallback(char* iTopic, byte* payload, unsigned int iLength)
{
#ifdef _DEBUG_
  Serial.print("Message arrived [");
  Serial.print(iTopic);
  Serial.print("] ");
  for (int i = 0; i < iLength; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif
  String sTopic = iTopic;
  if(sTopic.startsWith("/prise"))
  {
    if(sTopic.endsWith("/name"))
    {
      char *tempString = (char *) malloc(sizeof(char)*(iLength+1));
      memcpy(tempString, payload, iLength);
      tempString[iLength] = '\0';
      String sTmpName (tempString);
      free(tempString);
      if(name!=sTmpName)
      {
        if(sTmpName.length()==0) name = String(Id);
        else name = sTmpName;
        publishName();
        pLog->addLogEntry("Name changed to " + name);
      }
    }
    if(sTopic.endsWith("/status"))
    {
      char *tempString = (char *) malloc(sizeof(char)*(iLength+1));
      memcpy(tempString, payload, iLength);
      tempString[iLength] = '\0';
      int iStatus = atoi(tempString);
      free(tempString);
      if(iStatus ^ status)
      {
        if(iStatus) turnON();
        else turnOFF();
        status = iStatus;
      }
    }
    if(sTopic.endsWith("/rules"))
    {
      char *tempString = (char *) malloc(sizeof(char)*(iLength+1));
      memcpy(tempString, payload, iLength);
      tempString[iLength] = '\0';
      String iRules (tempString);
      free(tempString);

      //Parse rules coded in Json format to feed rule list
      pLog->addLogEntry("Rules have been updated");
      jsonToRules(iRules);

      //Republish to save rules for next restart and for web
      publishRules();

      //Stop blink as it will restart if new rule requires it (otherwise no way to stop blinking)
      bBlink = false;
      stopBlink();
    }
  }
}

bool PriseIOT::toggle()
{
  bool bResult = true;
  if(status) bResult = turnOFF();
  else bResult = turnON();
  return bResult;
}

bool PriseIOT::init()
{
  if(eepromStartAddr!=-1)
  {
    if(!eepromInit)
    {
      EEPROM.begin(eepromSize);
      eepromInit = true;
    }

    if(EEPROM.read(eepromStartAddr)==123)
    {
  #ifdef _DEBUG_
  Serial.println("Reading parameters from EEPROM");
  #endif
      //read EEPROM content with status, name and rules
      status =(bool) EEPROM.read(ADDRESS_STATUS + eepromStartAddr);

  #ifdef _DEBUG_
      Serial.printf("Status = %d\n", status);
  #endif

      char tmpName[23];
      EEPROM.get(ADDRESS_NAME + eepromStartAddr, tmpName);
      String sName(tmpName);
      if(sName.length()>0) name = sName;
  #ifdef _DEBUG_
      Serial.printf("Name = %s\n", sName.c_str());
  #endif

      char tmpRules[EEPROM_RULES_SIZE+1];

      EEPROM.get(ADDRESS_RULES + eepromStartAddr, tmpRules);
      String sRules(tmpRules);

  #ifdef _DEBUG_
    if(sRules.length()>0)
      Serial.printf("Rules = %s\n", sRules.c_str());
  #endif
      if(sRules.length()>1 && sRules.startsWith("{"))
      {
        jsonToRules(sRules);
      }
    }
    else
    {
      //Initialize EEPROM and store default values
#ifdef _DEBUG_
    Serial.println("Initialize EEPROM");
#endif
      EEPROM.write(eepromStartAddr, 123);
      EEPROM.commit();
      saveNameInEeprom();
      saveRulesInEeprom();
      saveStatusInEeprom();
    }
    if(status) turnON(true);
  }
  return true;
}

bool PriseIOT::publishParams()
{
  bool bResult = true;
#ifdef _DEBUG_
  Serial.println("PriseIOT::PublishParams");
#endif

  String pubTopic = "/net/prise/" + String(Id) + "/type";
  bResult &= publishMsg(pubTopic, String(type), true);

  pubTopic = "/net/prise/" + String(Id) + "/name";
  bResult &= publishMsg(pubTopic, name, true);

  bResult &= publishStatus();

  bResult &= publishRules();
  return bResult;
}

bool PriseIOT_RF433::turnON(bool iForce)
{
#ifdef _DEBUG_
  Serial.printf("TurnON prise %d\n",Id);
#endif
  if(!status || iForce) SendMessage(plugNumber, NEXA_ON);
  status = true;
  publishStatus();
  pLog->addLogEntry("TurnON " + String(ID));
  return true;

}

bool PriseIOT_RF433::turnOFF()
{
#ifdef _DEBUG_
  Serial.printf("TurnOFF prise %d\n",Id);
#endif
  if(status) SendMessage(plugNumber, NEXA_OFF);
  status = false;
  publishStatus();
  pLog->addLogEntry("TurnOFF " + String(ID) );
  return true;
}

PriseIOT_ESP::PriseIOT_ESP (PubSubClient *iMqttClient, short iID, int iEepromStartAddr) : PriseIOT(iMqttClient, iID, objType::priseWifi, iEepromStartAddr)
{
  pinMode(12, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

bool PriseIOT_ESP::turnON(bool iForce)
{
#ifdef _DEBUG_
  Serial.printf("TurnON prise ESP %d\n",Id);
#endif
  if(digitalRead(12)!=HIGH || iForce) digitalWrite(12, HIGH);
  status = true;
  publishStatus();
  pLog->addLogEntry("TurnON");
  return true;

}

bool PriseIOT_ESP::turnOFF()
{
#ifdef _DEBUG_
  Serial.printf("TurnOFF prise %d\n",Id);
#endif
  if(digitalRead(12)==HIGH) digitalWrite(12, LOW);
  status = false;
  publishStatus();
  pLog->addLogEntry("TurnOFF");
  return true;
}

bool PriseIOT_ESP::blink()
{
  if(millis()-lastBlink>BLINK_DELAY*1000)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    lastBlink = millis();
  }
  return true;
}

bool PriseIOT_ESP::stopBlink()
{
  digitalWrite(LED_BUILTIN, HIGH);
  pLog->addLogEntry("STOP BLINK");
  return true;
}

bool Prise::evalRule(String iRule, bool lastEvalResult)
{
//Serial.println("EvalRule: " + iRule);
  bool bResult = false;
  iRule.replace(" ", "");
  //AND rules
  if(iRule.indexOf(";")!=-1)
  {
    String iRule1 = iRule.substring(0, iRule.indexOf(";"));
    String iRule2 = iRule.substring(iRule.indexOf(";")+1);

    bResult = evalRule(iRule1, lastEvalResult) && evalRule(iRule2, lastEvalResult);
  }
  else if(iRule.indexOf("<H<")>0)
  {
    long currentTime = getTimeSec();
    //long delta = difftime(time(nullptr), pEnv->getSunriseTime());
    //Sunrise and sunset time are compared with absolute
    //while H time is compared as time in the day...
    String sStart = iRule.substring(0, iRule.indexOf("<"));

    long sTime = 0;
    bool bStartCond = false;
    bool bEndCond = false;
    
    if(sStart.startsWith("Sun"))
    {
      if(!pEnv->isSunriseSunsetUptodate()) pEnv->updateSunriseSunsetTime();
      if(sStart.startsWith("Sunrise"))
        sTime = pEnv->getSunriseTime();
      else if(sStart.startsWith("Sunset"))
        sTime = pEnv->getSunsetTime() + 1800;
      
      if(difftime(time(nullptr), sTime)>0)
        bStartCond = true;

    }
    else
    {
      sTime = sStart.toInt() * 300;
      if(sTime<currentTime)
        bStartCond = true;
    }
    
    long eTime = 288*300;
    String sEnd =  iRule.substring(iRule.lastIndexOf("<")+1);
    if(sEnd.startsWith("Sun"))
    {
      if(!pEnv->isSunriseSunsetUptodate()) pEnv->updateSunriseSunsetTime();
      if(sEnd.startsWith("Sunrise"))
        eTime = pEnv->getSunriseTime() - 1800;
      else if(sEnd.startsWith("Sunset"))
        eTime = pEnv->getSunsetTime();

      if(difftime(time(nullptr), eTime)<0)
        bEndCond = true;
    }
    else
    {
      eTime = sEnd.toInt() *300; 
      if(currentTime<eTime)
        bEndCond = true;
    }

    if(bStartCond && bEndCond)
      bResult = true;
  }
  else if(iRule.startsWith("CarW"))
  {
    long tempSensorId = iRule.substring(iRule.indexOf("/")+1, iRule.lastIndexOf("/")).toInt();
    float currTemp = pEnv->getSensorValue(tempSensorId);
    long currentTime = getTimeSec();
    long targetTime = iRule.substring(iRule.lastIndexOf("/")+1).toInt();
    long startTime = 0;
    if(!lastEvalResult) //if rule is still active, do not calculate new start time to avoid limit case where temperature changes after started
      startTime = getCarWarmerStartTime(targetTime, currTemp); 
    

    //If currentTime > target time, then rule is fulfilled and can be deactivated
    if(currentTime>startTime && currentTime<=targetTime)
      bResult = true;
  }
  //Probably a sensor ID
  //But do not check again to not untrigger a rule started (only hours can untrigger a rule...)
  // /!\ it means it cannot drive a socket only with temperature...
  else if(!lastEvalResult)
  {
    int ID = iRule.substring(0,5).toInt();
    if(ID>0)
    {
      float currValue = pEnv->getSensorValue(ID);
      if(currValue>-100)
      {
        float targetValue = iRule.substring(6).toFloat();
        if(iRule.charAt(5)=='>')
          if(currValue>targetValue)
            bResult = true;
        if(iRule.charAt(5)=='<')
          if(currValue<targetValue)
            bResult = true;
        if(iRule.charAt(5)=='=')
          if(currValue==targetValue)
            bResult = true;
      }
    }
  }
  else
  {
    bResult = true;
  }
  
  return bResult;
}

long Prise::getCarWarmerStartTime(long iTargetTime, float iTemp)
{
  //means never as current time will not be greater than that
  long startTime = 86400;
  if(iTemp<-100) startTime = iTargetTime - 3600; //Could not get proper time => start 1H earlier to be sure
  /*else if(iTemp<-10) startTime = iTargetTime - 9000;//2H30
  else if(iTemp<-7) startTime = iTargetTime - 7200;//2H00
  else if(iTemp<-4) startTime = iTargetTime - 5400;//1H30
  else if(iTemp<-1) startTime = iTargetTime - 3600;//1H
  else if(iTemp<1) startTime = iTargetTime - 2700;//45min*/
  else if(iTemp<=1) startTime = iTargetTime + 60*(10*iTemp-50);

  if(startTime<0) startTime = 0; //Start now as should have already started

  return startTime;
}

/*String Prise::getOperande(String iString)
{
  int nbBrace = 0;
  int i = 0;

  while( ( (iString.charAt(i)!=',' && iString.charAt(i)!=')') || nbBrace>0) && i<iString.length())
  {
    if(iString.charAt(i)=='(') nbBrace++;
    if(iString.charAt(i)==')') nbBrace--;
    i++;
  }

  return iString.substring(0, i);
}*/

String Prise::rulesToJson(bool iActiveOnly)
{
  String output;

  const size_t capacity = JSON_ARRAY_SIZE(aRules.size()) + JSON_OBJECT_SIZE(1) + aRules.size()*JSON_OBJECT_SIZE(3)+200;
  DynamicJsonDocument doc(capacity);
  JsonArray rules = doc.createNestedArray("rules");
  for(int i=0;i<aRules.size();i++)
  {
    if(!iActiveOnly || (iActiveOnly && aRules[i].active))
    {
      JsonObject rule = rules.createNestedObject();
      rule["freq"] = aRules[i].frequency;
      rule["rule"] = aRules[i].condition;
      rule["active"] = aRules[i].active;
      switch(aRules[i].action)
      {
        case Action::turnOn: rule["action"] = "turnON"; break;
        case Action::turnOff: rule["action"] = "turnOFF"; break;
        case Action::blink: rule["action"] = "blink"; break;
        case Action::toggle: rule["action"] = "toggle"; break;
        default:rule["action"] = String(aRules[i].action);break;
      }
    }
  }
  serializeJson(doc, output);

  return output;
}

void Prise::jsonToRules(String iJson)
{
  aRules.removeAll();

  const size_t capacity = JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(1) + 9*JSON_OBJECT_SIZE(3) + 200;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, iJson);

  JsonArray rules = doc["rules"];
  for(int i=0; i<rules.size(); i++)
  {
    JsonObject myRule = rules[i];
    Rule currentRule;
    currentRule.frequency = myRule["freq"];
    String sCond = myRule["rule"];
    currentRule.condition = sCond;
    String sAction = myRule["action"];
    if (sAction=="turnON") currentRule.action = turnOn;
    else if (sAction=="turnOFF") currentRule.action = turnOff;
    else if (sAction=="blink") currentRule.action = blink;
    else
    {
      currentRule.action = toggle;
      Serial.println("ERROR in action, received " + sAction);
      pLog->addLogEntry("ERROR: UpdateRules(jsonToRules) unknown action " + sAction);
    }
    currentRule.active = myRule["active"];
    aRules.add(currentRule);
  }
  //pLog->addLogEntry("Rules have been updated");
}

int Prise::getNbActiveRules()
{
  int iNbActiveRule = 0;
  for(int i=0; i<aRules.size();i++)
    if (aRules[i].active) iNbActiveRule++;
  return iNbActiveRule;
}

SensorIOT::SensorIOT(PubSubClient *iMqttClient, short iId, sensorType iSType, int iEepromStartAddr) : MyMQTTClient(iMqttClient, String(iId)+"_sensor"), Sensor(iId, iSType, iEepromStartAddr)
{
  topicList.add("/sensor/"+String(Id)+"/name");
}

bool SensorIOT::init()
{
  if(eepromStartAddr!=-1)
  {
  if(!eepromInit)
  {
      EEPROM.begin(eepromSize);
      eepromInit = true;
    }
    if(EEPROM.read(eepromStartAddr)==123)
    {
  #ifdef _DEBUG_
  Serial.println("Reading parameters from EEPROM");
  #endif

      char tmpName[23];
      EEPROM.get(eepromStartAddr+ADDRESS_SENSOR_NAME, tmpName);
      String sName(tmpName);
      if(sName.length()>0) name = sName;
      else saveNameInEeprom();
  #ifdef _DEBUG_
      Serial.printf("Name = %s\n", sName.c_str());
  #endif
    }
    else
    {
      //Initialize EEPROM and store default values
      EEPROM.write(eepromStartAddr, 123);
      EEPROM.commit();
#ifdef _DEBUG_
  Serial.printf("Sensor::init reading eeprom start address=%d  value=%d\n", eepromStartAddr, EEPROM.read(eepromStartAddr));
#endif
      saveNameInEeprom();
    }
  }

  return update(true);
}

bool SensorIOT::update(bool iForce)
{
  MyMQTTClient::update(iForce);
  bool result = true;
  if(millis()-lastUpdateTime > SENSOR_UPDATE_FREQ*1000 || iForce)
  {
#ifdef _DEBUG_
    Serial.printf("Update Sensor %d\n", Id);
#endif
    float value = getValue();
    if(value >-100)
    {
        if(pEnv)
          pEnv->setSensorValue(Id, value);
        String pubTopic = "/sensor/" + String(Id) + "/value";
        publishMsg(pubTopic, String(value), true);
    }
    else
    {
#ifdef _DEBUG_
        Serial.println("ERROR: no temp");
#endif
        String pubTopic = "/sensor/" + String(Id) + "/value";
        publishMsg(pubTopic,"", false);
        result = false;
    }
    lastUpdateTime = millis();
  }
  return result;
}

void SensorIOT::handleMqttCallback(char* iTopic, byte* payload, unsigned int iLength)
{
#ifdef _DEBUG_
  Serial.print("Message arrived [");
  Serial.print(iTopic);
  Serial.print("] ");
  for (int i = 0; i < iLength; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif
  String sTopic = iTopic;
  if(sTopic.endsWith("/name"))
  {
    char *tempString = (char *) malloc(sizeof(char)*(iLength+1));
    memcpy(tempString, payload, iLength);
    tempString[iLength] = '\0';
    String sTmpName (tempString);
    free(tempString);
    if(name!=sTmpName)
    {
      if(sTmpName.length()==0) name = String(Id);
      else name = sTmpName;
      pLog->addLogEntry("Name changed to " + name);
      publishName();
    }
  }
}

bool SensorIOT::publishName()
{
  bool bResult = true;
  String pubTopic = "/net/sensor/" + String(Id) + "/name";
  bResult &= publishMsg(pubTopic, name, true);
  //Save to EEPROM
  if(eepromStartAddr!=-1)
  {
    char tmpName[23];
    EEPROM.get(eepromStartAddr+ADDRESS_SENSOR_NAME, tmpName);
    String sName(tmpName);
    if(sName!=name)
      bResult &= saveNameInEeprom();
  }

  return bResult;
}

bool SensorIOT::saveNameInEeprom()
{
  bool bResult = false;
  if(eepromStartAddr!=-1)
  {
    char tmpName[23];
    memcpy(tmpName, name.substring(0, 22).c_str(), name.substring(0, 22).length());
    tmpName[name.substring(0, 22).length()]  ='\0';
    EEPROM.put(eepromStartAddr+ ADDRESS_SENSOR_NAME, tmpName);
    if(!EEPROM.commit()) 
    {
       Serial.printf("Write Sensor Name in EEPROM FAILED\n");
       pLog->addLogEntry("ERROR: Write Sensor Name in EEPROM FAILED");
    }
    else
    {
      Serial.printf("Write Sensor Name in EEPROM OK\n");
      bResult = true;
    }
  }
  return bResult;
}

bool SensorIOT::publishParams()
{
  bool bResult = true;
#ifdef _DEBUG_
  Serial.println("SensorIOT::PublishParams");
#endif

  String pubTopic = "/net/sensor/" + String(Id) + "/type";
  bResult &= publishMsg(pubTopic, String(sType), true);

  pubTopic = "/net/sensor/" + String(Id) + "/name";
  bResult &= publishMsg(pubTopic, name, true);

  return bResult;
}

bool TempSensorAttiny::init()
{
  pinMode(pinNumber, OUTPUT);
  digitalWrite(pinNumber, HIGH);
  return SensorIOT::init();
}

float TempSensorAttiny::getValue()
{
  float oValue = -255;
  digitalWrite(pinNumber, LOW);
  delay(50);
  digitalWrite(pinNumber, HIGH);
  String readTemp = Serial.readStringUntil('\n');

  if(readTemp.length()>0)
  {
#ifdef _DEBUG_
      Serial.printf("ReadTemp %s\n", readTemp.c_str());
#endif
      oValue = readTemp.substring(2).toFloat();
      //flush serial buffer
      delay(50);
      while(Serial.available())
      {
        Serial.read();
        delay(50);
      }
  }
  return oValue;
}

bool TempSensorDS18B20::init()
{
  pOneWire = new OneWire(pinNumber);
  pSensors = new DallasTemperature (pOneWire);
  pSensors->begin();
  pSensors->getAddress(sensorAddress, 0);
  pSensors->setResolution(sensorAddress, 9);
  return SensorIOT::init();
}

float TempSensorDS18B20::getValue()
{
  pSensors->requestTemperatures();
  float oValue = pSensors->getTempC(sensorAddress);
  return oValue;
}

void WeatherDisplayObj::init()
{
  if(eepromStartAddr!=-1)
  {
    if(!eepromInit)
    {
      EEPROM.begin(eepromSize);
      eepromInit = true;
    }

    if(EEPROM.read(eepromStartAddr)==123)
    {
  #ifdef _DEBUG_
  Serial.println("Reading parameters from EEPROM");
  #endif

      char tmpName[23];
      EEPROM.get(ADDRESS_DISPLAY_NAME + eepromStartAddr, tmpName);
      String sName(tmpName);
      if(sName.length()>0) name = sName;
  #ifdef _DEBUG_
      Serial.printf("Name = %s\n", sName.c_str());
  #endif

      char tmpLInfo[10];
      EEPROM.get(ADDRESS_DISPLAY_LINFO + eepromStartAddr, tmpLInfo);
      String sLInfo(tmpLInfo);
      if(sLInfo.length()>0) leftInfo = sLInfo;
  #ifdef _DEBUG_
      Serial.printf("leftInfo = %s\n", leftInfo.c_str());
  #endif

      char tmpRInfo[10];
      EEPROM.get(ADDRESS_DISPLAY_RINFO + eepromStartAddr, tmpRInfo);
      String sRInfo(tmpRInfo);
      if(sRInfo.length()>0) rightInfo = sRInfo;
  #ifdef _DEBUG_
      Serial.printf("rightInfo = %s\n", rightInfo.c_str());
  #endif

      int tmpLayout = EEPROM.read(ADDRESS_DISPLAY_LAYOUT+ eepromStartAddr);
      if(tmpLayout == 1 || tmpLayout == 2)
      {
        layout = tmpLayout;
  #ifdef _DEBUG_
        Serial.printf("layout = %d\n", layout);
  #endif
      }
    }
    else
    {
      EEPROM.write(eepromStartAddr, 123);
      EEPROM.commit();
      saveInEeprom();
    }
  }
  return;
}

bool WeatherDisplayObj::saveInEeprom()
{
  bool bResult = true;
  if(eepromStartAddr!=-1)
  {
    char tmpName[23];
    memcpy(tmpName, name.substring(0, 22).c_str(), name.substring(0, 22).length());
    tmpName[name.substring(0, 22).length()]  ='\0';
    EEPROM.put(ADDRESS_DISPLAY_NAME+ eepromStartAddr, tmpName);
    if(!EEPROM.commit())
    {
      Serial.printf("Write Display Name in EEPROM FAILED\n");
      pLog->addLogEntry("ERROR: Write Display Name in EEPROM FAILED");
      bResult = false;
    }
    else
      Serial.printf("Write Display Name in EEPROM OK\n");

    char tmpLInfo[10];
    memcpy(tmpLInfo, leftInfo.c_str(), leftInfo.length());
    tmpLInfo[leftInfo.length()]  ='\0';
    EEPROM.put(ADDRESS_DISPLAY_LINFO+ eepromStartAddr, tmpLInfo);
    if(!EEPROM.commit())
    {
      Serial.printf("Write Display LInfo in EEPROM FAILED\n");
      pLog->addLogEntry("ERROR: Write Display LInfo in EEPROM FAILED");
      bResult = false;
    }
    else
      Serial.printf("Write Display LInfo in EEPROM OK\n");

    char tmpRInfo[10];
    memcpy(tmpRInfo, rightInfo.c_str(), rightInfo.length());
    tmpRInfo[rightInfo.length()]  ='\0';
    EEPROM.put(ADDRESS_DISPLAY_RINFO+ eepromStartAddr, tmpRInfo);
    if(!EEPROM.commit())
    {
      Serial.printf("Write Display RInfo in EEPROM FAILED\n");
      pLog->addLogEntry("ERROR: Write Display RInfo in EEPROM FAILED");
      bResult = false;
    }
    else
      Serial.printf("Write Display RInfo in EEPROM OK\n");

    /*char tmpRInfo[10];
    memcpy(tmpRInfo, rightInfo.c_str(), rightInfo.length());
    tmpRInfo[rightInfo.length()]  ='\0';*/
    EEPROM.put(ADDRESS_DISPLAY_LAYOUT+ eepromStartAddr, layout);
    if(!EEPROM.commit())
    {
      Serial.printf("Write Display Layout in EEPROM FAILED\n");
      pLog->addLogEntry("ERROR: Write Display Layout in EEPROM FAILED");
      bResult = false;
    }
    else
      Serial.printf("Write Display Layout in EEPROM OK\n");
  }
  return bResult;
}

float ESP_Vcc::getValue()
{
#if defined(ESP8266)
  return (float)ESP.getVcc()/1024.0;
#else
  return (float)analogRead(A13)/4095*3.5287*2;
#endif
}
