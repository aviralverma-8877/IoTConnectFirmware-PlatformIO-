#include <Arduino.h>


String chipid = String(ESP.getChipId());  //Fetching ESP device ID.
bool debugging = false;                   //Turn On or Off the serial output.
String debugtopic = chipid+"-debug";      //MQTT Topic for sending debug data.
String outtopic = chipid+"-out";          //MQTT Topic for sending data from ESP.
String intopic = chipid+"-in";            //MQTT Topic for reciving data to ESP.
/*-------------Webpage Data---------------------------------*/
String DefaultResponseHTML = "<!DOCTYPE html>\
                          <head>\
                            <meta name=\"viewport\" content=\"width=device-width,user-scalable=no,initial-scale=1\">\
                            <title>\
                              ESPID : "+chipid+" | IoT Connect : Solutions for smart homes.\
                            </title>\
                            <link href=\"https://fonts.googleapis.com/css2?family=Fjalla+One&display=swap\" rel=\"stylesheet\">\
                          </head>\
                          <style>\
                          body{\
                            background-color: #e7eaf9;\
                          }\
                          .style_btn\
                          {\
                              background-color: #4CAF50;\
                              border: none;\
                              color: white;\
                              padding: 15px 32px;\
                              text-align: center;\
                              text-decoration: none;\
                              display: inline-block;\
                              font-size: 16px;\
                              margin: 4px 2px;\
                              cursor: pointer;\
                          }\
                          </style>\
                          <script>\
                          function httpGet(relay, value, action)\
                          {\
                              if(action == \"toggle_relay\")\
                              {\
                                theUrl = '/control?command={\"relay\":'+relay+',\"action\":'+value+'}';\
                              }\
                              if(action == \"reset_device\")\
                              {\
                                theUrl = '/control?device={\"action\":\"reset\"}';\
                              }\
                              if(action == \"reboot_device\")\
                              {\
                                theUrl = '/control?device={\"action\":\"reboot\"}';\
                              }\
                              if(action == \"get_status\")\
                              {\
                                theUrl = '/get_status';\
                              }\
                              var xmlHttp = new XMLHttpRequest();\
                              xmlHttp.open( \"GET\", theUrl, false );\
                              xmlHttp.send( null );\
                              return xmlHttp.responseText;\
                          }\
                          function print_table(){\
                            var table = document.getElementById(\"relay_table\");\
                            var content = \"\";\
                            for(var i = 0; i<8; i++)\
                            {\
                              content = content + \"<tr>\
                                <th>\
                                  Relay \"+i+\" : <span id='status-\"+i+\"'></span>\
                                </th>\
                                <th>\
                                  <button class='style_btn' onclick='httpGet(\"+i+\",1,\\\"toggle_relay\\\")'>\
                                    ON\
                                  </button>\
                                  <button class='style_btn' onclick='httpGet(\"+i+\",0,\\\"toggle_relay\\\")'>\
                                    OFF\
                                  </button>\
                                </th>\
                              </tr>\
                              \";\
                            }\
                            table.innerHTML = content;\
                            setInterval(function()\
                            {\
                              data = httpGet(0, 0, 'get_status');\
                              json = JSON.parse(data);\
                              if(json != null)\
                              {\
                                for(var i=0; i<json['relay_status'].length; i++)\
                                {\
                                  var element = document.getElementById('status-'+i);\
                                  if(json['relay_status'][i] == 0)\
                                  {\
                                    element.innerHTML = 'OFF';\
                                  }\
                                  if(json['relay_status'][i] == 1)\
                                  {\
                                    element.innerHTML = 'ON';\
                                  }\
                                }\
                                var cont = \"\";\
                                wifi_ssid = json['wifi_ssid'];\
                                wifi_type = json['type'];\
                                cont = \"Connected to <b>\"+wifi_ssid+\"</b>\";\
                                cont += \" | Device Type <b>\"+wifi_type+\"</b>\";\
                                if(json['temp'] != undefined)\
                                {\
                                  temp = json['temp'];\
                                  humid = json['humid'];\
                                  lumin = json['lumin'];\
                                  cont = cont + \" | Temperature : \"+temp+\" C | Humidity : \"+humid+\" % | Lumin : \"+lumin+\" % \"\
                                }\
                                element = document.getElementById('WiFi_Status');\
                                element.innerHTML = cont;\
                              }\
                            },1000);\
                          }\
                          </script>\
                        <body onload=\"print_table()\">\
                            <h1 style=\"font-family: 'Fjalla One'\">IoT Connect : Solutions for smart homes.</h1>\
                            <hr /><center>\
                            <div id='WiFi_Status' align=\"left\" style=\"font-family: 'Fjalla One'\">\
                            </div>\
                            <div align=\"right\">\
                              <button class='style_btn' onclick=\"if(confirm('Are you sure you want to reset this device?')){httpGet(0,0,'reset_device')}\">Reset</button>\
                              <button class='style_btn' onclick=\"httpGet(0,0,'reboot_device')\">Reboot</button>\
                              <button class='style_btn' onclick=\"location.href = ('/update')\">Update Firmware</button>\
                            </div>\
                            <table id=\"relay_table\" cellspacing=\"15\" style=\"border:#ccc solid thin\">\
                            </table>\
                            </center>\
                        </body>\
                      </html>";
/*-------------Webpage Data---------------------------------*/
