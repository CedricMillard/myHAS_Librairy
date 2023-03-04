//Create <range-picker> tag
//Update WebPage template to add range-picker (including #I, #value...)
class rangePicker extends HTMLElement {
static get observedAttributes() {
      return ['lBound', 'rBound'];
}

get lBound() {
    return this.getAttribute('lBound');
}

set lBound(val) {
    this.setAttribute('lBound', val);
}

get rBound() {
    return this.getAttribute('rBound');
}

set rBound(val) {
    this.setAttribute('rBound', val);
}

get disabled() {
    return this.hasAttribute('disabled');
}

set disabled(val) {
    // Reflect the value of the disabled property as an HTML attribute.
    if (val) {
      this.setAttribute('disabled', '');
      this.rangeL.disabled = true;
      this.rangeR.disabled = true;
    } else {
      this.removeAttribute('disabled');
      if(!this.lBound.startsWith("Sun"))
        this.rangeL.disabled = false;
      if(!this.rBound.startsWith("Sun"))
        this.rangeR.disabled = false;
    }
}

changeL()
{
  this.rangeL.value=Math.min(this.rangeL.value,this.rangeR.value-1);
	var value=(100/(parseInt(this.rangeL.max)-parseInt(this.rangeL.min)))*parseInt(this.rangeL.value)-(100/(parseInt(this.rangeL.max)-parseInt(this.rangeL.min)))*parseInt(this.rangeL.min);
	var children = this.shadowRoot.querySelector("[slider]").childNodes[1].childNodes;
	children[1].style.width=value+'%';
	children[5].style.left=value+'%';
	children[7].style.left=value+'%';children[11].style.left=value+'%';
	var h = String(Math.floor((this.rangeL.value*5)/60));
	if(h.length ==1) h = "0"+h;
	var m = String((this.rangeL.value*5)%60);
	if(m.length ==1) m = "0"+m;
	children[11].childNodes[1].innerHTML=h+':'+m;
  this.lBound = this.rangeL.value;
  this.dispatchEvent(new Event("change")); 
}

changeR()
{
  this.rangeR.value=Math.max(this.rangeR.value,this.rangeL.value-(-1));
  var nRight=(100/(parseInt(this.rangeR.max)-parseInt(this.rangeR.min)))*parseInt(this.rangeR.value)-(100/(parseInt(this.rangeR.max)-parseInt(this.rangeR.min)))*parseInt(this.rangeR.min);
  var children = this.shadowRoot.querySelector("[slider]").childNodes[1].childNodes;
  children[3].style.width=(100-nRight)+'%';
  children[5].style.right=(100-nRight)+'%';
  children[9].style.left=nRight+'%';children[13].style.left=nRight+'%';
  var h = String(Math.floor((this.rangeR.value*5)/60));
  if(h.length ==1) h = "0"+h;
  var m = String((this.rangeR.value*5)%60);
  if(m.length ==1) m = "0"+m;
  children[13].childNodes[1].innerHTML=h+':'+m;
  this.rBound = this.rangeR.value;
  this.dispatchEvent(new Event("change"));
}

changeTypeL()
{
  if(this.typeL.value=="Time")
  {
    this.rangeL.disabled = false;
    //Not needed, but usefull to refresh the value
    this.changeL();
    this.rangeL.style.zIndex = this.rangeR.style.zIndex+1;
  }
  else
  {
    this.rangeL.disabled = true;
    if(this.typeL.value=="Sunrise")
    {
      this.rangeL.value = 0;
      this.changeL();
      this.shadowRoot.querySelector("[sign].left").childNodes[1].innerHTML = "Sunrise";
      this.lBound = "Sunrise";
      if(this.typeR.value=="Sunrise")
      {
        this.typeR.value="Time";
        this.changeTypeR();
      }
    }
    else if(this.typeL.value=="Sunset")
    {
      this.rangeL.value = 144;
      if(this.typeR.value!="Time")
      {
        this.typeR.value="Time";
        this.changeTypeR();
      }
      else if(this.rangeR.value<145) 
      {
        this.rangeR.value= 145;
        this.changeR();
        this.rangeL.style.zIndex = this.rangeR.style.zIndex-1;
      }
      this.changeL();
      this.shadowRoot.querySelector("[sign].left").childNodes[1].innerHTML = "Sunset";
      this.lBound = "Sunset";
    }
    this.dispatchEvent(new Event("change"));
  }
}

changeTypeR()
{
  if(this.typeR.value=="Time")
  {
    this.rangeR.disabled = false;
    this.changeR();
    this.rangeL.style.zIndex = this.rangeR.style.zIndex-1;
  }
  else
  {
    this.rangeR.disabled = true;
    if(this.typeR.value=="Sunset")
    {
      this.rangeR.value = 288;
      this.changeR();
      this.shadowRoot.querySelector("[sign].right").childNodes[1].innerHTML = "Sunset";
      this.rBound = "Sunset";
      if(this.typeL.value=="Sunset")
      {
        this.typeL.value="Time";
        this.changeTypeL();
      }
    }
    else if(this.typeR.value=="Sunrise")
    {
      this.rangeR.value = 144;
      if(this.typeL.value!="Time")
      {
        this.typeL.value="Time";
        this.changeTypeL();
      }
    else if(this.rangeL.value>143) 
      {
        this.rangeL.value= 143;
        this.changeL();
        this.rangeL.style.zIndex = this.rangeR.style.zIndex+1;
      }
      this.changeR();
      this.shadowRoot.querySelector("[sign].right").childNodes[1].innerHTML = "Sunrise";
      this.rBound = "Sunrise";
    }
    this.dispatchEvent(new Event("change"));
  }
}

connectedCallback()
{
	this.rangeL.addEventListener('input', this.changeL);
  this.rangeR.addEventListener('input', this.changeR);
  
  this.typeL.addEventListener('change', this.changeTypeL);
  this.typeR.addEventListener('change', this.changeTypeR);

  this.rangeR.value = 288;
  this.rangeL.value = 0;

	if(!this.hasAttribute('lBound')) 
    this.setAttribute('lBound',0);
     
  if(this.lBound=="Sunset" || this.lBound=="Sunrise")
  {
    this.typeL.value = this.lBound;
    this.changeTypeL();
  }
  else
  {
    this.rangeL.value=this.lBound;
    this.changeL();
  }

	if(!this.hasAttribute('rBound'))
	  this.setAttribute('rBound',288);
    if(this.rBound=="Sunset" || this.rBound=="Sunrise")
    {
      this.typeR.value = this.rBound;
      this.changeTypeR();
    }
    else
    {
      this.rangeR.value=this.rBound;
      this.changeR();
    }
}

attributeChangedCallback(name, oldValue, newValue) {
  if(name=='lBound')
	{
	  this.rangeL.value=this.newValue;
	  this.changeL();
	}
	if(name=='rBound')
	{
    this.rangeR.value=this.newValue;
	  this.changeR();
	}
}

disconnectedCallback() {
  this.rangeL.removeEventListener('input', this.changeL);
  this.rangeR.removeEventListener('input', this.changeR);
}

constructor() {
	super(); // always call super() first in the constructor.

  this.changeL = this.changeL.bind(this);
  this.changeR = this.changeR.bind(this);
  
  this.changeTypeL = this.changeTypeL.bind(this);
  this.changeTypeR = this.changeTypeR.bind(this);

  // Attach a shadow root
  const shadowRoot = this.attachShadow({mode: 'open'});

	shadowRoot.innerHTML = `
	<link rel="stylesheet" type="text/css" href="http://cedric.millard.free.fr/MyHAS.css">
	
	<select class="icon" style="display:inline-block;" id="typeL">
	  <option value="Time" class="icon">&#xe81c;</option>
	  <option value="Sunrise" class="icon">&#xe802;&#xe804;</option>
	  <option value="Sunset" class="icon">&#xe803;&#xe805;</option>
	</select>

	<div slider id="slider-distance">
	  <div>
	    <div inverse-left style="width:10%;"></div>
	    <div inverse-right style="width:51%;"></div>
	    <div range style="left:10%;right:51%;"></div>
	    <span thumb style="left:10%;"></span>
	    <span thumb style="left:49%;"></span>
	    <div sign class="left" style="left:10%;">
	      <span id="value">2:20</span>
	    </div>
	    <div sign class="right" style="left:49%;">
	      <span id="value">11:40</span>
	    </div>
	  </div>
	  <input type="range" tabindex="0" value="0" max="288" min="0" step="1" style="z-index:3;"/>

	  <input type="range" tabindex="0" value="288" max="288" min="0" step="1" style="z-index:3;"/>
	</div>
	<select class="icon" style="display:inline-block;" id="typeR">
	  <option value="Time" class="icon">&#xe81c;</option>
	  <option value="Sunrise" class="icon">&#xe802;&#xe804;</option>
	  <option value="Sunset" class="icon">&#xe803;&#xe805;</option>
	</select>
    `;

	this.rangeL =  this.shadowRoot.querySelector("[slider]").childNodes[3];
  this.rangeR =  this.shadowRoot.querySelector("[slider]").childNodes[5];
  
  this.typeL = this.shadowRoot.getElementById("typeL");
  this.typeR = this.shadowRoot.getElementById("typeR");
}
}

