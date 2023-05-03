#include "WebPage.h"
#include "FS.h"
#ifndef ESP8266
  #include "SPIFFS.h"
#endif

String WebPage::getIndexHTML()
{
  String htmlPage = FPSTR(checkbox_style);
  htmlPage +=FPSTR(index_html);
  htmlPage.replace("#TITLE#", sTitle);
  htmlPage.replace("#TIME#", getTimeFr());
  htmlPage.replace("#DATE#", getDateFr());
  for(int i = 0; i<aPrises.size();i++)
  {
    String Prise = FPSTR(index_html_switch);
    if(aPrises[i]->status) Prise.replace("#STATUS#", "checked");
    else Prise.replace("#STATUS#", "");
    Prise.replace("#NAME#", aPrises[i]->name);
    Prise.replace("#ID#", String(aPrises[i]->Id));
    int nbActiveRules = aPrises[i]->getNbActiveRules();
    int nbRules = aPrises[i]->aRules.size();
    String ruleLabel = String(nbActiveRules)+"/"+String(nbRules)+" Rules";
    if(nbRules ==0) ruleLabel = "No Rules";
    Prise.replace("#NB_RULES#",ruleLabel);
    htmlPage+=Prise;
  }
  
  if(aSensors.size())
    htmlPage +="<hr>";

  for(int i = 0; i<aSensors.size();i++)
  {
    String Sensor =FPSTR(index_html_sensor);
    Sensor.replace("#NAME#", aSensors[i]->name);
    Sensor.replace("#ID#", String(aSensors[i]->Id));
    if(pEnv)
      Sensor.replace("#SENSOR_VALUE#", String(pEnv->getSensorValue(aSensors[i]->Id))+pEnv->getSensorUnit(aSensors[i]->Id));
    else
      Sensor.replace("#SENSOR_VALUE#", "");

    Sensor.replace("*", "&deg;");
    switch(aSensors[i]->sType)
    {
      case temp: Sensor.replace("#BTN_VISIBLE#", ""); htmlPage+=temp_icon; break;
      case tension: Sensor.replace("#BTN_VISIBLE#", ""); htmlPage+=bat_icon; break;
      case temp_ro: Sensor.replace("#BTN_VISIBLE#", "disabled"); htmlPage+=temp_icon; break;
      default: Sensor.replace("#BTN_VISIBLE#", ""); htmlPage+=temp_icon; break;
    }
    htmlPage+=Sensor;
  }

  if(aDisplays.size())
    htmlPage +="<hr>";

  for(int j=0; j<aDisplays.size();j++)
  {
    String sSensorOptionsL= "";
    String sSensorOptionsR= "";

    for(int i = 0; i<aSensors.size();i++)
    {
      String sO(FPSTR(weather_option));

      //<option value="#SENSOR_ID#" #_SELECTED_#>#SENSOR_NAME#</option>
      String sensorId = String(aSensors[i]->Id);
      sO.replace("#SENSOR_ID#", sensorId);
      sO.replace("#SENSOR_NAME#", aSensors[i]->name);
      sSensorOptionsL+=sO;
      sSensorOptionsR += sO;

      if(aDisplays[j]->leftInfo==sensorId) sSensorOptionsL.replace("#_SELECTED_#", "selected");
      else sSensorOptionsL.replace("#_SELECTED_#", "");

      if(aDisplays[j]->rightInfo==sensorId) sSensorOptionsR.replace("#_SELECTED_#", "selected");
      else sSensorOptionsR.replace("#_SELECTED_#", "");

    }

    String sWeatherDisp = weatherSta_icon + String(FPSTR(index_html_weatherDisp));
    sWeatherDisp.replace("#ID#", String(aDisplays[j]->Id));
    sWeatherDisp.replace("#NAME#", String(aDisplays[j]->name));
    sWeatherDisp.replace("#SENSOR_LIST_L#",sSensorOptionsL);
    sWeatherDisp.replace("#SENSOR_LIST_R#",sSensorOptionsR);
    if(aDisplays[j]->leftInfo=="Weather3") sWeatherDisp.replace("#W_SELECTED_L#","selected");
    else sWeatherDisp.replace("#W_SELECTED_L#","");
    if(aDisplays[j]->rightInfo=="Weather3") sWeatherDisp.replace("#W_SELECTED_R#","selected");
    else sWeatherDisp.replace("#W_SELECTED_R#","");
    if(aDisplays[j]->layout==1) sWeatherDisp.replace("#W_SELECTED_LAYOUT1#","selected");
    if(aDisplays[j]->layout==2) sWeatherDisp.replace("#W_SELECTED_LAYOUT2#","selected");
    if(aDisplays[j]->layout==3) sWeatherDisp.replace("#W_SELECTED_LAYOUT3#","selected");
    sWeatherDisp.replace("#W_SELECTED_LAYOUT1#","");
    sWeatherDisp.replace("#W_SELECTED_LAYOUT2#","");
    sWeatherDisp.replace("#W_SELECTED_LAYOUT3#","");

    htmlPage+=sWeatherDisp;
  }
  
  htmlPage +="<hr>";
  
  htmlPage+=pLog->getLogHTML();

  htmlPage+=FPSTR(index_html_footer);
  return htmlPage;
}

