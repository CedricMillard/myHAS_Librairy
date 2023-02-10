#include "Logging.h"
#include "Environment.h"
#include "FS.h"
#ifndef ESP8266
  #include "SPIFFS.h"
#endif
Logging::Logging(short iId) 
{
Serial.println("Logging::Logging start");
  sObjectID = iId;
  //Mount the filesystem
#ifdef ESP8266
  bool result = SPIFFS.begin();
#else
  bool result = SPIFFS.begin(true);
#endif
if(result)
  {
    File logFile = SPIFFS.open(LOG_FILE_PATH, "a");
    String logEntry = String(sObjectID) + " - ReStarted";
    logFile.println(logEntry.c_str());
    logFile.close();

    logFile = SPIFFS.open(LOG_FILE_PATH, "r");
    while(logFile.available()) 
    {
      logFile.readStringUntil('\n');
      nbLinesInLog++;
    }
    logFile.close();
  }
} 

String Logging::getLogHTML()
{
    String sLogText;
    File logFile = SPIFFS.open(LOG_FILE_PATH, "r");
    int currentLine = 0;
    while(logFile.available()) 
    {
      //read line by line from the file
      String buf = logFile.readStringUntil('\n') + "<br>";
      if(nbLinesInLog - currentLine<=LOG_NB_LATEST_EVENTS)
        sLogText = buf + sLogText;
      currentLine++;
    }
    sLogText = "FileSize : " + String(logFile.size()) + " NbRows=" +String(nbLinesInLog) + "<br>" + sLogText;

#ifdef ESP8266    
    FSInfo pFSInfo;
    SPIFFS.info(pFSInfo);
    sLogText = "<a href='#' onclick=\"if(document.getElementById('log').style.display=='none'){document.getElementById('log').style.display='block';this.innerHTML='Display logs<<';}else{document.getElementById('log').style.display='none';this.innerHTML='Display logs>>';}\">Display logs>></a><br>"
      "<div id='log' style=\"display:none;\"><a href='/log'>Full log</a><br>"
      "FileSystem space used: " + String(pFSInfo.usedBytes) + "/" + String(pFSInfo.totalBytes) +"<br>" + sLogText;
    sLogText += "</div>";
#else
    sLogText = "<a href='#' onclick=\"if(document.getElementById('log').style.display=='none'){document.getElementById('log').style.display='block';this.innerHTML='Display logs<<';}else{document.getElementById('log').style.display='none';this.innerHTML='Display logs>>';}\">Display logs>></a><br>"
      "<div id='log' style=\"display:none;\"><a href='/log'>Full log</a><br>"
      "FileSystem space used: " + String(SPIFFS.usedBytes()) + "/" + String(SPIFFS.totalBytes()) +"<br>" + sLogText;
    sLogText += "</div>";
#endif    
    logFile.close();
    
  return sLogText;
}

void Logging::addLogEntry(String iMsg)
{
  // open the file in write mode
    File logFile = SPIFFS.open(LOG_FILE_PATH, "a");
    String logEntry = String(sObjectID) + "-" + getDateShort() + "-" + getTimeFr() + "- " + iMsg;
    logFile.println(logEntry.c_str());
    logFile.close();
    nbLinesInLog++;
    if(nbLinesInLog>LOG_MAX_EVENTS)
      cleanUpLog();
}

/*
* Keep only 100 lines in the file to avoid overflow
*/
void Logging::cleanUpLog()
{
  if(nbLinesInLog>=LOG_MAX_EVENTS)
  {
    File logFile = SPIFFS.open(LOG_FILE_PATH, "r");
    File logFileNew = SPIFFS.open(String(LOG_FILE_PATH)+".new", "w");
    int currLine = 1;
    while(logFile.available()) 
    {
      //read line by line from the file
      String sbuf = logFile.readStringUntil('\n');
      if(currLine>nbLinesInLog-LOG_MAX_EVENTS+LOG_SIZE_BUFFER)
        logFileNew.println(sbuf);
      currLine++;
    }
    
    logFile.close();
    logFileNew.close();
    SPIFFS.remove(LOG_FILE_PATH);
    SPIFFS.rename(String(LOG_FILE_PATH)+".new", LOG_FILE_PATH);
    nbLinesInLog = LOG_MAX_EVENTS-LOG_SIZE_BUFFER;
  }
}
