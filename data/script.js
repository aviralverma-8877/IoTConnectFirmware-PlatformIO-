function httpGet(relay, value, action)
{
    if(action == "toggle_relay")
    {
    theUrl = '/control?command={"relay":'+relay+',"action":'+value+'}';
    }
    if(action == "reset_device")
    {
    theUrl = '/control?device={"action":"reset"}';
    }
    if(action == "reboot_device")
    {
    theUrl = '/control?device={\"action\":\"reboot\"}';
    }
    if(action == "get_status")
    {
    theUrl = '/get_status';
    }
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, false );
    xmlHttp.send( null );
    return xmlHttp.responseText;
}
function print_table()
{
    var table = document.getElementById("relay_table");
    var content = "";
    for(var i = 0; i<8; i++)
    {
        content = content + "<tr>\
        <th>\
            Relay "+i+" : <span id='status-"+i+"'></span>\
        </th>\
        <th>\
            <button class='style_btn' onclick='httpGet("+i+",1,\"toggle_relay\")'>\
            ON\
            </button>\
            <button class='style_btn' onclick='httpGet("+i+",0,\"toggle_relay\")'>\
            OFF\
            </button>\
        </th>\
        </tr>";
    }
    table.innerHTML = content;
    setInterval(function()
    {
        data = httpGet(0, 0, 'get_status');
        json = JSON.parse(data);
        if(json != null)
        {
        for(var i=0; i<json['relay_status'].length; i++)
        {
            var element = document.getElementById('status-'+i);
            if(json['relay_status'][i] == 0)
            {
            element.innerHTML = 'OFF';
            }
            if(json['relay_status'][i] == 1)
            {
            element.innerHTML = 'ON';
            }
        }
        var cont = "";
        wifi_ssid = json['wifi_ssid'];
        wifi_type = json['type'];
        cont = "Connected to <b>"+wifi_ssid+"</b>";
        cont += " | Device Type <b>"+wifi_type+"</b>";
        if(json['temp'] != undefined)
        {
            temp = json['temp'];
            humid = json['humid'];
            lumin = json['lumin'];
            cont = cont + " | Temperature : "+temp+" C | Humidity : "+humid+" % | Lumin : "+lumin+" % "
        }
        element = document.getElementById('WiFi_Status');
        element.innerHTML = cont;
        }
    },1000);
}
