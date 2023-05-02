 /*
 * Library to gather all environmental data like real time and temperature
 * TODO:
 *  - Add a location search engine from https://locationiq.com/ for web server
 *  - Store all parameters in mqtt messages
 *  - (Make connection persistent to avoid timeout)
 * (c) Cedric Millard 2020
 */

#ifndef Environment_h
#define Environment_h

#include "Config.h"
#include "NexaTransmitter.h"
#include "MyMQTTClient.h"
#include "ArrayCed.h"
#include "Logging.h"

struct Weather
{
  float Tmax = -255;
  float Tmin = -255;
  float T6 = -255;
  short Weather = 0;
  float Wind = -255;
//  float Moon = -255;
  time_t updateTime = 0;
//  time_t Sunrise = 0;
//  time_t Sunset = 0;
};

bool operator==(const Weather& lhs, const Weather& rhs);

bool operator!=(const Weather& lhs, const Weather& rhs);

class Environment : public MyMQTTClient
{
  public:
    Environment(PubSubClient *iMqttClientEnv, short iId);
    float getTemperatureExtFromWeather();
    float getSensorValue(int iSensorId);
    void setSensorValue(int iSensorId, float iValue);
    void setWeatherDaily(String iWeatherJson);
    void setWeatherHourly(String iWeatherJson);
    String getSensorUnit(int iSensorId);
    float getTemperatureMorning();
    Weather getTodayWeather();
    Weather getTomorrowWeather();
    Weather getWeatherDay(int day);
    Weather getWeatherHour(long hour);
    bool update(bool iForce = false);
    void handleMqttCallback(char* iTopic, byte* payload, unsigned int iLength);
    time_t getSunsetTime();
    time_t getSunriseTime();
    void setLog(Logging *iLog){pLog = iLog;}
    void setLocation(float iLat, float iLong);
    bool isSunriseSunsetUptodate();
    void updateSunriseSunsetTime(bool iForce = false);

  private:
    bool setRealTime();
    
    unsigned long lastNTPUpdate = 0;
    float TemperatureExt = 100;
    float weatherLat = 59.449062;
    float weatherLong = 16.332060;
    Weather weather_d[4];
    Weather weather_h[24];
    Array<float> sensorValues;
    Array<String> sensorUnits;
    Logging *pLog = NULL;
    unsigned long sunsetTime = 0;
    unsigned long sunriseTime = 0;
    
};

String getDateFr();

String getDateShort();

String getWeekNumber();

String getTimeFr();

long getTimeSec();

uint8_t getDay();

time_t timegm(tm * tm);

#if defined(ESP8266)
//bool getLocalTime(struct tm * info, uint32_t ms=5000);
#endif

#endif
