/*
 * Library to gather all environmental data like real time and temperature
 * TODO:
 *  - Ajouter action blink
 *  - Ajouter action apairage prise RF
 * (c) Cedric Millard 2020
 */

#ifndef ConnectedOjects_h
#define ConnectedOjects_h

#include "ArrayCed.h"
#include "MyMQTTClient.h"
#include "NexaTransmitter.h"
#include "Environment.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Logging.h"


enum Action { turnOn, turnOff, toggle, blink };
enum objType { prise, priseRF, priseWifi, sensor, display };
enum sensorType { temp, weather, tension, temp_ro };

struct Rule
{
  //Format T1<H<T2 or CARW/SensorID/Time or SensorID<Time or SensorID>Time or AND conditions separated by ; (53<H<230;10010<25)
  String condition;

  Action action;
  //Freq coded on 8 bits. higher bit means one time only
  // example: 10000000 = 128 => one time, no repeat
  //          01000001 = 65 => every saturday and sunday (2^0 = sunday, 2^1 = monday...)
  uint8_t frequency;

  //inactive rules are kept but not evaluated
  bool active = true;

  //Internal flag to deactivate a rule once it has been evaluated true then back to false
  bool lastEvalResult = false;
};

String rulesToJson(Array<Rule> *aRules);

class Objet
{
public:
  Objet(short iId, objType iType): Id(iId), type(iType){name = String(iId);}
  short Id = 0;
  String name;
  objType type;
  //Used by topics cleaner to detect if a node is no longer connected
  long _lastUpdate = 0;
protected:
  static int eepromSize;
  static bool eepromInit;
  int eepromStartAddr = 0;
};

class Prise : public Objet
{
  public:
  Prise(objType iType = objType::prise): Objet(0, iType){}
  Prise(short iID, objType iType = objType::prise, int iEepromStartAddr = 0);
  void setEnv(Environment *iEnv){pEnv = iEnv;}
  void setLog(Logging *iLog){pLog = iLog;}
  int getNbActiveRules();
  String rulesToJson(bool iActiveOnly = false);
  void jsonToRules(String iJson);
  Array<Rule> aRules;
  bool status = false;
protected:
  bool evalRule(String iRule, bool lastEvalResult = false);
  Logging *pLog = NULL;
private:
  //String getOperande(String iString);
  long getCarWarmerStartTime(long iTargetTime, float iTemp);
  Environment *pEnv = NULL;
};

class PriseIOT : public Prise, public MyMQTTClient
{
  public:
  PriseIOT(PubSubClient *iMqttClient, short iID, objType iType = objType::prise, int iEepromStartAddr = 0);
  void handleMqttCallback(char* iTopic, byte* payload, unsigned int length);
  bool update(bool iForce = false);
  virtual bool turnON(bool iForce = false) = 0;
  virtual bool turnOFF() = 0;
  virtual bool blink();
  virtual bool stopBlink();
  bool toggle();
  bool init();
  bool publishStatus();
  bool publishRules();
  bool publishName();
  bool saveStatusInEeprom();
  bool saveRulesInEeprom();

protected:
  bool saveNameInEeprom();
  bool publishParams();
  bool bBlink = false;
};

class PriseIOT_RF433 : public PriseIOT, public NexaTransmitter
{
  public:
  PriseIOT_RF433 (PubSubClient *iMqttClient, short iID, int iTxPin, unsigned long iTransmitterCode, char iPlugNumber, int iEepromStartAddr = 0) : PriseIOT(iMqttClient, iID, objType::priseRF, iEepromStartAddr), NexaTransmitter(iTxPin, iTransmitterCode){plugNumber = iPlugNumber;}
  bool turnOFF();
  bool turnON(bool iForce = false);

  private:
  char plugNumber;
};

class PriseIOT_ESP : public PriseIOT
{
  public:
    PriseIOT_ESP (PubSubClient *iMqttClient, short iID, int iEepromStartAddr = 0);
    bool turnON(bool iForce = false);
    bool turnOFF();
    bool blink();
    bool stopBlink();

  private:
    long lastBlink = 0;
};

class Sensor : public Objet
{
  public:
    Sensor(int iEepromStartAddr = 0):Objet(0, objType::sensor){eepromStartAddr = iEepromStartAddr;}
    Sensor(short iId, int iEepromStartAddr = 0) : Objet(iId, objType::sensor){eepromStartAddr = iEepromStartAddr;}
    Sensor(short iId, sensorType iSType, int iEepromStartAddr = 0) : Objet(iId, objType::sensor), sType(iSType){eepromStartAddr = iEepromStartAddr;}
    void setLog(Logging *iLog){pLog = iLog;}
    sensorType sType;

  protected:
    Logging *pLog = NULL;
};

class SensorIOT : public Sensor, public MyMQTTClient
{
public:
  SensorIOT(PubSubClient *iMqttClient, short iId, sensorType iSType, int iEepromStartAddr = 0);
  void handleMqttCallback(char* iTopic, byte* payload, unsigned int iLength);
  bool update(bool iForce = false);
  bool init();
  void setEnv(Environment *iEnv){pEnv = iEnv;}
  virtual float getValue() = 0;
  bool publishName();

protected:
  bool saveNameInEeprom();
  bool publishParams();

private:
  long lastUpdateTime = 0;
  Environment *pEnv = NULL;
};

class TempSensorAttiny : public SensorIOT
{
public:
  TempSensorAttiny(PubSubClient *iMqttClient, short iId, int iPinNumber, int iEepromStartAddr = 0) : SensorIOT(iMqttClient, iId, sensorType::temp, iEepromStartAddr){pinNumber = iPinNumber;}
  float getValue();
  bool init();
private:
  int pinNumber = 1;
};

class TempSensorDS18B20 : public SensorIOT
{
public:
  TempSensorDS18B20(PubSubClient *iMqttClient, short iId, int iPinNumber, int iEepromStartAddr = 0) : SensorIOT(iMqttClient, iId, sensorType::temp, iEepromStartAddr){pinNumber = iPinNumber;}
  float getValue();
  bool init();
private:
  OneWire *pOneWire = NULL;
  DallasTemperature *pSensors = NULL;
  int pinNumber = 1;
  DeviceAddress sensorAddress;
};

class ESP_Vcc : public SensorIOT
{
public:
  ESP_Vcc(PubSubClient *iMqttClient, short iId, int iEepromStartAddr = 0) : SensorIOT(iMqttClient, iId, sensorType::tension, iEepromStartAddr){}
  float getValue();
  //bool init();
};

class WeatherDisplayObj : public Objet
{
  public:
  WeatherDisplayObj() : Objet(0, objType::display){eepromStartAddr = 0;}
  WeatherDisplayObj(short iId, int iEepromStartAddr = 0) : Objet(iId, objType::display){eepromStartAddr = iEepromStartAddr;}
  String leftInfo = "Weather3";
  String rightInfo = "20010";
  int layout = 1;
  void init();
  bool saveInEeprom();
  void setLog(Logging *iLog){pLog = iLog;}

  protected:
    Logging *pLog = NULL;
};

#endif