String WebPage::getIndexHTML_file()
{
#ifdef ESP8266
  bool result = SPIFFS.begin();
#else
  bool result = SPIFFS.begin(true);
#endif

  if(!result) return "";

  File htmlFile = SPIFFS.open(INDEX_FILE_PATH, "w");
  
  String htmlPage = FPSTR(checkbox_style);
  htmlPage +=FPSTR(index_html);
  htmlPage.replace("#TITLE#", sTitle);
  htmlPage.replace("#TIME#", getTimeFr());
  htmlPage.replace("#DATE#", getDateFr());
  htmlFile.println(htmlPage);
  for(int i = 0; i<aPrises.size();i++)
  {
    String Prise = FPSTR(index_html_switch);
    if(aPrises[i]->status) Prise.replace("#STATUS#", "checked");
    else Prise.replace("#STATUS#", "");
    Prise.replace("#NAME#", aPrises[i]->name);
    Prise.replace("#ID#", String(aPrises[i]->Id));
    int nbActiveRules = aPrises[i]->getNbActiveRules();
    int nbRules = aPrises[i]->aRules.size();
    String ruleLabel = String(nbActiveRules)+"/"+String(nbRules)+" Rules";
    if(nbRules ==0) ruleLabel = "No Rules";
    Prise.replace("#NB_RULES#",ruleLabel);
    htmlFile.println(Prise);
  }
  
  if(aSensors.size())
    htmlFile.println("<hr>");

  for(int i = 0; i<aSensors.size();i++)
  {
    String Sensor =FPSTR(index_html_sensor);
    Sensor.replace("#NAME#", aSensors[i]->name);
    Sensor.replace("#ID#", String(aSensors[i]->Id));
    if(pEnv)
      Sensor.replace("#SENSOR_VALUE#", String(pEnv->getSensorValue(aSensors[i]->Id))+pEnv->getSensorUnit(aSensors[i]->Id));
    else
      Sensor.replace("#SENSOR_VALUE#", "");

    Sensor.replace("*", "&deg;");
    switch(aSensors[i]->sType)
    {
      case temp: Sensor.replace("#BTN_VISIBLE#", ""); htmlFile.println(temp_icon); break;
      case tension: Sensor.replace("#BTN_VISIBLE#", ""); htmlFile.println(bat_icon); break;
      case temp_ro: Sensor.replace("#BTN_VISIBLE#", "disabled"); htmlFile.println(temp_icon); break;
      default: Sensor.replace("#BTN_VISIBLE#", ""); htmlFile.println(temp_icon); break;
    }
    htmlFile.println(Sensor);
  }

  if(aDisplays.size())
    htmlFile.println("<hr>");

  for(int j=0; j<aDisplays.size();j++)
  {
    String sSensorOptionsL= "";
    String sSensorOptionsR= "";

    for(int i = 0; i<aSensors.size();i++)
    {
      String sO(FPSTR(weather_option));

      //<option value="#SENSOR_ID#" #_SELECTED_#>#SENSOR_NAME#</option>
      String sensorId = String(aSensors[i]->Id);
      sO.replace("#SENSOR_ID#", sensorId);
      sO.replace("#SENSOR_NAME#", aSensors[i]->name);
      sSensorOptionsL+=sO;
      sSensorOptionsR += sO;

      if(aDisplays[j]->leftInfo==sensorId) sSensorOptionsL.replace("#_SELECTED_#", "selected");
      else sSensorOptionsL.replace("#_SELECTED_#", "");

      if(aDisplays[j]->rightInfo==sensorId) sSensorOptionsR.replace("#_SELECTED_#", "selected");
      else sSensorOptionsR.replace("#_SELECTED_#", "");

    }

    String sWeatherDisp = weatherSta_icon + String(FPSTR(index_html_weatherDisp));
    sWeatherDisp.replace("#ID#", String(aDisplays[j]->Id));
    sWeatherDisp.replace("#NAME#", String(aDisplays[j]->name));
    sWeatherDisp.replace("#SENSOR_LIST_L#",sSensorOptionsL);
    sWeatherDisp.replace("#SENSOR_LIST_R#",sSensorOptionsR);
    if(aDisplays[j]->leftInfo=="Weather3") sWeatherDisp.replace("#W_SELECTED_L#","selected");
    else sWeatherDisp.replace("#W_SELECTED_L#","");
    if(aDisplays[j]->rightInfo=="Weather3") sWeatherDisp.replace("#W_SELECTED_R#","selected");
    else sWeatherDisp.replace("#W_SELECTED_R#","");
    if(aDisplays[j]->layout==1) sWeatherDisp.replace("#W_SELECTED_LAYOUT1#","selected");
    if(aDisplays[j]->layout==2) sWeatherDisp.replace("#W_SELECTED_LAYOUT2#","selected");
    if(aDisplays[j]->layout==3) sWeatherDisp.replace("#W_SELECTED_LAYOUT3#","selected");
    sWeatherDisp.replace("#W_SELECTED_LAYOUT1#","");
    sWeatherDisp.replace("#W_SELECTED_LAYOUT2#","");
    sWeatherDisp.replace("#W_SELECTED_LAYOUT3#","");

    htmlFile.println(sWeatherDisp);
  }
  
  htmlFile.println("<hr>");
  htmlFile.println(pLog->getLogHTML());
  htmlFile.println(FPSTR(index_html_footer));
  htmlFile.close();

  return INDEX_FILE_PATH;
}

