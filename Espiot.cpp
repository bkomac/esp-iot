
#include "Espiot.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <FS.h>

// APP
String FIRM_VER = "1.0.4";
String SENSOR = "ESP"; // BMP180, HTU21, DHT11, DS18B20
String appVersion = "1.0.0";

int BUILTINLED = 2;

String app_id = "";
float adc;
String espIp;

String apSsid = "Config_" + app_id;
String apPass = "esp12345";
int rssi;
String ssid;
int apStartTime;
long startTime;
int apTimeOut = 300000;
boolean vccMode = false;

// CONF
char deviceName[200] = "ESP";
char essid[40] = "";
char epwd[40] = "";
String securityToken = "";
String defaultMODE;
String MODE = "AUTO";
int timeOut = 5000;

// mqtt config
char mqttAddress[200] = "";
int mqttPort = 1883;
char mqttUser[20] = "";
char mqttPassword[20] = "";
char mqttPublishTopic[200] = "esp";
char mqttSuscribeTopic[200] = "esp";

// REST API CONFIG
char rest_server[40] = "";
boolean rest_ssl = false;
char rest_path[200] = "";
int rest_port = 80;
char api_token[100] = "";
char api_payload[400] = "";

ESP8266WebServer server(80);

WiFiClient client;
PubSubClient mqClient(client);

// MDNS
String mdns = "";

Espiot::Espiot() {}

void Espiot::init() { init("1.0.0"); }

void Espiot::init(String appVer) {
  appVersion = appVer;
  pinMode(BUILTINLED, OUTPUT);
  readFS();

  Serial.print(F("**Security token: "));
  Serial.println(securityToken);
  app_id = "ESP" + getMac();
  apSsid = "Config_" + app_id;
  apPass = "esp12345";

  startTime = millis();

  Serial.print(F("**App ID: "));
  Serial.println(app_id);
  app_id.toCharArray(deviceName, 200, app_id.length() + 2);

  connectToWiFi();
}

void Espiot::loop() {
  yield();

  if (WiFi.status() != WL_CONNECTED && apStartTime + apTimeOut < millis()) {
    Serial.print("\nRetray to connect to AP... " + String(essid));
    Serial.print("\nstatus: " + String(WiFi.status()) + " " +
                 String(apStartTime + apTimeOut) + " < " + String(millis()));
    testWifi();
  }

  rssi = WiFi.RSSI();
  yield();
  if (vccMode == true)
    adc = ESP.getVcc() / 1024.00f;

  server.handleClient();

  // MQTT client
  if (String(mqttSuscribeTopic) != "")
    mqClient.loop();
}

void Espiot::connectToWiFi() {

  // auto connect
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  if (String(essid) != "" && String(essid) != "nan") {
    Serial.print("SID found. Trying to connect to: ");
    Serial.print(essid);
    Serial.println("");
    WiFi.begin(essid, epwd);
    delay(100);
  }

  if (!testWifi())
    setupAP();
  else {
    if (!MDNS.begin(app_id.c_str())) {
      Serial.println("Error setting up MDNS responder!");
      mdns = "error";
    } else {
      Serial.println("mDNS responder started");
      // Add service to MDNS-SD
      MDNS.addService("http", "tcp", 80);
      mdns = app_id.c_str();
    }
  }

  ssid = WiFi.SSID();
  Serial.print(F("\nconnected to "));
  Serial.print(ssid);
  Serial.print(F(" rssi:"));
  Serial.print(rssi);
  Serial.print(F(" status: "));
  Serial.print(WiFi.status());
  Serial.print(F(" = "));
  Serial.println(WL_CONNECTED + String("-WL_CONNECTED?"));

  Serial.println(" ");
  IPAddress ip = WiFi.localIP();
  espIp = getIP(ip);
  Serial.print(F("local ip: "));
  Serial.println(espIp);
  yield();
  createWebServer();

  // MQTT
  if (String(mqttAddress) != "") {
    Serial.print(F("Seting mqtt server and callback... "));
    mqClient.setServer(mqttAddress, mqttPort);
    /*mqClient.setCallback(
        [this](char *topic, byte *payload, unsigned int length) {
          this->mqCallback(topic, payload, length);
      }); */
    mqReconnect();
  }
}