customElements.define('range-picker', rangePicker);


//Update to handle several rules => generate 1 string
function sendForm() {
  updateFreq();
  document.getElementById("NbRule").value = nbRule;
  var ruleLen = 12 + nbVisibleRule * 54;

  for(i=1; i<=nbRule; i++)
  {
    document.getElementById("rule"+i).disabled = (i?false:true);
    if(document.getElementById("status"+i).value != "deleted")
    {
      ruleLen += document.getElementById("Freq"+i).value.length + document.getElementById("rule"+i).value.length;
    }
  }

  if(ruleLen<=450)
  {
    for(i=0; i<=nbRule; i++)
    {
      document.getElementById("rule"+i).disabled = (i?false:true);

      for(j=1; j<=document.getElementById("NbCondition"+i).value; j++)
      {
        document.getElementById("variable_"+j+"_"+i).disabled = true;
        document.getElementById("operator_"+j+"_"+i).disabled = true;
        //if(document.getElementById("time_"+j+"_"+i).value != "")
          document.getElementById("time_"+j+"_"+i).disabled = true;
        //if(document.getElementById("temp_"+j+"_"+i).value != "")
          document.getElementById("temp_"+j+"_"+i).disabled = true;
          document.getElementById("timeRange_"+j+"_"+i).disabled = true;
      }

      document.getElementById("Active"+i).disabled = true;
      document.getElementById("turnON"+i).disabled = (i?false:true);
      //document.getElementById("turnOFF"+i).disabled = (i?false:true);
      if(document.getElementById("blink"+i).type == "radio") document.getElementById("blink"+i).disabled = (i?false:true);
    }
    document.getElementById("FormR").submit();
  }

  else alert("Too many rules to be saved in EEPROM");
}

