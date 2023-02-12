 /*
 * Library to gather all environmental data like real time and temperature
 * (c) Cedric Millard 2020
 */

#include "MyMQTTClient.h"

bool MyMQTTClient::mqttReconnect() {

  bool bResult = true;

  if (!pMqttClient->connected()) {
#ifdef _DEBUG_
    Serial.print("Attempting MQTT connection...");
#endif
    // Create a random client ID
    String clientId = ID;
    // Attempt to connect
    if (pMqttClient->connect(clientId.c_str(), mqttLogin, mqttPassword, false)) {
#ifdef _DEBUG_
      Serial.print(clientId);
      Serial.println(" connected");
#endif
      for(int i=0; i<topicList.size(); i++)
      {
        pMqttClient->subscribe(topicList[i].c_str(), 1);
      }

      publishParams();
      lastParamPublished = millis();

    } else {
#ifdef _DEBUG_
      Serial.printf(" failed, rc=%d trying again in 30 seconds\n", pMqttClient->state());
#endif
      bResult = false;
    }
  }
  lastAttemptedMqttConnection = millis();
  return bResult;
}


MyMQTTClient::MyMQTTClient(PubSubClient *iMqttClient, String iId)
{
  pMqttClient = iMqttClient;
  ID = iId;
}

void MyMQTTClient::checkMQTTConnection(bool force)
{
  if (!pMqttClient->connected()) {
    if( lastAttemptedMqttConnection == 0 || (unsigned long)(millis()-lastAttemptedMqttConnection) > 30000 || force)
      mqttReconnect();
  }
  else
    pMqttClient->loop();
}

bool MyMQTTClient::publishMsg(String iTopic, String iMessage, bool iRetained)
{
  checkMQTTConnection(true);
  pMqttClient->publish(iTopic.c_str(), iMessage.c_str(), iRetained);
}

bool MyMQTTClient::update(bool iForce)
{
    checkMQTTConnection(iForce);
    if((unsigned long)(millis()-lastParamPublished)>PUBLISH_PARAMS_FREQ*1000)
    {
      lastParamPublished = millis();
      publishParams();
    }
    return true;
}

void MyMQTTClient::addTopic(String iTopic)
{
  topicList.add(iTopic);
}

bool MyMQTTClient::publishParams()
{
  #ifdef _DEBUG_
      Serial.println("MyMQTTClient::publishParams()");
  #endif
  return false;
}

void MyMQTTClient::setMqttServer(const char * iMqttServer, int iMqttPort, const char * iMqttLogin, const char * iMqttPassword)
{
  pMqttClient->setServer(iMqttServer, iMqttPort);
  mqttLogin = iMqttLogin;
  mqttPassword = iMqttPassword;
}
