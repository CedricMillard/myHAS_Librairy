 /*
 * Root class for all objects requiring an MQTT connection
 * TODO:
 *  - Add topic list to subscribe to
 * (c) Cedric Millard 2020
 */

#ifndef MyMQTTClient_h
#define MyMQTTClient_h

#include "Config.h"
#include "ArrayCed.h"
#include <PubSubClient.h>

class MyMQTTClient
{
  public:
   MyMQTTClient(PubSubClient *iMqttClient, String iId);
   void addTopic(String iTopic);
   bool publishMsg(String iTopic, String iMessage, bool iRetained);
   void setMqttServer(String iMqttServer, int iMqttPort, String iMqttLogin, String iMqttPassword);
   virtual void handleMqttCallback(char* iTopic, byte* payload, unsigned int iLength) = 0;
   virtual bool update(bool iForce = false);


  protected:
    void checkMQTTConnection(bool force = false);
    bool mqttReconnect();
    virtual bool publishParams();

    unsigned long lastAttemptedMqttConnection = 0;
    unsigned long lastParamPublished = 0;
    PubSubClient *pMqttClient;
    String ID;
    Array<String> topicList;

  private:
    String mqttLogin;
    String mqttPassword;
};

#endif
