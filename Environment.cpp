 /*
 * Library to gather all environmental data like real time and temperature
 * (c) Cedric Millard 2020
 */

#include "Environment.h"
#include <time.h>
#include <SunRise.h>
#if defined(ESP8266)
#include <TZ.h>
#endif
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <Arduino.h>

Environment::Environment(PubSubClient *iMqttClientEnv, short iId) : MyMQTTClient(iMqttClientEnv, String(iId) + "_Env")
{
  addTopic(SENSOR_TOPIC);
  addTopic(WEATHER_TOPIC_DAILY);
  addTopic(WEATHER_TOPIC_HOURLY);
  addTopic("/net/sensor/#");
}

void Environment::handleMqttCallback(char* iTopic, byte* payload, unsigned int iLength)
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

  String topic = iTopic;
  char *tempString = (char *) malloc(sizeof(char)*(iLength+1));
  memcpy(tempString, payload, iLength);
  tempString[iLength] = '\0';
  String sPayload(tempString);
  free(tempString);

  if(topic.indexOf("/sensor/")>=0)
  {
    //retrieve ID of the prise
    String sID = topic.substring(topic.indexOf("/sensor/")+8, topic.indexOf("/", topic.indexOf("/sensor/")+8));
    short iID = sID.toInt();
    if(topic.endsWith("/value"))
    {
      if(sPayload.length()>0)
        setSensorValue(iID, sPayload.toFloat());
      else
        setSensorValue(iID, -255);
    }
    if(topic.endsWith("/type"))
    {
      if(!sensorUnits.exists(iID))
      {
        switch(sPayload.toInt())
        {
          case 0:sensorUnits.add("*C", iID);break;
          case 2:sensorUnits.add("V", iID);break;
          case 3:sensorUnits.add("*C", iID);break;
          default: sensorUnits.add("", iID);break;
        }
      }
    }
  }

  if(topic==WEATHER_TOPIC_DAILY)
  {
    setWeatherDaily(sPayload);
  }

  if(topic==WEATHER_TOPIC_HOURLY)
  {
    setWeatherHourly(sPayload);
  }
}

bool Environment::update(bool iForce)
{
  bool result = true;

  checkMQTTConnection(iForce);

  if( lastNTPUpdate == 0 || (unsigned long)(millis()- lastNTPUpdate) > REALTIME_UPDATE_FREQ * 1000 || iForce)
  {
    if(!setRealTime()) result = false;
  }
  
  updateSunriseSunsetTime(iForce);

  return result;
}

bool Environment::setRealTime()
{
#if defined(ESP8266)
  configTime(TZ_Europe_Amsterdam, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
#else
  configTime(3600, 3600, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
  setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
  tzset();
#endif

  

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo,5000U)){
#ifdef _DEBUG_
  Serial.println("Connection to NTP server failed");
#endif
    pLog->addLogEntry("Connection to NTP server failed");
    return false;
  }

#ifdef _DEBUG_
  Serial.println("Time updated from NTP");
#endif
  pLog->addLogEntry("Time updated from NTP");
  lastNTPUpdate = millis();
  return true;
}

void Environment::setLocation(float iLat, float iLong)
{
  weatherLat = iLat;
  weatherLong = iLong;
}

float Environment::getTemperatureExtFromWeather()
{
  return getWeatherHour(time(nullptr)).Tmax;
}

float Environment::getSensorValue(int iSensorId)
{
  if(sensorValues.exists(iSensorId))
    return sensorValues.getItem(iSensorId);
  else
    return -255;
}

void Environment::setSensorValue(int iSensorId, float iValue)
{
  if(!sensorValues.exists(iSensorId))
  {
    sensorValues.add(iValue, iSensorId);
  }
  else sensorValues.getItem(iSensorId) = iValue;
}

