/*
*
*/


#ifndef Logging_h
#define Logging_h

#include <WString.h>

//log file path
#define LOG_FILE_PATH "/MyHAS_log.txt"
#define LOG_NB_LATEST_EVENTS 20
#define LOG_MAX_EVENTS 200
#define LOG_SIZE_BUFFER 50


class Logging
{
public:
  Logging(short iId);
  String getLogHTML();
  void addLogEntry(String iMsg);
  //void setEnv(Environment *iEnv){pEnv = iEnv;}

private:
  void cleanUpLog();
  int nbLinesInLog = 0;
  short sObjectID;
};

#endif
