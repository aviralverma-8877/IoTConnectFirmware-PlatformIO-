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
    if(action == "update_device_config")
    {
        theUrl = '/update_device_config?options='+JSON.stringify(options);
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
    console.log(json);
    if(!json.init_setup)
    {
        start_init_setup()
    }
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

var avail_GPIO = [0,2,4,5,12,13,14,15,16]
var ele_list = [
    "sr_dpin",
    "sr_cpin",
    "sr_lpin",
    "status_led",
    "rst_pin",
    "rly_1",
    "rly_2",
    "rly_3",
    "rly_4",
    "rly_5",
    "sen_pin"
]

function list_gpio()
{
    for(j=0; j<ele_list.length; j++)
    {   ele = document.getElementById(ele_list[j]);
        ele.innerHTML = "<option value='N/A'>N/A</option>";
        for(i=0; i<avail_GPIO.length; i++)
        {
            ele.innerHTML += "<option value='"+avail_GPIO[i]+"'>GPIO "+avail_GPIO[i]+"</option>"
        }
    }
}

function start_init_setup()
{
    document.getElementById("cover").style.display = "block";
    document.getElementById("popup").style.display = "block";
    setTimeout(()=>{
        document.getElementById("cover").style.opacity = 1;
        document.getElementById("popup").style.opacity = 1;
        setTimeout(function(){
            list_gpio();
        },700)
    }, 10);
}
function close_init_setup()
{
    document.getElementById("cover").style.opacity = 0;
    document.getElementById("popup").style.opacity = 0;
    setTimeout(()=>{
        document.getElementById("cover").style.display = "none";
        document.getElementById("popup").style.display = "none";
    }, 700);
}

function check_board(board_type)
{
    if(board_type == "Custom Board")
    {
        document.getElementById("custom_board_configs").style.display = "block";
    }
    else
    {
        document.getElementById("custom_board_configs").style.display = "none";
    }
}

function check_config()
{
    if(document.getElementById("device_type").value == "Custom Board")
    {
        sta = document.getElementById("status_led").value;
        res = document.getElementById("rst_pin").value;
        if(sta != "" && sta != "N/A")
        {
            if(res != "" && res != "N/A")
            {
                if(document.getElementById("sr_avail").checked)
                {
                    if(document.getElementById("sr_dpin").value == "N/A")
                    {
                        alert("No data pin alloted to shift register.")
                        return false
                    }
                    if(document.getElementById("sr_cpin").value == "N/A")
                    {
                        alert("No clock pin alloted to shift register.")
                        return false
                    }
                    if(document.getElementById("sr_lpin").value == "N/A")
                    {
                        alert("No latch pin alloted to shift register.")
                        return false
                    }
                }
                if(document.getElementById("dht_sens_avail").checked)
                {
                    if(document.getElementById("sen_pin").value == "N/A")
                    {
                        alert("No DHT sensor pin alloted.")
                        return false
                    }
                }
                var used_GPIO = []
                for(j=0; j<ele_list.length; j++)
                {   ele = document.getElementById(ele_list[j]);
                    if(ele.value != "N/A" && ele.value != "")
                    {
                        if(used_GPIO.indexOf(ele.value) != -1)
                        {
                            alert("GPIO "+ele.value+" has been assigned more than once.");
                            return false;
                        }
                        used_GPIO.push(ele.value)
                    }
                }
                return true;
            }
            else
            {
                alert("Reset button pin needs to be defined.")
                return false;
            }
        }
        else
        {
            alert("Status LED pin needs to be defined.")
            return false;
        }
    }
    else{
        return true;
    }
}

function save_config()
{
    if(check_config())
    {
        var config = {
            "init_setup_done":true,
            "device_cofig":{
                "shift_out_reg" : {
                    "avail" : true,
                    "serialDataPin" : 16,
                    "clockPin" : 14,
                    "latchPin" : 12
                },
                "status_led" : 13,
                "reset_btn" : 4,
                "relay" : {
                    "count" : 0,
                    "GPIO" : []
                },
                "dht":{
                    "TYPE":"DHT11",
                    "GPIO":2,
                    "INSTALLED":false
                },
                "light":{
                    "GPIO":"A0",
                    "INSTALLED":false
                }
            }
        }
        if(document.getElementById("device_type").value == "IoT Connect Board Rev 1")
        {
            config.device_cofig.shift_out_reg.avail = true;
            config.device_cofig.shift_out_reg.serialDataPin = 16;
            config.device_cofig.shift_out_reg.clockPin = 14;
            config.device_cofig.shift_out_reg.latchPin = 12;
            config.device_cofig.status_led = 13;
            config.device_cofig.reset_btn = 4;
            config.device_cofig.relay.count = 0;
            config.device_cofig.dht.INSTALLED = false;
            config.device_cofig.light.INSTALLED = false;
        }
        if(document.getElementById("device_type").value == "IoT Connect Board Rev 2")
        {
            config.device_cofig.shift_out_reg.avail = true;
            config.device_cofig.shift_out_reg.serialDataPin = 16;
            config.device_cofig.shift_out_reg.clockPin = 14;
            config.device_cofig.shift_out_reg.latchPin = 12;
            config.device_cofig.status_led = 13;
            config.device_cofig.reset_btn = 4;
            config.device_cofig.relay.count = 0;
            config.device_cofig.dht.INSTALLED = true;
            config.device_cofig.dht.TYPE = "dht11"
            config.device_cofig.dht.GPIO = 2
            config.device_cofig.light.INSTALLED = true;
            config.device_cofig.light.GPIO = "A0"
        }
        if(document.getElementById("device_type").value == "Sonoff Basics")
        {
            config.device_cofig.shift_out_reg.avail = false;
            config.device_cofig.status_led = 13;
            config.device_cofig.reset_btn = 0;
            config.device_cofig.relay.count = 1;
            config.device_cofig.relay.GPIO = [12]
            config.device_cofig.dht.INSTALLED = false;
            config.device_cofig.light.INSTALLED = false;
        }
        if(document.getElementById("device_type").value == "Custom Board")
        {
            if(document.getElementById("sr_avail").checked)
            {
                config.device_cofig.shift_out_reg.avail = true;
                config.device_cofig.shift_out_reg.serialDataPin = document.getElementById("sr_dpin").value;
                config.device_cofig.shift_out_reg.clockPin = document.getElementById("sr_cpin").value;
                config.device_cofig.shift_out_reg.latchPin = document.getElementById("sr_lpin").value;
            }
            else
            {
                config.device_cofig.shift_out_reg.avail = false;
            }
            config.device_cofig.status_led = document.getElementById("status_led").value;
            config.device_cofig.reset_btn = document.getElementById("rst_pin").value;
            config.device_cofig.relay.count = 0;
            for(i=1; i<=5; i++)
            {
                if(document.getElementById("rly_"+i).value != "N/A")
                {
                    config.device_cofig.relay.count++;
                    config.device_cofig.relay.GPIO.push(document.getElementById("rly_"+i).value);
                }
            }
            if(document.getElementById("dht_sens_avail").checked)
            {
                config.device_cofig.dht.INSTALLED = true;
                config.device_cofig.dht.GPIO = document.getElementById("sen_pin").value;
            }
            else{
                config.device_cofig.dht.INSTALLED = false;
            }
            if(document.getElementById("ldr_sens_avail").checked)
            {
                config.device_cofig.light.INSTALLED = true;
            }
            else{
                config.device_cofig.light.INSTALLED = false;
                config.device_cofig.light.GPIO = "A0"
            }
        }
        result = JSON.parse(httpGet(0,0,"update_device_config", config))
        if(result.done)
        {
            alert("Device config saved successfully. Rebooting....");
            setTimeout(()=>{
                location.reload();
            },5000)
        }
        else{
            alert(result.error)
        }
    }
}