String WebPage::getRulesHTML(int iID)
{
  String htmlPage = FPSTR(checkbox_style);
  htmlPage += FPSTR(rules_html_header);

  htmlPage.replace("#ID#", String(iID));
  //Retrieve rules
  Prise *pPrise = aPrises.getItem(iID);
  int iNbRules = pPrise->aRules.size();
  htmlPage.replace("#NB_RULES#", String(iNbRules));
  htmlPage.replace("#PRISE_NAME#",pPrise->name);
  for(int i = -1; i<iNbRules;i++)
  {
    String sRule = FPSTR(rules_html_rule);
    sRule.replace("#I#", String(i+1));
    //frequency
    Rule *pCurrentRule = NULL;

    if(i>=0)
    {
      pCurrentRule = &(pPrise->aRules[i]);
      uint8_t freq = pCurrentRule->frequency; //pPrise->aRules[i].frequency;
      if(freq & 1) sRule.replace("#SUN_CHECKED#", "checked");
      if(freq & 2) sRule.replace("#MON_CHECKED#", "checked");
      if(freq & 4) sRule.replace("#TUE_CHECKED#", "checked");
      if(freq & 8) sRule.replace("#WED_CHECKED#", "checked");
      if(freq & 16) sRule.replace("#THU_CHECKED#", "checked");
      if(freq & 32) sRule.replace("#FRI_CHECKED#", "checked");
      if(freq & 64) sRule.replace("#SAT_CHECKED#", "checked");
      sRule.replace("#FREQ#", String(freq));
    }
    sRule.replace("#SUN_CHECKED#", "");
    sRule.replace("#MON_CHECKED#", "");
    sRule.replace("#TUE_CHECKED#", "");
    sRule.replace("#WED_CHECKED#", "");
    sRule.replace("#THU_CHECKED#", "");
    sRule.replace("#FRI_CHECKED#", "");
    sRule.replace("#SAT_CHECKED#", "");
    sRule.replace("#FREQ#", "0");

    //condition
    if(i>=0)
    {
      int j = 0;
      String sCondHTML;
      String sCond = pCurrentRule->condition;//pPrise->aRules[i].condition;
      sRule.replace("#RULE#", sCond);
      sRule.replace("#FORM_VISIBLE#", "");
      CreateCondition(sCondHTML, sCond, j);
      sRule.replace("#CONDITION#", sCondHTML);
      sRule.replace("#I#", String(i+1));
      sRule.replace("#NB_COND#", String(j));
    }
    else
    {
      sRule.replace("#CONDITION#", FPSTR(condition_html_rule));
      sRule.replace("#EXTRA_CONDITIONS#", getExtraConditions());
      sRule.replace("#TEMP_SENSORS#", getCarWSensors());
      sRule.replace("#I#", String(i+1));
      sRule.replace("#J#", "1");
      sRule.replace("#H_SELECTED#", "selected");
      sRule.replace("#EQ_SELECTED#", "selected");
      sRule.replace("#INF_SELECTED#", "");
      sRule.replace("#SUP_SELECTED#", "");
      sRule.replace("#TIME_TYPE#", "time");
      sRule.replace("#TIME#", getTimeFr());
      sRule.replace("#RULE#", "H="+String(getTimeSec()));
      sRule.replace("#NB_COND#", String(1));
    }
    //Remove all not already replaced tags
    sRule.replace("#TE_SELECTED#", "");
    sRule.replace("#H_SELECTED#", "");
    sRule.replace("#TM_SELECTED#", "");
    sRule.replace("#CARW_SELECTED#", "");
    sRule.replace("#EQ_SELECTED#", "");
    sRule.replace("#SUP_SELECTED#", "");
    sRule.replace("#INF_SELECTED#", "");
    sRule.replace("#TIME#", "");
    sRule.replace("#TEMP#", "");
    sRule.replace("#START_TIME#", "0");
    sRule.replace("#END_TIME#", "288");
    sRule.replace("#T_LABEL_STYLE#", "style='display:none;'");
    sRule.replace("#TIME_TYPE#", "hidden");
    sRule.replace("#NUMBER_TYPE#", "hidden");
    sRule.replace("#OP_VISIBLE#", "style='display:none;'");
    sRule.replace("#FORM_VISIBLE#", "style='display:none;'");
    sRule.replace("#RANGE_VISIBLE#", "style='display:none;'");
    sRule.replace("#CARWT_VISIBLE#", "style='display:none;'");
    sRule.replace("#TEMP_SENSORS#", getCarWSensors());
    sRule.replace("#I#", String(i+1));
    sRule.replace("#J#", "1");
    
    //action
    if(i>=0)
    {
      switch( pCurrentRule->action/*pPrise->aRules[i].action*/)
      {
        case turnOn: sRule.replace("#ON_CHECKED#", "checked");break;
        //case turnOff: sRule.replace("#OFF_CHECKED#", "checked");break;
        case blink: sRule.replace("#BLINK_CHECKED#", "checked");break;
      }
    }
    else sRule.replace("#ON_CHECKED#", "checked");
    sRule.replace("#ON_CHECKED#", "");
    sRule.replace("#BLINK_CHECKED#", "");

    // Active
    if(i>=0)
    {
      if(!pCurrentRule->active/*pPrise->aRules[i].active*/)
      {
        sRule.replace("#STATUS_CHECKED#", "");
        sRule.replace("#STATUS#", "disabled");
      }
    }
    sRule.replace("#STATUS_CHECKED#", "checked");
    sRule.replace("#STATUS#", "active");

    htmlPage+=sRule;
  }
  if(pPrise->type == priseWifi)
    htmlPage.replace("#BLINK_ENABLED#", "");
  else
    htmlPage.replace("#BLINK_ENABLED#", "style='display:none;'");

  htmlPage+=FPSTR(rules_html_footer);
  return htmlPage;
}

