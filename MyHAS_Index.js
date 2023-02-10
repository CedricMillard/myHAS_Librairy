function SendForm() {
  var x = document.getElementById("Form1");
  var txt = "";
  var i;
  for (i = 0; i < x.length; i++)
    if(x.elements[i].type == "checkbox")
      if(x.elements[i].checked)
        document.getElementById(x.elements[i].id.concat("_h")).disabled = true;

  x.submit();
}

function noenter() {
  return !(window.event && window.event.keyCode == 13);
}

function httpGet(theUrl)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, false ); // false for synchronous request
    xmlHttp.send( null );
    return xmlHttp.responseText;
}

function checkRefresh()
{
  if(httpGet("/refresh")==1)
    location.reload();
}
setInterval(checkRefresh, 5000);