PubSubClient Espiot::getMqClient() { return mqClient; }

void Espiot::setupAP(void) {
  Serial.print(F("Setting up access point. WiFi.mode= "));
  Serial.println(WIFI_STA);
  WiFi.mode(WIFI_STA);
  // WiFi.disconnect();

  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println(F("Scanning network done."));
  if (n == 0)
    Serial.println(F("No networks found."));
  else {
    Serial.print(n);
    Serial.println(F(" networks found"));
    Serial.println(F("---------------------------------"));
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(F(": "));
      Serial.print(WiFi.SSID(i));
      Serial.print(F(" ("));
      Serial.print(WiFi.RSSI(i));
      Serial.print(F(")"));
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  delay(100);

  WiFi.softAP(apSsid.c_str(), apPass.c_str());
  Serial.print(F("SoftAP ready. AP SID: "));
  Serial.print(apSsid);

  IPAddress apip = WiFi.softAPIP();
  Serial.print(F(", AP IP address: "));

  String softAp = getIP(apip);
  apStartTime = millis();
  Serial.println(apip);
  espIp = String(apip);
  blink(5, 100);
}

// MQTT
void Espiot::mqCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  String msg = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg = msg + (char)payload[i];
  }
  Serial.println();
  blink(3, 50);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &rootMqtt = jsonBuffer.parseObject(msg);
}

bool Espiot::mqReconnect() {
  yield();
  if (!mqClient.connected()) {
    Serial.print(F("\nAttempting MQTT connection... "));
    Serial.print(mqttAddress);
    Serial.print(F(":"));
    Serial.print(mqttPort);
    Serial.print(F("@"));
    Serial.print(mqttUser);
    Serial.print(F(" Pub: "));
    Serial.print(mqttPublishTopic);
    Serial.print(F(", Sub: "));
    Serial.print(mqttSuscribeTopic);

    yield();
    // Attempt to connect
    if (mqClient.connect(app_id.c_str(), mqttUser, mqttPassword)) {
      yield();
      Serial.print(F("\nconnected with cid: "));
      Serial.println(app_id);

      // suscribe
      if (String(mqttSuscribeTopic) != "") {
        mqClient.subscribe(String(mqttSuscribeTopic).c_str());
        Serial.print(F("\nSuscribed to toppic: "));
        Serial.println(mqttSuscribeTopic);
      }

    } else {
      Serial.print(F("\nfailed to connect! Client state: "));
      Serial.println(mqClient.state());
    }
  }
  return mqClient.connected();
}

void Espiot::mqPublish(String msg) {

  if (mqReconnect()) {

    // mqClient.loop();

    Serial.print(F("\nPublish message to topic '"));
    Serial.print(mqttPublishTopic);
    Serial.print(F("':"));
    Serial.println(msg);
    mqClient.publish(String(mqttPublishTopic).c_str(), msg.c_str());

  } else {
    Serial.print(F("\nPublish failed!"));
  }
}