String WebPage::getRulesHTML_file(int iID)
{
  #ifdef ESP8266
  bool result = SPIFFS.begin();
#else
  bool result = SPIFFS.begin(true);
#endif

  if(!result) return "";

  File htmlFile = SPIFFS.open(RULES_FILE_PATH, "w");

  String htmlPage = FPSTR(checkbox_style);
  htmlPage += FPSTR(rules_html_header);

  htmlPage.replace("#ID#", String(iID));
  //Retrieve rules
  Prise *pPrise = aPrises.getItem(iID);
  int iNbRules = pPrise->aRules.size();
  htmlPage.replace("#NB_RULES#", String(iNbRules));
  htmlPage.replace("#PRISE_NAME#",pPrise->name);
  
  htmlFile.println(htmlPage);

  for(int i = -1; i<iNbRules;i++)
  {
    String sRule = FPSTR(rules_html_rule);
    sRule.replace("#I#", String(i+1));
    //frequency
    Rule *pCurrentRule = NULL;

    if(i>=0)
    {
      pCurrentRule = &(pPrise->aRules[i]);
      uint8_t freq = pCurrentRule->frequency; //pPrise->aRules[i].frequency;
      if(freq & 1) sRule.replace("#SUN_CHECKED#", "checked");
      if(freq & 2) sRule.replace("#MON_CHECKED#", "checked");
      if(freq & 4) sRule.replace("#TUE_CHECKED#", "checked");
      if(freq & 8) sRule.replace("#WED_CHECKED#", "checked");
      if(freq & 16) sRule.replace("#THU_CHECKED#", "checked");
      if(freq & 32) sRule.replace("#FRI_CHECKED#", "checked");
      if(freq & 64) sRule.replace("#SAT_CHECKED#", "checked");
      sRule.replace("#FREQ#", String(freq));
    }
    sRule.replace("#SUN_CHECKED#", "");
    sRule.replace("#MON_CHECKED#", "");
    sRule.replace("#TUE_CHECKED#", "");
    sRule.replace("#WED_CHECKED#", "");
    sRule.replace("#THU_CHECKED#", "");
    sRule.replace("#FRI_CHECKED#", "");
    sRule.replace("#SAT_CHECKED#", "");
    sRule.replace("#FREQ#", "0");

    //condition
    if(i>=0)
    {
      int j = 0;
      String sCondHTML;
      String sCond = pCurrentRule->condition;//pPrise->aRules[i].condition;
      sRule.replace("#RULE#", sCond);
      sRule.replace("#FORM_VISIBLE#", "");
      CreateCondition(sCondHTML, sCond, j);
      sRule.replace("#CONDITION#", sCondHTML);
      sRule.replace("#I#", String(i+1));
      sRule.replace("#NB_COND#", String(j));
    }
    else
    {
      sRule.replace("#CONDITION#", FPSTR(condition_html_rule));
      sRule.replace("#EXTRA_CONDITIONS#", getExtraConditions());
      sRule.replace("#TEMP_SENSORS#", getCarWSensors());
      sRule.replace("#I#", String(i+1));
      sRule.replace("#J#", "1");
      sRule.replace("#H_SELECTED#", "selected");
      sRule.replace("#EQ_SELECTED#", "selected");
      sRule.replace("#INF_SELECTED#", "");
      sRule.replace("#SUP_SELECTED#", "");
      sRule.replace("#TIME_TYPE#", "time");
      sRule.replace("#TIME#", getTimeFr());
      sRule.replace("#RULE#", "H="+String(getTimeSec()));
      sRule.replace("#NB_COND#", String(1));
    }
    //Remove all not already replaced tags
    sRule.replace("#TE_SELECTED#", "");
    sRule.replace("#H_SELECTED#", "");
    sRule.replace("#TM_SELECTED#", "");
    sRule.replace("#CARW_SELECTED#", "");
    sRule.replace("#EQ_SELECTED#", "");
    sRule.replace("#SUP_SELECTED#", "");
    sRule.replace("#INF_SELECTED#", "");
    sRule.replace("#TIME#", "");
    sRule.replace("#TEMP#", "");
    sRule.replace("#START_TIME#", "0");
    sRule.replace("#END_TIME#", "288");
    sRule.replace("#T_LABEL_STYLE#", "style='display:none;'");
    sRule.replace("#TIME_TYPE#", "hidden");
    sRule.replace("#NUMBER_TYPE#", "hidden");
    sRule.replace("#OP_VISIBLE#", "style='display:none;'");
    sRule.replace("#FORM_VISIBLE#", "style='display:none;'");
    sRule.replace("#RANGE_VISIBLE#", "style='display:none;'");
    sRule.replace("#CARWT_VISIBLE#", "style='display:none;'");
    sRule.replace("#TEMP_SENSORS#", getCarWSensors());
    sRule.replace("#I#", String(i+1));
    sRule.replace("#J#", "1");
    
    //action
    if(i>=0)
    {
      switch( pCurrentRule->action/*pPrise->aRules[i].action*/)
      {
        case turnOn: sRule.replace("#ON_CHECKED#", "checked");break;
        //case turnOff: sRule.replace("#OFF_CHECKED#", "checked");break;
        case blink: sRule.replace("#BLINK_CHECKED#", "checked");break;
      }
    }
    else sRule.replace("#ON_CHECKED#", "checked");
    sRule.replace("#ON_CHECKED#", "");
    sRule.replace("#BLINK_CHECKED#", "");

    // Active
    if(i>=0)
    {
      if(!pCurrentRule->active/*pPrise->aRules[i].active*/)
      {
        sRule.replace("#STATUS_CHECKED#", "");
        sRule.replace("#STATUS#", "disabled");
      }
    }
    sRule.replace("#STATUS_CHECKED#", "checked");
    sRule.replace("#STATUS#", "active");
    if(pPrise->type == priseWifi)
      sRule.replace("#BLINK_ENABLED#", "");
    else
      sRule.replace("#BLINK_ENABLED#", "style='display:none;'");

    htmlFile.println(sRule);
  }
    
  htmlFile.println(FPSTR(rules_html_footer));
  htmlFile.close();

  return RULES_FILE_PATH;
}

