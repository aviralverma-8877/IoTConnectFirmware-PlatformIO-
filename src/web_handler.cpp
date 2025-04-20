#include "web_handler.h"

extern "C" uint32_t _FS_start;
extern "C" uint32_t _FS_end;

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request){
      //request->addInterestingHeader("ANY");
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      File index = SPIFFS.open("/index.html", "r");
      if (index) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->printf("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"3;url=http://%s/index\" /><title>Redirecting...</title></head><body>", WiFi.softAPIP().toString().c_str());
        response->printf("<p>Redirecting to <a href='http://%s/index'>this link</a><br />Please Wait.....</p>", WiFi.softAPIP().toString().c_str());
        response->print("</body></html>");
        request->send(response);
      }
      else{
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->printf("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"3;url=http://%s/update\" /><title>Redirecting...</title></head><body>", WiFi.softAPIP().toString().c_str());
        response->printf("<p>Redirecting to <a href='http://%s/update'>this link</a><br />Please Wait.....</p>", WiFi.softAPIP().toString().c_str());
        response->print("</body></html>");
        request->send(response);
      }
      index.close();
    }
};


/*-------Web Server Controller------------------------------*/
void handleWebControl(AsyncWebServerRequest *request)
{
  String message;
  JsonDocument doc;
  int params = request->params();
  if(request->hasParam("command"))
  {
    String command = request->arg("command");
    deserializeJson(doc, command);

    String relay = doc["relay"];
    bool action = doc["action"];
    relay_action(relay, action, "");

    doc["done"] = 1;
    serializeJson(doc, message);
  }
  if(request->hasParam("device"))
  {
    String command = request->arg("device");
    deserializeJson(doc, command);

    String action = doc["action"];
    doc["done"] = 1;
    serializeJson(doc, message);
    if(comp(action.c_str(),"reset"))
      reset();
    if(comp(action.c_str(),"reboot"))
      ESP.reset();
    if(comp(action.c_str(),"toggle_onb"))
    {
      bool status = doc["status"];
      conf.led_enabled = status;
      write_config(conf);
      if(conf.led_enabled)
      {
        digitalWrite(indicator_led, def_led_value);
      }
      else
      {
        digitalWrite(indicator_led, !def_led_value);
      }
    }
    if(comp(action.c_str(),"save_status"))
    {
      bool status = doc["status"];
      conf.save_eeprom = status;
      write_config(conf);
    }
    if(comp(action.c_str(),"change_relay_btn_action"))
    {
      String relay = doc["status"];
      conf.btn_relay_act = relay;
      write_config(conf);
    }
  }
  request->send(200, "application/json", message);
}

void handleWebStatus(AsyncWebServerRequest *request)
{
  String return_msg = device_status();
  request->send(200, "application/json", return_msg);
}
void handlefauxmo(AsyncWebServerRequest *request)
{
  int params = request->params();
  if(request->hasParam("options"))
  {
    String fauxmo_relay = request->arg("options");

    JsonDocument return_doc;
    String return_msg;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);

    read_config();
    conf.fauxmo_relay = fauxmo_relay;
    write_config(conf);
    TickerForTimeOut.once(1, [](){
      ESP.reset();
    });
  }
}
void handleDeviceConfig(AsyncWebServerRequest *request)
{
  String return_msg;
  int params = request->params();
  if(request->hasParam("options"))
  {
    String device_config = request->arg("options");
    JsonDocument doc;
    deserializeJson(doc, device_config);

    doc.shrinkToFit();
    JsonDocument return_doc;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);
    String service = doc["mqtt"]["service"];
    if(comp(service.c_str(),"IoT Connect"))
    {
      #include "iotconnect_mqtt_cred.h"
      doc["mqtt"]["host"] = MQTT_HOST;
      doc["mqtt"]["port"] = MQTT_PORT;
      doc["mqtt"]["uname"] = MQTT_UNAME;
      doc["mqtt"]["pass"] = MQTT_PASS;
      doc["mqtt"]["qos"] = MQTT_QoS;
      doc["mqtt"]["auth"] = true;
    }
    TickerForTimeOut.once_ms(10,[doc]{
      write_device_config(doc);
      TickerForTimeOut.once_ms(10,[doc]{
        generate_mqtt_topics();
      });
    });
  }
  else
  {
    JsonDocument return_doc;
    return_doc["done"] = false;
    return_doc["error"] = "No config available";
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);
  }
  

}
void web_scan_wifi(AsyncWebServerRequest *request)
{
  WiFi.scanNetworksAsync([request](int networksFound){
    JsonDocument wifi_ssid;
    JsonArray ssid = wifi_ssid.createNestedArray("ssid");
    for(int i=0; i<networksFound; i++)
    {
      JsonDocument wifi;
      wifi["ssid"] = WiFi.SSID(i);
      wifi["RSSI"] = String(WiFi.RSSI(i));
      ssid.add(wifi);
    }
    String return_msg;
    serializeJson(wifi_ssid, return_msg);
    request->send(200, "application/json", return_msg);
  });
}