void Espiot::readFS() {
  // read configuration from FS json
  Serial.println(F("-mounting FS..."));

  if (SPIFFS.begin()) {
    Serial.println(F("-mounted file system"));
    if (SPIFFS.exists("/config.json")) {
      // file exists, reading and loading
      Serial.println(F("-reading config.json file"));
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println(F("-opened config.json file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        DynamicJsonBuffer configJsonBuffer;
        JsonObject &jsonConfig = configJsonBuffer.parseObject(buf.get());
        jsonConfig.printTo(Serial);

        if (jsonConfig.success()) {
          Serial.println(F("\nparsed config.json"));

          // config parameters
          securityToken = jsonConfig["securityToken"].asString();

          String deviceName1 = jsonConfig["deviceName"].asString();
          deviceName1.toCharArray(deviceName, 200, deviceName1.length() + 2);

          String builtInLed1 = jsonConfig["statusLed"];
          BUILTINLED = builtInLed1.toInt();

          String timeOut1 = jsonConfig["timeOut"];
          timeOut = timeOut1.toInt();

          defaultMODE = jsonConfig["defaultMode"].asString();
          MODE = defaultMODE;

          String mqttAddress1 = jsonConfig["mqttAddress"].asString();
          mqttAddress1.toCharArray(mqttAddress, 200, 0);
          String mqttPort1 = jsonConfig["mqttPort"];
          mqttPort = mqttPort1.toInt();

          String mqttUser1 = jsonConfig["mqttUser"].asString();
          mqttUser1.toCharArray(mqttUser, 20, 0);
          String mqttPassword1 = jsonConfig["mqttPassword"].asString();
          mqttPassword1.toCharArray(mqttPassword, 20, 0);

          String mqttPubTopic1 = jsonConfig["mqttPublishTopic"].asString();
          mqttPubTopic1.toCharArray(mqttPublishTopic, 200, 0);
          String mqttSusTopic1 = jsonConfig["mqttSuscribeTopic"].asString();
          mqttSusTopic1.toCharArray(mqttSuscribeTopic, 200, 0);

          String rest_server1 = jsonConfig["restApiServer"].asString();
          rest_server1.toCharArray(rest_server, 40, 0);

          String rest_ssl1 = jsonConfig["restApiSSL"];
          if (rest_ssl1 == "true")
            rest_ssl = true;
          else
            rest_ssl = false;

          String rest_path1 = jsonConfig["restApiPath"].asString();
          rest_path1.toCharArray(rest_path, 200, 0);
          String rest_port1 = jsonConfig["restApiPort"];
          rest_port = rest_port1.toInt();
          String api_token1 = jsonConfig["restApiToken"].asString();
          api_token1.toCharArray(api_token, 200, 0);
          String api_payload1 = jsonConfig["restApiPayload"].asString();
          api_payload1.toCharArray(api_payload, 400, 0);

        } else {
          Serial.println(F("-failed to load json config!"));
        }
      }
    } else {
      Serial.println(F("Config.json file doesn't exist yet!"));
    }

    // ssid.json
    if (SPIFFS.exists("/ssid.json")) {
      // file exists, reading and loading
      Serial.println(F("-reading ssid.json file"));
      File ssidFile = SPIFFS.open("/ssid.json", "r");
      if (ssidFile) {
        Serial.println(F("-opened ssid file"));
        size_t size = ssidFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        ssidFile.readBytes(buf.get(), size);
        DynamicJsonBuffer ssidJsonBuffer;
        JsonObject &jsonConfig = ssidJsonBuffer.parseObject(buf.get());

        jsonConfig.printTo(Serial);
        if (jsonConfig.success()) {
          Serial.println(F("\nparsed ssid.json"));

          // ssid parameters
          String ssid1 = jsonConfig["ssid"].asString();
          ssid1.toCharArray(essid, 40, 0);
          String pwd1 = jsonConfig["password"].asString();
          pwd1.toCharArray(epwd, 40, 0);

        } else {
          Serial.println(F("failed to load ssid.json"));
        }
      }
    } else {
      Serial.println(F("ssid.json file doesn't exist yet!"));
    }
  } else {
    Serial.println(F("failed to mount FS"));
    blink(10, 50, 20);
  }
  // end read
}

void Espiot::saveConfig(JsonObject &json) {

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println(F("failed to open config file for writing"));
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

void Espiot::saveSsid(JsonObject &json) {

  File configFile = SPIFFS.open("/ssid.json", "w");
  if (!configFile) {
    Serial.println(F("failed to open ssid.json file for writing"));
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

// web server
void Espiot::createWebServer() {
  Serial.println(F("Starting server..."));
  yield();
  Serial.println(F("REST handlers init..."));

  server.on("/", [this]() { onRoot(); });

  server.on("/status", HTTP_GET, [this]() { onStatusGET(); });

  server.on("/update", HTTP_GET, [this]() { onUpdateGET(); });

  server.on("/update", HTTP_POST, [this]() { onUpdatePOST(); });

  server.on("/config", HTTP_GET, [this]() { onConfigGET(); });

  server.on("/config", HTTP_POST, [this]() { onConfigPOST(); });

  server.on("/config", HTTP_OPTIONS, [this]() { onConfigOPTIONS(); });

  server.on("/ssid", HTTP_GET, [this]() { onSsidGET(); });

  server.on("/ssid", HTTP_POST, [this]() { onSsidPOST(); });

  server.on("/reset", HTTP_DELETE, [this]() { onResetDELETE(); });

  server.on("/reset", HTTP_GET, [this]() { onResetGET(); });

  server.begin();
  Serial.println("Server started");

} //--

void Espiot::onRoot() {
  blink();
  String content;

  content = "<!DOCTYPE HTML>\r\n<html>";
  content += "<h1><u>ESP web server</u></h1>";
  content += "<p>IP: " + espIp + "</p>";
  content += "<p>MAC/AppId: " + app_id + "</p>";
  content += "<p>Version: " + FIRM_VER + "</p>";
  content += "<p>ADC: " + String(adc) + "</p>";
  content += "<br><p><b>REST API: </b>";

  content += "<br>GET: <a href='http://" + espIp + "/status'>http://" + espIp +
             "/status </a>";
  content += "<br>GET: <a href='http://" + espIp + "/update'>http://" + espIp +
             "/update </a>";
  content += "<br>POST: <a href='http://" + espIp + "/update'>http://" + espIp +
             "/update </a>";
  content += "<br>GET: <a href='http://" + espIp + "/config'>http://" + espIp +
             "/config </a>";
  content += "<br>POST: <a href='http://" + espIp + "/config'>http://" + espIp +
             "/config </a>";
  content += "<br>GET: <a href='http://" + espIp + "/ssid'>http://" + espIp +
             "/ssid </a>";
  content += "<br>POST: <a href='http://" + espIp + "/ssid'>http://" + espIp +
             "/ssid </a>";
  content += "<br>GET: <a href='http://" + espIp + "/reset'>http://" + espIp +
             "/reset </a>";
  content += "<br>DELETE: <a href='http://" + espIp + "/reset'>http://" +
             espIp + "/reset </a>";

  content += "</html>";
  server.send(200, "text/html", content);
};

void Espiot::onStatusGET() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["id"] = app_id;
  root["deviceName"] = deviceName;
  JsonObject &meta = root.createNestedObject("meta");
  meta["espIotVersion"] = FIRM_VER;
  meta["appVersion"] = appVersion;
  meta["sensor"] = SENSOR;
  meta["adc_vcc"] = adc;
  meta["ssid"] = essid;
  meta["rssi"] = rssi;
  meta["mdns"] = mdns;
  meta["freeHeap"] = ESP.getFreeHeap();
  meta["upTimeSec"] = (millis() - startTime) / 1000;

  String content;
  root.printTo(content);
  server.send(200, "application/json", content);

  Serial.print("\n\n/status: ");
  root.printTo(Serial);
};

void Espiot::onUpdateGET() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["url"] = "Enter url to bin file here and POST json object to ESP.";

  String content;
  root.printTo(content);
  server.send(200, "application/json", content);
};

void Espiot::onUpdatePOST() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(server.arg("plain"));
  String content;

  String secToken = root["securityToken"].asString();
  if (secToken == securityToken) {
    Serial.println("Updating firmware...");
    String message = "";
    //    for (uint8_t i = 0; i < server.args(); i++) {
    //      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    //    }

    String url = root["url"];
    Serial.println("");
    Serial.print("Update url: ");
    Serial.println(url);

    blink(10, 80);

    t_httpUpdate_return ret = ESPhttpUpdate.update(url);

    switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s",
                    ESPhttpUpdate.getLastError(),
                    ESPhttpUpdate.getLastErrorString().c_str());
      root["rc"] = ESPhttpUpdate.getLastError();
      message += "HTTP_UPDATE_FAILD Error (";
      message += ESPhttpUpdate.getLastError();
      message += "): ";
      message += ESPhttpUpdate.getLastErrorString().c_str();
      root["msg"] = message;
      root.printTo(content);
      server.send(400, "application/json", content);
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");

      root["msg"] = "HTTP_UPDATE_NO_UPDATES";
      root.printTo(content);
      server.send(400, "application/json", content);
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");

      root["msg"] = "HTTP_UPDATE_OK";
      root.printTo(content);
      server.send(200, "application/json", content);
      break;
    }
  } else {

    root["msg"] = "Security token is not valid!";
    root.printTo(content);
    server.send(401, "application/json", content);
  }
};