void WebPage::CreateCondition(String &ioHTML, String iCondition, int& j)
{
  if(iCondition.indexOf(";")!=-1)
  {
    String cond1 = iCondition.substring(0, iCondition.indexOf(";"));
    String cond2 = iCondition.substring(iCondition.indexOf(";")+1);
    CreateCondition(ioHTML, cond1, j);
    CreateCondition(ioHTML, cond2, j);
  }
  else
  {
    j++;
    String sCond = FPSTR(condition_html_rule);
    sCond.replace("#J#", String(j));
    if(iCondition.indexOf("<H<")>0)
    {
      sCond.replace("#H_SELECTED#", "selected");
      sCond.replace("#RANGE_VISIBLE#", "style='display:inline;'");
      sCond.replace("#OP_VISIBLE#", "style='display:none;'");
            
      sCond.replace("#START_TIME#", iCondition.substring(0, iCondition.indexOf("<")));
      sCond.replace("#END_TIME#", iCondition.substring(iCondition.lastIndexOf("<")+1));
    }
    else if(iCondition.startsWith("CarW"))
    {
      sCond.replace("#CARW_SELECTED#", "selected");
      sCond.replace("#EQ_SELECTED#", "selected");
      sCond.replace("#INF_SELECTED#", "disabled");
      sCond.replace("#SUP_SELECTED#", "disabled");
      sCond.replace("#CARWT_VISIBLE#", "style='display:inline;'");
         
      sCond.replace("#TIME_TYPE#", "time");

      long targetTime = iCondition.substring(iCondition.lastIndexOf("/")+1).toInt();
      String hour = String(targetTime/3600);
      if(hour.length()<2) hour = "0" + hour;
      String minute = String((targetTime % 3600)/60);
      if(minute.length()<2) minute = "0" + minute;
      sCond.replace("#TIME#", hour+":"+minute);
      sCond.replace("#OP_VISIBLE#", "style='display:inline;'");
      sCond.replace("#CARWT_VISIBLE#", "style='display:inline;'");
      String sensorCarW="";
      for(int k=0; k<aSensors.size(); k++)
      {
        if(aSensors[k]->sType==sensorType::temp || aSensors[k]->sType==sensorType::temp_ro)
        { 
          String xCond = FPSTR(condition_option);
          xCond.replace("#OPT_ID#", String(aSensors[k]->Id));
          xCond.replace("#J#", String(j));
          xCond.replace("#OPT_Type#", "Temp");
          xCond.replace("#OPT_NAME#", aSensors[k]->name);
          if(iCondition.substring(iCondition.indexOf("/")+1,iCondition.lastIndexOf("/")).toInt()== aSensors[k]->Id)
          {
            xCond.replace("#_SELECTED_#", "selected");
          }
          else
            xCond.replace("#_SELECTED_#", "");
          sensorCarW+=xCond+"\n";
        }
      }
      sCond.replace("#TEMP_SENSORS#", sensorCarW);
    }    
    sCond.replace("#CARWT_VISIBLE#", "style='display:none;'");
    sCond.replace("#OP_VISIBLE#", "style='display:inline;'");

    String extraCond="";
    for(int i=0; i<aSensors.size(); i++)
    {
        //<option id="#OPT_ID#_#J#_#I#" value="#OPT_ID#" data-type="#OPT_Type#" #_SELECTED_#>#OPT_NAME#</option>
        String xCond = FPSTR(condition_option);
        xCond.replace("#OPT_ID#", String(aSensors[i]->Id));
        xCond.replace("#J#", String(j));
        switch(aSensors[i]->sType)
        {
          case sensorType::temp: xCond.replace("#OPT_Type#", "Temp"); break;
          case sensorType::tension: xCond.replace("#OPT_Type#", "Tension"); break;
          case sensorType::temp_ro: xCond.replace("#OPT_Type#", "Temp"); break;
          default: xCond.replace("#OPT_Type#", "Void");break;
        }
        xCond.replace("#OPT_NAME#", aSensors[i]->name);
        if(iCondition.substring(0,5).toInt()== aSensors[i]->Id)
        {
          xCond.replace("#_SELECTED_#", "selected");
          if(iCondition.charAt(5)=='>') sCond.replace("#SUP_SELECTED#", "selected");
          else if(iCondition.charAt(5)=='<') sCond.replace("#INF_SELECTED#", "selected");

          sCond.replace("#EQ_SELECTED#", "disabled");
          
          sCond.replace("#NUMBER_TYPE#", "number");
          sCond.replace("#T_LABEL_STYLE#", "");
          sCond.replace("#TEMP#", iCondition.substring(6));
        }
        else
          xCond.replace("#_SELECTED_#", "");
        extraCond+=xCond+"\n";
    }
    
    sCond.replace("#EXTRA_CONDITIONS#", extraCond);
    ioHTML+=sCond;
  }
}
//Add a filtering option for type of sensors to consider (only temp or all)
String WebPage::getExtraConditions()
{
  String extraCond="";
  for(int i=0; i<aSensors.size(); i++)
  {
      //<option id="#OPT_ID#_#J#_#I#" value="#OPT_ID#" data-type="#OPT_Type#" #_SELECTED_#>#OPT_NAME#</option>
      String xCond = FPSTR(condition_option);
      xCond.replace("#OPT_ID#", String(aSensors[i]->Id));
      switch(aSensors[i]->sType)
      {
        case sensorType::temp: xCond.replace("#OPT_Type#", "Temp"); break;
        case sensorType::temp_ro: xCond.replace("#OPT_Type#", "Temp"); break;
        default: xCond.replace("#OPT_Type#", "Void");break;
      }
      xCond.replace("#OPT_NAME#", aSensors[i]->name);
      xCond.replace("#_SELECTED_#", "");
      extraCond+=xCond+"\n";
  }
  return extraCond;
}

//Add a filtering option for type of sensors to consider (only temp or all)
String WebPage::getCarWSensors()
{
  String extraCond="";
  for(int i=0; i<aSensors.size(); i++)
  {
      //<option id="#OPT_ID#_#J#_#I#" value="#OPT_ID#" data-type="#OPT_Type#" #_SELECTED_#>#OPT_NAME#</option>
      if(aSensors[i]->sType == sensorType::temp || aSensors[i]->sType == sensorType::temp_ro)
      {
        String xCond = FPSTR(condition_option);
        xCond.replace("#OPT_ID#", String(aSensors[i]->Id));
        xCond.replace("#OPT_Type#", "Temp");
        xCond.replace("#OPT_NAME#", aSensors[i]->name);
        xCond.replace("#_SELECTED_#", "");
        extraCond+=xCond+"\n";
      }
  }
  return extraCond;
}
