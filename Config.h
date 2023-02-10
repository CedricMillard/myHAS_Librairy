#ifndef Config_h
#define Config_h

//Comment for not having serial trace (prod)
#define _DEBUG_

//Frequency to update the real time
#define REALTIME_UPDATE_FREQ 86400 //s = 24H

#define SENSOR_TOPIC "/sensor/#"
#define WEATHER_TOPIC_DAILY "/weather/daily"
#define WEATHER_TOPIC_HOURLY "/weather/hourly"
#define NET_TOPIC "/net/#"
#define PRISE_TOPIC "/prise/#"

#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.google.com"
#define NTP_SERVER_3 "time.windows.com"

#define SENSOR_UPDATE_FREQ 60 //seconds

#define PUBLISH_PARAMS_FREQ 3600 //seconds

#define BLINK_DELAY 0.5 //seconds

#define ADDRESS_STATUS 1
#define ADDRESS_NAME 2
#define ADDRESS_RULES 26
#define EEPROM_RULES_SIZE 450
#define ADDRESS_DISPLAY_NAME 1
#define ADDRESS_DISPLAY_RINFO 25
#define ADDRESS_DISPLAY_LINFO 35
#define ADDRESS_DISPLAY_LAYOUT 45
#define ADDRESS_SENSOR_NAME 1
#endif