void Environment::setWeatherDaily(String iWeatherJson)
{
  const size_t capacity = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(2) + 4*JSON_OBJECT_SIZE(7)+200;
  DynamicJsonDocument doc(capacity);
DeserializationError toto;
  if(deserializeJson(doc, iWeatherJson)!=DeserializationError::Ok)
    Serial.printf("\nERROR ERROR ERROR DeserializationError %d\n%s\n\n", deserializeJson(doc, iWeatherJson), iWeatherJson.c_str());

  JsonArray daily = doc["daily"];

  long updated = doc["updated"];

  for(int i=0; i<4; i++)
  {
    JsonObject currentDay = daily[i];
    weather_d[i].Weather = (short) currentDay["weather"];
    weather_d[i].Tmax = (float)currentDay["Tmax"];
    weather_d[i].Tmin = (float)currentDay["Tmin"];
    weather_d[i].Wind = (float)currentDay["wind"];
    weather_d[i].Moon = (float)currentDay["moon"];
    weather_d[i].Sunrise = (unsigned long)currentDay["sunrise"];
    weather_d[i].Sunset = (unsigned long)currentDay["sunset"];
    weather_d[i].updateTime = updated;
  }
}

void Environment::setWeatherHourly(String iWeatherJson)
{
  const size_t capacity = JSON_ARRAY_SIZE(14) + JSON_OBJECT_SIZE(2) + 14*JSON_OBJECT_SIZE(7)+500;
  DynamicJsonDocument doc(capacity);
  
  if(deserializeJson(doc, iWeatherJson)!=DeserializationError::Ok)
    Serial.printf("\nERROR ERROR ERROR DeserializationError %d\n%s\n\n", deserializeJson(doc, iWeatherJson), iWeatherJson.c_str());

  JsonArray hourly = doc["hourly"];

  for(int i=0; i<14; i++)
  {
    JsonObject currentHour = hourly[i];
    weather_h[i].Weather = (short) currentHour["weather"];
    weather_h[i].Tmax = weather_h[i].Tmin = (float)currentHour["Temp"];
    weather_h[i].Wind = (float)currentHour["wind"];
    weather_h[i].updateTime = currentHour["time"];
  }
}

String Environment::getSensorUnit(int iSensorId)
{
  if(sensorUnits.exists(iSensorId))
    return sensorUnits.getItem(iSensorId);
  else
    return "";
}

float Environment::getTemperatureMorning()
{
  return getTodayWeather().Tmin;
}

Weather Environment::getTodayWeather()
{
  return getWeatherDay(0);
}

Weather Environment::getTomorrowWeather()
{
  return getWeatherDay(1);
}

Weather Environment::getWeatherDay(int day)
{
  Weather emptyWeather;
  if (day>=4 || weather_d[0].updateTime == 0)
  {
    return emptyWeather;
  }

  struct tm updatedDay = *localtime(&(weather_d[0].updateTime));
  updatedDay.tm_hour = 0; updatedDay.tm_min = 0; updatedDay.tm_sec = 0;

  long delta = difftime(time(nullptr), mktime(&updatedDay));

  //Weather updated today
  if (delta < 86400) return weather_d[day];
  //Weather was updated yesterday
  if (delta < 172800 && day<3) return weather_d[day+1];
  if (delta < 259200 && day<2) return weather_d[day+2];
  if (delta < 345600 && day<1) return weather_d[day+3];
  return emptyWeather;
}

Weather Environment::getWeatherHour(long hour)
{
  Weather emptyWeather;
  
  //Compute time index
  int index = (hour - weather_h[0].updateTime)/3600;

  if (index<0 || index > 13)
  {
    return emptyWeather;
  }

  return  weather_h[index];
}

bool Environment::isSunriseSunsetUptodate()
{
  //Time in UTC
  time_t t = time (nullptr);
  tm * srTM_midnight = localtime(&t);
  srTM_midnight->tm_hour = 0;
  srTM_midnight->tm_min = 0;
  srTM_midnight->tm_sec = 0;

  time_t t_midnight = mktime(srTM_midnight);
  return difftime(getSunriseTime(), t_midnight) > 0;
}

