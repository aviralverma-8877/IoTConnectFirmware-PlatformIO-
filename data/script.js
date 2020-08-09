var Socket;
function init_socket()
{
    Socket = new WebSocket('ws://'+window.location.hostname+":81/");
    Socket.onmessage = function(event){
        json = JSON.parse(event.data);
        update_table_data(json);
    }
    Socket.onopen = function(event){
        console.log("Connected to web sockets...")
    }
    Socket.onclose = function(event){
        console.log("Connection to websockets closed....")
    }
    Socket.onerror = function(event){
        console.log("Error in websockets");
    }
}

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
    if(action == "update_login")
    {
        theUrl = '/update_login?options='+JSON.stringify(options);
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
            <label class=\"switch\">\
                <input type=\"checkbox\" id=\"checkbox-"+i+"\" onchange=\"toggle_relay(this,"+i+")\">\
                <span class=\"slider round\"></span>\
            </label>\
        </th>\
        </tr>";
    }
    table.innerHTML = content;
    init_socket();
    data = httpGet(0, 0, 'get_status');
    json = JSON.parse(data);
    update_table_data(json);
}
function toggle_relay(element, relay_no)
{
    if(element.checked) 
        httpGet(relay_no,1,"toggle_relay"); 
    else 
        httpGet(relay_no,0,"toggle_relay");
}
function update_table_data(json)
{
    if(json != null)
    {
        for(var i=0; i<json['v'].length; i++)
        {
            var element = document.getElementById('status-'+i);
            if(json['v'][i] == 0)
            {
                document.getElementById('status-'+i).style.color = "red";
                document.getElementById('checkbox-'+i).checked = false;
                element.innerHTML = 'OFF';
            }
            if(json['v'][i] == 1)
            {
                document.getElementById('status-'+i).style.color = "green";
                document.getElementById('checkbox-'+i).checked = true;
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
}
function update_wifi(ssid, pass)
{
    return new Promise (() => {
        response = httpGet(0,0,"set_wifi",{"ssid":ssid,"pass":pass});
        setTimeout(()=>
        {
            location.reload();
        },20000);
    });
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
        return new Promise (() => {
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
        });
    },10);
}

function update_login(login_uname_input, login_pass_input, login_confirm_pass_input)
{
    if(login_pass_input.value == login_confirm_pass_input.value)
    {
        if(login_uname_input.value.length > 6){
            if(login_pass_input.value.length > 6)
            {
                response = httpGet(0,0,"update_login",
                {
                    "uname":login_uname_input.value,
                    "password":login_pass_input.value
                });
                data = JSON.parse(response);
                done = data["done"];
                if(done)
                {
                    alert("Username and password updated.")
                }
            }
            else{
                alert("Password length should be more than 6 charecter.");
            }
        }
        else{
            alert("Username length should be more than 6 charecter.");
        }
    }
    else 
        alert('Password and confirm password not matching.');
}