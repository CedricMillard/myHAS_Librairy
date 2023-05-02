/*
* Class to manage web page for weather station and socket
*/

#ifndef WebPage_h
#define WebPage_h

#include "ConnectedObjects.h"
#include "Environment.h"
#include "Logging.h"

#define INDEX_FILE_PATH "/index.html"
#define RULES_FILE_PATH "/rules.html"

class WebPage
{
public:
  WebPage(){}
  String getIndexHTML();
  String getIndexHTML_file();
  String getRulesHTML(int iID);
  String getRulesHTML_file(int iID);
  void setEnv(Environment *iEnv) {pEnv = iEnv;}
  void setTitle(String iTitle){sTitle = iTitle;}
  void setLog(Logging *iLog){pLog = iLog;}
  
  Array<Prise*> aPrises;
  Array<Sensor*> aSensors;
  Array<WeatherDisplayObj*> aDisplays;

protected:
  Logging *pLog = NULL;

private:
  void CreateCondition(String &ioHTML, String iCondition, int& j);
  String getExtraConditions();
  String getCarWSensors();
  Environment *pEnv = NULL;
  String sTitle = "My HAS";
};

const char checkbox_style[] PROGMEM = R"checkbox(<!DOCTYPE HTML><html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1" />
<link rel="stylesheet" type="text/css" href="https://cedricmillard.github.io/MyHAS.css">
</head>
)checkbox";

const char index_html[] PROGMEM = R"header(
<body>
<script src="https://cedricmillard.github.io/MyHAS_Index.js"></script>
<h1 align=center>#TITLE#</h1>
<div>System Time = #DATE# #TIME#</div><br><br>
<form id="Form1" action="/" method="post">
)header";


const char index_html_switch[] PROGMEM = R"switch(<label class="switch">
  <input type="checkbox" id="prise_#ID#_status" name="prise_#ID#_status" onchange="SendForm()" #STATUS# value="1">
  <span class="slider round"></span>
</label>
  <input id="prise_#ID#_status_h" type="hidden" value="0" name="prise_#ID#_status">
<em id="#ID#_label" class="big">#NAME#</em>
<input id="#ID#_name" type="hidden" name="prise_#ID#_name" value="#NAME#" onKeyPress="return noenter()"><input type="button" value="Edit name" onclick="if(this.value=='Edit name'){document.getElementById('#ID#_label').hidden=true; document.getElementById('#ID#_name').disabled = false; document.getElementById('#ID#_name').type='Text';this.value='save'}else{SendForm();}"></input>
<input type="button" value="#NB_RULES#" onclick="window.location.href = '/rules?ID=#ID#'"></input> <em id="#ID#_label2" class="small">(#ID#)</em>
<br><br>)switch";

const char index_html_sensor[] PROGMEM = R"sensor(
<em id="sensor_#ID#_label" class="big">#NAME#</em>
<input id="sensor_#ID#_name" type="hidden" name="sensor_#ID#_name" value="#NAME#" onKeyPress="return noenter()"><input type="button" value="Edit name" onclick="if(this.value=='Edit name'){document.getElementById('sensor_#ID#_label').hidden=true; document.getElementById('sensor_#ID#_name').disabled = false; document.getElementById('sensor_#ID#_name').type='Text';this.value='save'}else{SendForm();}" #BTN_VISIBLE#> #SENSOR_VALUE# <em id="sensor_#ID#_label2" class="small">(#ID#)</em>
<br><br>)sensor";

const char index_html_weatherDisp[] PROGMEM = R"weather(
<em id="display_#ID#_label" class="big">#NAME#</em>
<input id="display_#ID#_name" type="hidden" name="display_#ID#_name" value="#NAME#" onKeyPress="return noenter()"><input type="button" value="Edit name" onclick="if(this.value=='Edit name'){document.getElementById('display_#ID#_label').hidden=true; document.getElementById('display_#ID#_name').disabled = false; document.getElementById('display_#ID#_name').type='Text';this.value='save'}else{SendForm();}"> <em id="display_#ID#_label2" class="small">(#ID#)</em>
<br>
<label for="weatherLayout">Layout:
<select id="weatherLayout" name="display_#ID#_Layout" onchange="SendForm()">
<option value="1" #W_SELECTED_LAYOUT1#>Clock and Daily Forecast</option>
<option value="2" #W_SELECTED_LAYOUT2#>Clock and Daily Forecast Mirror</option>
<option value="3" #W_SELECTED_LAYOUT3#>Daily & Hourly Forecast</option>
</select></label>
<br>
<label for="weatherL">Left:
<select id="weatherL" name="display_#ID#_Left" onchange="SendForm()">
<option value="Weather3" #W_SELECTED_L#>Weather D+3</option>
#SENSOR_LIST_L#
</select></label>

<label for="weatherR"> Right:
<select id="weatherR" name="display_#ID#_Right" onchange="SendForm()">
<option value="Weather3" #W_SELECTED_R#>Weather D+3</option>
#SENSOR_LIST_R#
</select>
</label>
<br><br>)weather";

const char weather_option[] PROGMEM = R"opt(<option value="#SENSOR_ID#" #_SELECTED_#>#SENSOR_NAME#</option>)opt";

const char index_html_footer[] PROGMEM = R"footer(
</form>
</body>
</html>)footer";