void Environment::updateSunriseSunsetTime(bool iForce)
{
  //Time in UTC
  time_t t = time (nullptr);
  
  if(!isSunriseSunsetUptodate() || iForce)
  {
    pLog->addLogEntry("Update Sunrise and Sunset time");
    tm * srTM = localtime(&t);
    
    //Calculate local epoch time
  #if defined(ESP8266)
    time_t gmtT = mktime(gmtime(&t));
    if(srTM->tm_isdst) gmtT-=3600;
    time_t t2 = t + t - gmtT;
  #else
    setenv("TZ", "GMT0",1);
    time_t t2 = mktime(srTM);
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
  #endif
    
    //local epoch time at midnight
    time_t t0 = t2 / (24*3600);
    
    //local epoch time at noon
    t2 = t2 - t2%(24*3600) + 12*3600;

    SunRise sr;
    sr.calculate(weatherLat, weatherLong, t2);
    sunsetTime = sr.setTime;
    sunriseTime = sr.riseTime;
  }
}

time_t Environment::getSunsetTime()
{
  return sunsetTime;
}

time_t Environment::getSunriseTime()
{
  return sunriseTime;
}

String getWeekNumber()
{
  String out = "W";
  struct tm timeinfo;
  char sTime[5];
   if(!getLocalTime(&timeinfo,5000U)){
#ifdef _DEBUG_
  Serial.println("RealTime not set");
#endif
    return out;
  }
  strftime(sTime, 5, "%V", &timeinfo);
  out += sTime;
  return out;
}

String getTimeFr()
{
  String out;
  struct tm timeinfo;
  char sTime[10];
   if(!getLocalTime(&timeinfo,5000U)){
#ifdef _DEBUG_
  Serial.println("RealTime not set");
#endif
    return out;
  }
  strftime(sTime, 10, "%R", &timeinfo);
  out = sTime;
  return out;
}

long getTimeSec()
{
  long out = -1;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo,5000U)){
#ifdef _DEBUG_
  Serial.println("RealTime not set");
#endif
    return out;
  }
  out = timeinfo.tm_sec + timeinfo.tm_min * 60 + timeinfo.tm_hour * 3600;

  return out;
}

String getDateFr()
{
  String out;
  struct tm timeinfo;
  char sTime[20];
   if(!getLocalTime(&timeinfo,5000U)){
#ifdef _DEBUG_
  Serial.println("RealTime not set");
#endif
    return out;
  }
  strftime(sTime, 20, "%a %d %b", &timeinfo);
  out = sTime;
  return out;
}

String getDateShort()
{
  String out;
  struct tm timeinfo;
  char sTime[6];
   if(!getLocalTime(&timeinfo,5000U)){
#ifdef _DEBUG_
  Serial.println("RealTime not set");
#endif
    return out;
  }
  strftime(sTime, 6, "%d/%m", &timeinfo);
  out = sTime;
  return out;
}

uint8_t getDay()
{
  uint8_t day;
  struct tm timeinfo;
  char sTime[2];
   if(!getLocalTime(&timeinfo,5000U)){
#ifdef _DEBUG_
  Serial.println("RealTime not set");
#endif
    return 0;
  }
  strftime(sTime, 2, "%w", &timeinfo);
  day = atoi(sTime);
  return day;
}


bool operator==(const Weather& lhs, const Weather& rhs)
{
    bool result = true;
    if(lhs.Tmax!=rhs.Tmax) result = false;
    if(lhs.Tmin!=rhs.Tmin) result = false;
    if(lhs.Weather!=rhs.Weather) result = false;
    if(lhs.Wind!=rhs.Wind) result = false;
    if(lhs.Moon!=rhs.Moon) result = false;

    return result;
}

bool operator!=(const Weather& lhs, const Weather& rhs)
{
    return !(lhs==rhs);
}

#if defined(ESP8266)
/*bool getLocalTime(struct tm * info, uint32_t ms=5000U)
{
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= ms) {
        time(&now);
        localtime_r(&now, info);
        if(info->tm_year > (2016 - 1900)){
            return true;
        }
        delay(10);
    }
    return false;
}*/
#endif