void device_template(AsyncWebServerRequest *request)
{
  String res = send_device_template(false);
  request->send(200, "application/json", res);
}

void web_update_login(AsyncWebServerRequest *request)
{
  if(request->hasParam("options"))
  {
    JsonDocument login_option;
    String option = request->arg("options");
    deserializeJson(login_option, option);

    String http_username = login_option["uname"];
    String http_password = login_option["password"];
    conf.http_username = http_username;
    conf.http_password = http_password;
    write_config(conf);

    String return_msg = "";
    JsonDocument return_doc;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg); 
  }
}
void web_set_wifi(AsyncWebServerRequest *request)
{
  if(request->hasParam("options"))
  {
    JsonDocument wifi_option;
    String option = request->arg("options");
    deserializeJson(wifi_option, option);

    String ssid = wifi_option["ssid"];
    String pass = wifi_option["pass"];
    conf.WiFi_SSID = ssid;
    conf.WiFi_PASS = pass;
    conf.wifi_setup_done = true;
    conf.setupFlag = false;
    write_config(conf);
    String return_msg = "";
    JsonDocument return_doc;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);

    WiFi.disconnect();
    TickerForTimeOut.once(1,[request](){
      WiFi.mode(WIFI_STA);
      WiFi.begin(conf.WiFi_SSID,conf.WiFi_PASS);
      int start_time = millis();
      TickerForTimeOut.once_ms(1000,[start_time](){
        if(WiFi.status() != WL_CONNECTED)
        {
          if((millis() - start_time) > 60000)
            reset();
        }
        else
        {
          ESP.reset();
        }        
      });
    });

  }
  else
  {
    request->send(404);
  }
  
}
/*-------Web Server Controller------------------------------*/
/*---------Firmware Update---------------------------------*/
// Simple Firmware Update Form
void firmware_web_updater()
{
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!ap_enabled)
    {
      if(!request->authenticate(conf.http_username.c_str(), conf.http_password.c_str()))
      {
        return request->requestAuthentication();
      }
    }
    String current_version = FIRMWARE_V;
    request->send(200, "text/html", "<script>\
    function httpGet(action, options = {})\
    {\
        if(action == \"device\")\
        {\
            theUrl = '/control?device='+options;\
        }\
        try\
        {\
            var xmlHttp = new XMLHttpRequest();\
            xmlHttp.open( \"GET\", theUrl, false );\
            xmlHttp.send( null );\
            return xmlHttp.responseText;\
        }\
        catch(err){\
            return JSON.stringify({\"done\":false,\"error\":err});\
        }\
    }\
    </script>\
    <h4><a href=\"https://github.com/aviralverma-8877/IoTConnect-Firmware-Releases\" target=\"_blank\">Click Here</a> to download latest firmware.</h4>\
    <b>(Current Firmware Version : "+current_version+")</b><br /><hr />\
    <form method='POST' action='/update_flash' enctype='multipart/form-data'>\
      <input type='file' placeholder='firmware.bin' name='update'><input type='submit' value='Update'> firmware.bin\
    </form></ br>\
    <form method='POST' action='/update_spiffs' enctype='multipart/form-data'>\
      <input type='file' placeholder='spiffs.bin' name='update'><input type='submit' value='Update'> spiffs.bin\
    </form><hr />\
    <button onclick=\"if(confirm('Are you sure you want to reset this device?')){httpGet('device','{\\'action\\':\\'reset\\'}')}\">Reset</button>");
  });

  server.on("/update_flash", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    if(shouldReboot)
    {
      request->send_P(200, "text/html", "Upload successfull, Rebooting....<br /><a href='/'>Home Page</a>\
      <script>\
        setTimeout(\
          function()\
            {\
              window.location.href = \"/\"\
            },10000);\
      </script>");
    }
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      if(debugging)
        Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000, U_FLASH)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
      if(debugging)
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        if(debugging)
          Serial.printf("Update Success: %uB\n", index+len);
        TickerForTimeOut.once(1,[](){
          ESP.reset();
        });
      } else {
        if(debugging)
          Update.printError(Serial);
      }
    }
  });

  server.on("/update_spiffs", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    if(shouldReboot)
    {
      request->send_P(200, "text/html", "Upload successfull, Rebooting....<br /><a href='/'>Home Page</a>\
      <script>\
        setTimeout(\
          function()\
            {\
              window.location.href = \"/\"\
            },10000);\
      </script>");
    }
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      if(debugging)
        Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
      close_all_fs();
      if (!Update.begin(fsSize, U_FS)){//start with max available size
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
      if(debugging)
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        if(debugging)
          Serial.printf("Update Success: %uB\n", index+len);
        TickerForTimeOut.once(1,[](){
          ESP.reset();
        });
      } else {
        if(debugging)
          Update.printError(Serial);
      }
    }
  });
}
void enable_sta()
{
  if(WiFi.status() != WL_CONNECTED)
  {
    serialDisplay("enable_sta","Enabling STA");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(conf.WiFi_SSID,conf.WiFi_PASS);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
  }
  ap_enabled = false;
}

