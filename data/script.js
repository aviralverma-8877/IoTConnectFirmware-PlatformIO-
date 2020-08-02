function httpGet(relay, value, action, options = {})
{
    if(action == "toggle_relay")
    {
        theUrl = '/control?command={"relay":'+relay+',"action":'+value+'}';
    }
    if(action == "device")
    {
        theUrl = '/control?device='+options;
    }
    if(action == "get_status")
    {
        theUrl = '/get_status';
    }
    if(action == "scan_wifi")
    {
        theUrl = '/scan_wifi';
    }
    if(action == "set_wifi")
    {
        theUrl = '/set_wifi?options='+JSON.stringify(options);
    }
    try
    {
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.open( "GET", theUrl, false );
        xmlHttp.send( null );
        return xmlHttp.responseText;
    }
    catch(err){
        return {"done":0}
    }
}
function print_table()
{
    var table = document.getElementById("relay_table");
    var content = "";
    for(var i = 0; i<8; i++)
    {
        content = content + "<tr>\
        <th style='font:Fjalla One'>\
            Relay "+(i+1)+" : <span id='status-"+i+"'></span>\
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
    setTimeout(function run()
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
            onb_status = json['onb_led'];
            document.getElementById("on_board_led").checked = onb_status
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
        setTimeout(run, 1000);
    },1000);
}
function update_wifi(ssid, pass)
{
    response = httpGet(0,0,"set_wifi",{"ssid":ssid,"pass":pass});
}
function scan_wifi()
{
    element = document.getElementById('ssid_list');
    element.innerHTML = "\
    <tr>\
        <td style='font:Fjalla One'>\
            <b>Scanning...</b>\
        </td>\
    </tr>";
    setTimeout(function(){
        response = httpGet(0,0,"scan_wifi");
        ssid_list = JSON.parse(response)["ssid"];
        content = "";
        for(i=0; i < ssid_list.length; i++)
        {
            content = content + "<tr>\
                                    <td style='font:Fjalla One'>\
                                        <a href='#ssid_input' onclick='ssid_input.value=\""+ssid_list[i]+"\"'><b>"+ssid_list[i]+"</b></a>\
                                    </td>\
                                </tr>"
        }
        element = document.getElementById('ssid_list');
        element.innerHTML = content;
    },10);
}