function activateRule(index = -1)
{
  var isActive = false;
  if(index==-1 && typeof event != "undefined")
  {
    var root = event.target.id;
    index = root.slice(-1);
  }
  if(document.getElementById("Active"+index).checked) isActive = true;

  for (const element of document.getElementById("myFormDiv"+index).getElementsByTagName("input"))
  {
    if(typeof element.type != "undefined")
    {
      if(element.type != "button" && element.type != "hidden" && element.type != "text" && element.id!="Active"+index)
        element.disabled= !isActive;

      if(element.id!="undefined")
        if(element.id.indexOf("And")==0 || (element.id.indexOf("delCond")==0 && element.id.indexOf("delCond_1")==-1) )
          element.disabled= !isActive;
    }
  }

  for (const element of document.getElementById("myFormDiv"+index).getElementsByTagName("select"))
  {
    element.disabled= !isActive;
  }

  for (const element of document.getElementById("myFormDiv"+index).getElementsByTagName("range-picker"))
  {
    element.disabled= !isActive;
  }

  document.getElementById("status"+index).value= (isActive?"active":"disabled");

}

function updateRule(iIndex=1, iCondNb = 1, checkEvent=true)
{
  var index = iIndex;
  var condNb = iCondNb;
  if(typeof event != "undefined" && checkEvent)
  {
    var root = event.target.id;
    index = root.slice(-1);
    condNb = root.substring(root.indexOf('_')+1, root.lastIndexOf('_'));
  }

  //Step1: manage display of fields

  var variable;
  variable = document.getElementById("variable_"+condNb+"_"+index).value;
  var optField= variable+"_"+condNb+"_"+index;
  var variableType;
  if(document.getElementById(optField).dataset.type!="undefined")
    variableType = document.getElementById(optField).dataset.type;

  if(variableType=="timeRange")
  {
    document.getElementById("operator_"+condNb+"_"+index).style.display = "none";
    document.getElementById("inf_"+condNb+"_"+index).disabled = true;
    document.getElementById("sup_"+condNb+"_"+index).disabled = true;
    document.getElementById("time_"+condNb+"_"+index).type = "hidden";
    document.getElementById("temp_"+condNb+"_"+index).type = "hidden";
    document.getElementById("tempLabel_"+condNb+"_"+index).style.display = "none";
    document.getElementById("timeRange_"+condNb+"_"+index).style.display = "inline";
    document.getElementById("CarWTemp_"+condNb+"_"+index).style.display = "none";
  }
  else if(variableType=="Time")
  {
    document.getElementById("timeRange_"+condNb+"_"+index).style.display = "none";
    document.getElementById("operator_"+condNb+"_"+index).style.display = "inline";
    document.getElementById("eq_"+condNb+"_"+index).disabled = false;
    document.getElementById("inf_"+condNb+"_"+index).disabled = true;
    document.getElementById("sup_"+condNb+"_"+index).disabled = true;
    document.getElementById("time_"+condNb+"_"+index).type = "time";
    document.getElementById("temp_"+condNb+"_"+index).type = "hidden";
    document.getElementById("tempLabel_"+condNb+"_"+index).style.display = "none";
    if(document.getElementById("operator_"+condNb+"_"+index).value != "eq") document.getElementById("operator_"+condNb+"_"+index).value = "eq";
    if(variable=="CarW") 
      document.getElementById("CarWTemp_"+condNb+"_"+index).style.display = "inline";
    else 
      document.getElementById("CarWTemp_"+condNb+"_"+index).style.display = "none";
  }
  else if(variableType=="Void")
  {
    document.getElementById("timeRange_"+condNb+"_"+index).style.display = "none";
    document.getElementById("operator_"+condNb+"_"+index).style.display = "none";
    document.getElementById("inf_"+condNb+"_"+index).disabled = true;
    document.getElementById("sup_"+condNb+"_"+index).disabled = true;
    document.getElementById("time_"+condNb+"_"+index).type = "hidden";
    document.getElementById("temp_"+condNb+"_"+index).type = "hidden";
    document.getElementById("tempLabel_"+condNb+"_"+index).style.display = "none";
    document.getElementById("CarWTemp_"+condNb+"_"+index).style.display = "none";
  }
  else if(variableType=="Temp")
  {
    document.getElementById("timeRange_"+condNb+"_"+index).style.display = "none";
    document.getElementById("operator_"+condNb+"_"+index).style.display = "inline";
    document.getElementById("eq_"+condNb+"_"+index).disabled = true;
    document.getElementById("inf_"+condNb+"_"+index).disabled = false;
    document.getElementById("sup_"+condNb+"_"+index).disabled = false;
    document.getElementById("time_"+condNb+"_"+index).type = "hidden";
    document.getElementById("temp_"+condNb+"_"+index).type = "number";
    document.getElementById("tempLabel_"+condNb+"_"+index).innerHTML = "&deg;C";
    document.getElementById("tempLabel_"+condNb+"_"+index).style.display = "inline";
    document.getElementById("CarWTemp_"+condNb+"_"+index).style.display = "none";
    if(document.getElementById("operator_"+condNb+"_"+index).value == "eq") document.getElementById("operator_"+condNb+"_"+index).value = "inf";
  }
  else if(variableType=="Tension")
  {
    document.getElementById("timeRange_"+condNb+"_"+index).style.display = "none";
    document.getElementById("operator_"+condNb+"_"+index).style.display = "inline";
    document.getElementById("eq_"+condNb+"_"+index).disabled = true;
    document.getElementById("inf_"+condNb+"_"+index).disabled = false;
    document.getElementById("sup_"+condNb+"_"+index).disabled = false;
    document.getElementById("time_"+condNb+"_"+index).type = "hidden";
    document.getElementById("temp_"+condNb+"_"+index).type = "number";
    document.getElementById("tempLabel_"+condNb+"_"+index).innerHTML = "V";
    document.getElementById("tempLabel_"+condNb+"_"+index).style.display = "inline";
    document.getElementById("CarWTemp_"+condNb+"_"+index).style.display = "none";
    if(document.getElementById("operator_"+condNb+"_"+index).value == "eq") document.getElementById("operator_"+condNb+"_"+index).value = "inf";
  }

  //Step2: Compute Rule
  computeRule(index);
}