void enable_ap()
{
  serialDisplay("enable_ap","Enabling AP");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  // WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("IoT Connect", "12345678", 10);
  ap_enabled = true;
}

void disable_ap()
{
  WiFi.mode(WIFI_STA);
}

/*---------Firmware Update---------------------------------*/
void setup_web_server()
{
  connectToWiFi();      //Connect to Access Point or start AP depending on config
  serialDisplay("setup","Enabling DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
/*-------Web Update Server----------------------------------*/
  firmware_web_updater();
/*-------Web Update Server----------------------------------*/
/*-------Web Server Setup-----------------------------------*/
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    handleWebControl(request);
  });
  server.on("/update_fauxmo", HTTP_GET, [](AsyncWebServerRequest *request){
    handlefauxmo(request);
  });
  server.on("/get_status", HTTP_GET, [](AsyncWebServerRequest *request){
    handleWebStatus(request);
  });
  server.on("/scan_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    web_scan_wifi(request);
  });
  server.on("/set_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    web_set_wifi(request);
  });
  server.on("/device_template", HTTP_GET, [](AsyncWebServerRequest *request){
    device_template(request);
  });
  server.on("/update_login", HTTP_GET, [](AsyncWebServerRequest *request){
    web_update_login(request);
  });
  server.on("/update_device_config", HTTP_GET, [](AsyncWebServerRequest *request){
    handleDeviceConfig(request);
  });
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    File index = SPIFFS.open("/index.html", "r");
    if (index) {
      if(!ap_enabled)
      {
        if(!request->authenticate(conf.http_username.c_str(), conf.http_password.c_str()))
        {
          return request->requestAuthentication();
        }
      }
      request->send(SPIFFS, "/index.html", "text/html");
    }
    else{
      request->redirect("/update");
    }
    index.close();
  });
  if(debugging)
  {
    server.serveStatic("/device_config.json", SPIFFS, "/device_config.json");
    server.serveStatic("/config.json", SPIFFS, "/config.json");
    server.serveStatic("/mqtt_topics.json", SPIFFS, "/mqtt_topics.json");
  }
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  server.serveStatic("/bk_img.png", SPIFFS, "/bk_img.png");
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    //fauxmo request handling
      if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
    //fauxmo request handling
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    //fauxmo request handling
      String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
      if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
    //fauxmo request handling
    //Page not found request handling
      if(!ap_enabled)
      {
        if(!request->authenticate(conf.http_username.c_str(), conf.http_password.c_str()))
        {
          return request->requestAuthentication();
        }
      }
      request->redirect("/");
  });
  if(!ap_enabled)
  {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      File index = SPIFFS.open("/index.html", "r");
      if (index) {
        if(!request->authenticate(conf.http_username.c_str(), conf.http_password.c_str()))
        {
          return request->requestAuthentication();
        }
        request->send(SPIFFS, "/index.html", "text/html");
      }
      else{
        request->redirect("/update");
      }
      index.close();
    });
  }
  else
  {
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  }
  server.begin();

/*-------Web Server Setup-----------------------------------*/
  bool setup_flag = bool(conf.setupFlag);
  serialDisplay("setup_web_server","Setup Flag : "+ String(setup_flag));

  if(!ap_enabled)
  {
    if(WiFi.status()!= WL_CONNECTED)
    {
      while(WiFi.status()!= WL_CONNECTED)
      {
        delay(100);
      }
    }

    serialDisplay("setup_web_server","iot-connect-"+chipid);
    WiFi.hostname("iot-connect-"+chipid);

    serialDisplay("setup_web_server","Enabling mdns : iot-connect-"+chipid);
    if (MDNS.begin("iot-connect-"+chipid)) {  //Start mDNS with name esp8266
      MDNS.addService("http", "tcp", 80);
      serialDisplay("setup_web_server","MDNS started");
    }
    else{
      serialDisplay("setup_web_server","MDNS failed");
    }
  }
}