void Espiot::onConfigGET() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["securityToken"] = "";

  root["statusLed"] = BUILTINLED;
  root["timeOut"] = timeOut;

  root["mqttAddress"] = mqttAddress;
  root["mqttPort"] = mqttPort;
  root["mqttUser"] = mqttUser;
  root["mqttPassword"] = "******";

  root["mqttPublishTopic"] = mqttPublishTopic;
  root["mqttSuscribeTopic"] = mqttSuscribeTopic;

  root["restApiServer"] = rest_server;
  root["restApiSSL"] = rest_ssl;
  root["restApiPath"] = rest_path;
  root["restApiPort"] = rest_port;
  root["restApiToken"] = api_token;
  root["restApiPayload"] = api_payload;

  root["deviceName"] = deviceName;

  String content;
  root.printTo(content);
  server.send(200, "application/json", content);

  Serial.print("\n\n/config: ");
  root.printTo(Serial);
};

void Espiot::onConfigPOST() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(server.arg("plain"));
  Serial.println(F("\nSaving config..."));
  String content;

  String secToken = root["securityToken"].asString();
  if (secToken == securityToken) {

    securityToken = root["newSecurityToken"].asString();
    root["securityToken"] = root["newSecurityToken"];

    String builtInLed1 = root["statusLed"];
    BUILTINLED = builtInLed1.toInt();
    String timeOut1 = root["timeOut"];
    timeOut = timeOut1.toInt();

    String mqttAddress1 = root["mqttAddress"].asString();
    mqttAddress1.toCharArray(mqttAddress, 200, 0);
    String mqttPort1 = root["mqttPort"];
    mqttPort = mqttPort1.toInt();
    String mqttUser1 = root["mqttUser"].asString();
    mqttUser1.toCharArray(mqttUser, 20, 0);
    String mqttPassword1 = root["mqttPassword"].asString();
    mqttPassword1.toCharArray(mqttPassword, 20, 0);

    String mqttPubTopic1 = root["mqttPublishTopic"].asString();
    mqttPubTopic1.toCharArray(mqttPublishTopic, 200, 0);
    String mqttSusTopic1 = root["mqttSuscribeTopic"].asString();
    mqttSusTopic1.toCharArray(mqttSuscribeTopic, 200, 0);

    String rest_server1 = root["restApiServer"].asString();
    rest_server1.toCharArray(rest_server, 40, 0);

    String rest_ssl1 = root["restApiSSL"];
    if (rest_ssl1 == "true")
      rest_ssl = true;
    else
      rest_ssl = false;

    String rest_path1 = root["restApiPath"].asString();
    rest_path1.toCharArray(rest_path, 200, 0);
    String rest_port1 = root["restApiPort"];
    rest_port = rest_port1.toInt();
    String api_token1 = root["restApiToken"].asString();
    api_token1.toCharArray(api_token, 200, 0);
    String api_payload1 = root["restApiPayload"].asString();
    api_payload1.toCharArray(api_payload, 400, 0);

    String deviceName1 = root["deviceName"].asString();
    deviceName1.toCharArray(deviceName, 200, deviceName1.length() + 2);

    root.printTo(Serial);

    saveConfig(root);

    Serial.println(F("\nConfiguration is saved."));

    root["msg"] = "Configuration is saved.";
    root.printTo(content);
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers",
                      "Origin, X-Requested-With, Content-Type, Accept");
    server.send(200, "application/json", content);
  } else {
    Serial.println(F("\nSecurity tokent not valid!"));

    root["msg"] = "Security tokent not valid!";
    root.printTo(content);
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers",
                      "Origin, X-Requested-With, Content-Type, Accept");
    server.send(401, "application/json", content);
  }
  if (mqttAddress != "") {
    mqClient.setServer(mqttAddress, mqttPort);
  }
};