function computeRule(index)
{
  var nbCond = document.getElementById("NbCondition"+index).value;
  var sRule;

  for(i=1; i<=nbCond; i++)
  {
    if(i==1) sRule = computeCondition(index, i);
    else if(document.getElementById("Condition_"+i+"_"+index).style.display!="none") sRule = sRule+";"+computeCondition(index, i);
  }

  //alert("rule"+index+" "+sRule);
  document.getElementById("rule"+index).value = sRule;
}

function computeCondition(index, condNb)
{
  var sRule;
  var valField="";
  sRule = document.getElementById("variable_"+condNb+"_"+index).value;
  
  var optField = sRule+"_"+condNb+"_"+index;
  var valField;
  if(document.getElementById(optField).dataset.type!="undefined")
  {
    if(document.getElementById(optField).dataset.type=="Time") valField = "time_"+condNb+"_"+index;
    else if(document.getElementById(optField).dataset.type=="Temp") valField = "temp_"+condNb+"_"+index;
    else if(document.getElementById(optField).dataset.type=="timeRange") valField = "timeRange_"+condNb+"_"+index;
  }

  if(valField!="")
  {
    if(valField.startsWith("timeRange"))
    {
      var rangePicker = document.getElementById(valField);
      sRule=rangePicker.lBound+"<"+sRule+"<"+rangePicker.rBound;
    }
    else if(sRule=="CarW")
    {
      sRule+= "/";
      sRule+= document.getElementById("CarWTemp_"+condNb+"_"+index).value;
      sRule+= "/";
      var targetVal = document.getElementById(valField).value;
      if(valField.indexOf("time")>=0)
      {
        var sTime = document.getElementById(valField).value;
        var dot = sTime.search(":");
        var hh = parseInt(sTime.substring(0, dot));
        var min = parseInt(sTime.substring(dot+1));
        targetVal = hh*3600+min*60;
      }
      sRule+=targetVal;
    }
    else
    {
      switch(document.getElementById("operator_"+condNb+"_"+index).value)
      {
        case "eq": sRule += "="; break;
        case "inf": sRule += "<"; break;
        case "sup": sRule += ">"; break;
      }

      var targetVal = document.getElementById(valField).value;
      if(valField.indexOf("time")>=0)
      {
        var sTime = document.getElementById(valField).value;
        var dot = sTime.search(":");
        var hh = parseInt(sTime.substring(0, dot));
        var min = parseInt(sTime.substring(dot+1));
        targetVal = hh*3600+min*60;
      }
      sRule+=targetVal;
    }
  }

  return sRule;
}

