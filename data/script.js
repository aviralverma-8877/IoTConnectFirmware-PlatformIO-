var Socket;
var wifi_status=false;
var chipid = "";
function init_socket()
{
    Socket = new WebSocket('ws://'+window.location.hostname+":81/");
    Socket.onmessage = function(event){
        console.log("Text : "+event.data);
        try
        {
            json = JSON.parse(event.data);
            if(json.action == "alert")
            {
                alert(json.msg)
            }
            else if(json.action == "status")
            {
                if(json.relay != null)
                    update_table_data(json);
            }
        }
        catch(err)
        {
            console.log("Error : "+err);
        }
    }
    Socket.onopen = function(event){
        console.log("Connected to web sockets...")
        httpGet(0, 0, 'get_status');
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
        theUrl = '/control?command={"relay":\"'+relay+'\","action":'+value+'}';
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
        return JSON.stringify({"done":false,"error":err});
    }
}
function set_ui()
{
    data = httpGet(0, 0, 'get_status');
    json = JSON.parse(data);
    if(json.chip_id != undefined)
        chipid = json.chip_id;
    init_socket();
    if(!json.init_setup)
    {
        document.getElementById("cover").onclick = "";
        start_init_setup();
    }
    if(json != null)
    {
        onb_status = json['onb_led'];
        document.getElementById("on_board_led").checked = onb_status
        var cont = "";
        wifi_ssid = json['wifi_ssid'];
        wifi_type = json['type'];
        mqtt_status = json["mqtt_status"];
        wifi_status = json["wifi_status"];
        if(wifi_status)
            cont = "Connected to <b>"+wifi_ssid+"</b>";
        if(mqtt_status)
            cont += "<br />MQTT Status : <b>Connected</b>";
        else
            cont += "<br />MQTT Status : <b>Not Connected</b>";
        element = document.getElementById('WiFi_Status');
        element.innerHTML = cont;
        document.getElementById("firmware_version").innerHTML = json["firmware_version"]
    }    
}
function toggle_relay(element, relay)
{
    if(element.checked) 
        httpGet(relay,1,"toggle_relay"); 
    else 
        httpGet(relay,0,"toggle_relay");
}
table_printed = false;
function print_table(relay_count, on_change_val, name)
{
    var table = document.getElementById("relay_table");
    var content = "";
    content = "<tr>\
    <th style='font:Fjalla One'>\
        "+name+" : <span id='status-"+relay_count+"'></span>\
    </th>\
    <th>\
        <label class=\"switch\">\
            <input type=\"checkbox\" id=\"checkbox-"+relay_count+"\" onchange=\""+on_change_val+"\">\
            <span class=\"slider round\"></span>\
        </label>\
    </th>\
    </tr>";
    table.innerHTML += content;
}
function update_table_data(json)
{
    relays = json.relay;
    if(!table_printed)
    {
        relays.forEach(function(element) 
        {
            on_change_val = "toggle_relay(this, '"+element.name+"')"
            pin = element.comp + "-" + element.pin;
            print_table(pin, on_change_val, element.name);
        });
        table_printed = true;
    }
    relays.forEach(function(ele) 
    {
        pin = ele.comp + "-" + ele.pin;
        var element = document.getElementById('status-'+pin);
        if(!ele.status)
        {
            document.getElementById('status-'+pin).style.color = "red";
            document.getElementById('checkbox-'+pin).checked = false;
            element.innerHTML = 'OFF';
        }
        else
        {
            document.getElementById('status-'+pin).style.color = "green";
            document.getElementById('checkbox-'+pin).checked = true;
            element.innerHTML = 'ON';
        }    
    });
}
function update_wifi(ssid, pass)
{
    return new Promise (() => {
        response = httpGet(0,0,"set_wifi",{"ssid":ssid,"pass":pass});
        setTimeout(()=>
        {
            window.location.replace("http://iot-connect-"+chipid+".local/");
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

var avail_GPIO = [0,1,2,3,4,5,12,13,14,15,16]
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

function check_broker(board_type)
{
    if(board_type == "Custom")
    {
        document.getElementById("mqtt_detail_table").style.display = "block";
    }
    else
    {
        document.getElementById("mqtt_detail_table").style.display = "none";
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
            "mqtt":{
                "service":"",
                "host":"",
                "port":1883,
                "uname":"",
                "pass":"",
                "auth":false
            },
            "device_config":{
                "shift_out_reg" : {
                    "avail" : true,
                    "serialDataPin" : 16,
                    "clockPin" : 14,
                    "latchPin" : 12
                },
                "status_led" : {
                    "led_pin":13,
                    "status":true
                },
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
        if(document.getElementById("mqtt_broker").value == "Custom")
        {
            service = "Custom"
            host = document.getElementById("mqtt_host").value;
            port = document.getElementById("mqtt_port").value;
            uname = document.getElementById("mqtt_uname").value;
            pass = document.getElementById("mqtt_pass").value;
            prefix = document.getElementById("mqtt_prefix").value;
            if(host != "")
            {    
                if(port != "" && port > 0)
                {
                    if(uname == "")
                        auth = false
                    else{
                        auth = true
                        if(pass == "")
                        {
                            alert("Please provide MQTT login password.")
                            return;
                        }
                    }
                    config.mqtt.service = service;
                    config.mqtt.host = host;
                    config.mqtt.port = port;
                    config.mqtt.uname = uname;
                    config.mqtt.pass = pass;
                    config.mqtt.prefix = prefix;
                }
                else
                {
                    alert("Please provide valid MQTT port number.")
                }
            }
            else
            {
                alert("Please provide valid MQTT hostname.")
            }
        }
        else
        {
            service = document.getElementById("mqtt_broker").value;
            config.mqtt.service = service;
        }
        if(document.getElementById("device_type").value == "IoT Connect Board Rev 1")
        {
            config.device_config.shift_out_reg.avail = true;
            config.device_config.shift_out_reg.serialDataPin = 16;
            config.device_config.shift_out_reg.clockPin = 14;
            config.device_config.shift_out_reg.latchPin = 12;
            config.device_config.status_led.led_pin = 13;
            config.device_config.reset_btn = 4;
            config.device_config.relay.count = 0;
            config.device_config.dht.INSTALLED = false;
            config.device_config.light.INSTALLED = false;
        }
        if(document.getElementById("device_type").value == "IoT Connect Board Rev 2")
        {
            config.device_config.shift_out_reg.avail = true;
            config.device_config.shift_out_reg.serialDataPin = 16;
            config.device_config.shift_out_reg.clockPin = 14;
            config.device_config.shift_out_reg.latchPin = 12;
            config.device_config.status_led.led_pin = 13;
            config.device_config.reset_btn = 4;
            config.device_config.relay.count = 0;
            config.device_config.dht.INSTALLED = true;
            config.device_config.dht.TYPE = "dht11"
            config.device_config.dht.GPIO = 2
            config.device_config.light.INSTALLED = true;
            config.device_config.light.GPIO = "A0"
        }
        if(document.getElementById("device_type").value == "Sonoff Basics")
        {
            config.device_config.shift_out_reg.avail = false;
            config.device_config.status_led.led_pin = 13;
            config.device_config.reset_btn = 0;
            config.device_config.relay.count = 1;
            config.device_config.relay.GPIO = [12]
            config.device_config.dht.INSTALLED = false;
            config.device_config.light.INSTALLED = false;
        }
        if(document.getElementById("device_type").value == "Custom Board")
        {
            if(document.getElementById("sr_avail").checked)
            {
                config.device_config.shift_out_reg.avail = true;
                config.device_config.shift_out_reg.serialDataPin = document.getElementById("sr_dpin").value;
                config.device_config.shift_out_reg.clockPin = document.getElementById("sr_cpin").value;
                config.device_config.shift_out_reg.latchPin = document.getElementById("sr_lpin").value;
            }
            else
            {
                config.device_config.shift_out_reg.avail = false;
            }
            config.device_config.status_led.led_pin = document.getElementById("status_led").value;
            config.device_config.reset_btn = document.getElementById("rst_pin").value;
            config.device_config.relay.count = 0;
            for(i=1; i<=5; i++)
            {
                if(document.getElementById("rly_"+i).value != "N/A")
                {
                    config.device_config.relay.count++;
                    config.device_config.relay.GPIO.push(document.getElementById("rly_"+i).value);
                }
            }
            if(document.getElementById("dht_sens_avail").checked)
            {
                config.device_config.dht.INSTALLED = true;
                config.device_config.dht.GPIO = document.getElementById("sen_pin").value;
            }
            else{
                config.device_config.dht.INSTALLED = false;
            }
            if(document.getElementById("ldr_sens_avail").checked)
            {
                config.device_config.light.INSTALLED = true;
            }
            else{
                config.device_config.light.INSTALLED = false;
                config.device_config.light.GPIO = "A0"
            }
        }
        console.log(config);
        data = httpGet(0,0,"update_device_config", config);
        result = JSON.parse(data);
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