void Espiot::onConfigOPTIONS() {
  blink();

  // server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers",
                    "Origin, X-Requested-With, Content-Type, Accept");
  server.send(200, "text/plain", "");
};

void Espiot::onSsidGET() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["ssid"] = essid;
  root["password"] = epwd;

  root["connectedTo"] = WiFi.SSID();
  root["localIP"] = WiFi.localIP().toString();

  root["softAPIP"] = WiFi.softAPIP().toString();

  String content;
  root.printTo(content);
  server.send(200, "application/json", content);
};

void Espiot::onSsidPOST() {
  blink(5);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(server.arg("plain"));
  Serial.print(F("\nSetting ssid to "));

  String secToken = root["securityToken"].asString();
  if (secToken == securityToken) {
    String ssid1 = root["ssid"].asString();
    ssid1.toCharArray(essid, 40, 0);
    String pwd1 = root["password"].asString();
    pwd1.toCharArray(epwd, 40, 0);

    Serial.println(essid);

    Serial.println(F("\nNew ssid is set. ESP will reconect..."));
    root["msg"] = "New ssid is set. ESP will reconect.";

    String content;
    root.printTo(content);
    saveSsid(root);
    server.send(200, "application/json", content);

    delay(500);
    ESP.eraseConfig();
    delay(1000);
    WiFi.disconnect();
    delay(1000);
    WiFi.mode(WIFI_STA);
    delay(1000);
    WiFi.begin(essid, epwd);
    delay(1000);
    testWifi();
  } else {
    Serial.println(F("\nSecurity token not valid!"));

    root["msg"] = "Security token not valid!";

    String content;
    root.printTo(content);
    server.send(401, "application/json", content);
  }
};