function deleteRule()
{
  var index = 0;
  if(typeof event != "undefined")
  {
    var root = event.target.id;
    index = root.slice(-1);
  }
  if(index>0)
  {
    document.getElementById("status"+index).value = "deleted";
    document.getElementById("myFormDiv"+index).style.display = "none";
    nbVisibleRule--;
  }
}

function addRule()
{
  if(nbVisibleRule<9)
  {
    nbRule++;
    nbVisibleRule++;
    var cln = document.getElementById('myFormDiv0').cloneNode(true);
    //change all ids
    for (const element of cln.getElementsByTagName("*"))
    {
      if(typeof element.id != "undefined")
        element.id = element.id.substring(0,element.id.length-1) + nbRule;
      if(typeof element.name != "undefined")
        element.name = element.name.substring(0,element.name.length-1) + nbRule;
      if(typeof element.for != "undefined")
        element.for = element.for.substring(0,element.for.length-1) + nbRule;
    }
    cln.id = "myFormDiv"+nbRule;
    cln.style.display = "block";
    document.getElementById('FormR').insertBefore(cln, document.getElementById('sub'));
  }
}

function addCondition()
{
  var index = 1;
  if(typeof event != "undefined")
  {
    var root = event.target.id;
    index = root.slice(-1);
  }

  nbCondition = document.getElementById("NbCondition"+index).value;
  if(nbCondition<9)
  {
    nbCondition++;
    document.getElementById("NbCondition"+index).value = nbCondition;

    var cln = document.getElementById('Condition_1_'+index).cloneNode(true);
    //change all ids
    for (const element of cln.getElementsByTagName("*"))
    {
      if(typeof element.id != "undefined" && element.id != "")
      {
        element.id = element.id.substring(0,element.id.indexOf("_")+1) + nbCondition + "_" + index;
        if(element.id.startsWith("delCond")) element.disabled=false;
      }
      if(typeof element.name != "undefined")
        element.name = element.name.substring(0,element.name.indexOf("_")+1) + nbCondition + "_" + index;
      if(typeof element.for != "undefined")
        element.for = element.for.substring(0,element.for.indexOf("_")+1) + nbCondition + "_" + index;
    }

    cln.id = "Condition_"+nbCondition+"_"+index;
    cln.style.display = "block";
    document.getElementById("myFormDiv"+index).insertBefore(cln, document.getElementById("NbCondition"+index));
  }

  updateRule(index, nbCondition, false);
}

function delCondition()
{
  var index;
  var condNb;
  if(typeof event != "undefined")
  {
    var root = event.target.id;
    index = root.slice(-1);
    condNb = root.substring(root.indexOf('_')+1, root.lastIndexOf('_'));
  }
  document.getElementById("Condition_"+condNb+"_"+index).style.display = "none";
  computeRule(index);
}

function updateFreq()
{
  for (i=1; i<=nbRule; i++)
  {
    var listCheckboxes = document.getElementById("myFormDiv"+i).getElementsByTagName("input");
    var freqMask = 0;
    var j;
    for (j = 0; j < listCheckboxes.length; j++) {
      if(listCheckboxes[j].type == "checkbox" && listCheckboxes[j].id.substring(0,listCheckboxes[j].id.length-1)!="Active")
      {
        if(listCheckboxes[j].checked) {
          freqMask += parseInt(listCheckboxes[j].value);
          listCheckboxes[j].disabled = true;
        }
      }
    }
    if(freqMask==0) freqMask = 128;
    document.getElementById("Freq"+i).value = freqMask;
  }
}