const char rules_html_header[] PROGMEM = R"rules(
<body>
<script>
var nbRule = #NB_RULES#;
var nbVisibleRule = #NB_RULES#;
</script>
<script src="https://cedricmillard.github.io/MyHAS_Rules.js"></script>
<h1>Rules for #PRISE_NAME#</h1>
<form id="FormR" action="/saveRules" method="post">
<input type="hidden" id="ID" name="ID" value=#ID#>
<input type="hidden" id="NbRule" name="NbRule" value=#NB_RULES#>
<input type="hidden" id="output" name="output" value="">
)rules";

const char rules_html_rule[] PROGMEM = R"rules(
<div id="myFormDiv#I#" #FORM_VISIBLE#>
<label class="switch">
  <input type="checkbox" id="Active#I#" onchange="activateRule()" #STATUS_CHECKED#>
  <span class="slider round"></span>
</label><em class="big">Active</em><br>
<label for="Mon#I#"><input type="checkbox" id="Mon#I#" name="Mon#I#" value=2 #MON_CHECKED#>Mon</label>
<label for="Tue#I#"><input type="checkbox" id="Tue#I#" name="Tue#I#" value=4 #TUE_CHECKED#>Tue</label>
<label for="Wed#I#"><input type="checkbox" id="Wed#I#" name="Wed#I#" value=8 #WED_CHECKED#>Wed</label>
<label for="Thu#I#"><input type="checkbox" id="Thu#I#" name="Thu#I#" value=16 #THU_CHECKED#>Thu</label>
<label for="Fri#I#"><input type="checkbox" id="Fri#I#" name="Fri#I#" value=32 #FRI_CHECKED#>Fri</label>
<label for="Sat#I#"><input type="checkbox" id="Sat#I#" name="Sat#I#" value=64 #SAT_CHECKED#>Sat</label>
<label for="Sun#I#"><input type="checkbox" id="Sun#I#" name="Sun#I#" value=1 #SUN_CHECKED#>Sun</label>
<input type="hidden" id="Freq#I#" name="Freq#I#" value=#FREQ#>
<br>
#CONDITION#
<input type="hidden" id="NbCondition#I#" value="#NB_COND#" disabled>
<input type="button" id="And#I#" value="AND" onclick="addCondition()">
<br>
<label for="rule#I#">Rule: </label><input type="text" id="rule#I#" name="rule#I#" value="#RULE#" disabled>
<br><br>
<label>Action :</label><br>
<label for="turnON#I#"><input type="radio" id="turnON#I#" name="action#I#" value="turnON" #ON_CHECKED#>turn ON</label><br>
<label for="blink#I#" #BLINK_ENABLED#><input type="radio" id="blink#I#" name="action#I#" value="blink" #BLINK_CHECKED#>blink</label><br>
<br>
<input type="hidden" id="status#I#" name="status#I#" value="#STATUS#">
<input type="button" id="delete#I#" value="Delete Rule" onclick="deleteRule()">
<br><hr><br>
</div>)rules";

const char rules_html_footer[] PROGMEM = R"rules(
<input id="sub" type="button" value="Submit" onclick="sendForm()">
<input type="button" value="Add Rule" onclick="addRule()">
<script>
for(i=1;i<=nbRule; i++)
{
  activateRule(i);
}
</script>
</form>
</body>
</html>)rules";


const char condition_html_rule[] PROGMEM = R"cond(
<div id="Condition_#J#_#I#">
<br>
<select id="variable_#J#_#I#" name="variable#I#" onchange="updateRule()">
  <option value="H" id="H_#J#_#I#" data-type="timeRange" #H_SELECTED#>H</option>
  <option value="CarW" id="CarW_#J#_#I#" data-type="Time" #CARW_SELECTED#>CarWarmer</option>
  #EXTRA_CONDITIONS#
</select>
<select id="CarWTemp_#J#_#I#" name="CarWTemp#I#" onchange="updateRule()" #CARWT_VISIBLE#>
  #TEMP_SENSORS#
</select>
<select id="operator_#J#_#I#" name="operator#I#" onchange="updateRule()" #OP_VISIBLE#>
  <option id="eq_#J#_#I#" value="eq" #EQ_SELECTED#>=</option>
  <option id="inf_#J#_#I#" value="inf" #INF_SELECTED#>&lt;</option>
  <option id="sup_#J#_#I#" value="sup" #SUP_SELECTED#>&gt;</option>
</select>
<range-picker id="timeRange_#J#_#I#" lBound="#START_TIME#" rBound="#END_TIME#" onchange="updateRule()" #RANGE_VISIBLE#></range-picker>
<input type="#TIME_TYPE#" id="time_#J#_#I#" name="time#I#" onchange="updateRule()" value="#TIME#" required>
<input type="#NUMBER_TYPE#" id="temp_#J#_#I#" name="temp#I#" min="-20" max="40" onchange="updateRule()" value="#TEMP#" required><label id="tempLabel_#J#_#I#" for="temp_#J#_#I#" #T_LABEL_STYLE#>&deg;C</label>
<input type="button" id="delCond_#J#_#I#" value="Delete" onclick="delCondition()" disabled>
</div>
)cond";

const char condition_option[] PROGMEM = R"opt(<option id="#OPT_ID#_#J#_#I#" value="#OPT_ID#" data-type="#OPT_Type#" #_SELECTED_#>#OPT_NAME#</option>)opt";

const char temp_icon[] = "<img src='https://cedricmillard.github.io/myHAS_tempSensor.png' align='middle'>";
const char bat_icon[] = "<img src='https://cedricmillard.github.io/myHAS_battery.png' align='middle'>";
const char weatherSta_icon[] = "<img src='https://cedricmillard.github.io/myHAS_weatherStation.png' align='middle'>";

#endif