void Espiot::onResetDELETE() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  String secToken = root["securityToken"].asString();
  if (secToken == securityToken) {

    root["msg"] = "Reseting ESP config. Configuration will be erased ...";
    Serial.println(F("\nReseting ESP config. Configuration will be erased..."));

    String content;
    root.printTo(content);
    server.send(200, "application/json", content);
    delay(3000);

    // clean FS, for testing
    SPIFFS.format();
    delay(1000);
    // reset settings - for testing
    WiFi.disconnect(true);
    delay(1000);

    ESP.eraseConfig();

    ESP.reset();
    delay(5000);
  } else {

    root["msg"] = "Security token not valid!";
    Serial.println(F("\nSecurity token not valid!"));

    String content;
    root.printTo(content);
    server.send(401, "application/json", content);
  }
};

void Espiot::onResetGET() {
  blink();
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["msg"] = "Reseting ESP...";
  Serial.println(F("\nReseting ESP..."));

  String content;
  root.printTo(content);
  server.send(200, "application/json", content);

  ESP.reset();
  delay(5000);
};

// tesing for wifi connection
bool Espiot::testWifi() {
  int c = 0;
  Serial.println("\nWaiting for Wifi to connect...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(essid, epwd);
  apStartTime = millis();
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print(F("WiFi connected to "));
      Serial.println(WiFi.SSID());
      Serial.print(F("IP: "));
      IPAddress ip = WiFi.localIP();
      Serial.println(getIP(ip));

      blink(2, 30, 1000);
      return true;
    }
    blink(1, 200);
    delay(500);
    Serial.print(F("Retrying to connect to WiFi ssid: "));
    Serial.print(essid);
    Serial.print(F(" status="));
    Serial.println(WiFi.status());

    c++;
  }
  Serial.println(F(""));
  Serial.println(F("Connect timed out."));

  blink(20, 30);
  delay(1000);
  // reset esp
  // ESP.reset();
  // delay(3000);
  return false;
}

// blink
void Espiot::blink() { blink(1, 30, 30); }

void Espiot::blink(int times) { blink(times, 30, 30); }

void Espiot::blink(int times, int milisec) { blink(times, milisec, milisec); }

void Espiot::blink(int times, int himilisec, int lowmilisec) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUILTINLED, LOW);
    delay(lowmilisec);
    digitalWrite(BUILTINLED, HIGH);
    delay(himilisec);
  }
}

String Espiot::getIP(IPAddress ip) {
  String ipo = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' +
               String(ip[3]);
  if (ipo == "0.0.0.0")
    ipo = "";
  return ipo;
}

String Espiot::getMac() {
  unsigned char mac[6];
  WiFi.macAddress(mac);
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
  }
  return result;
}

String Espiot::getDeviceId() { return app_id; }

void Espiot::enableVccMeasure() { vccMode = true; }
