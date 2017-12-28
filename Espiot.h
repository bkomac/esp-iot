/*
  ESPIoT Library
*/
#ifndef espiot_h
#define espiot_h

#include "Arduino.h"
#include <FS.h>
#include <Wire.h>

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <PubSubClient.h>
#include <WebSocketsServer.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <ArduinoJson.h>
#include <SPI.h>

class Espiot {
private:
  void createWebServer();
  void readFS();
  void saveConfig(JsonObject &json);
  void saveSsid(JsonObject &json);
  bool testWifi();
  String getIP(IPAddress ip);
  String getMac();
  void setupAP(void);
  void connectToWiFi();
  // void mqPublish(String msg);
  bool mqReconnect();
  void mqCallback(char *topic, byte *payload, unsigned int length);

  void onResetGET();
  void onResetDELETE();
  void onSsidPOST();
  void onSsidGET();
  void onConfigOPTIONS();
  void onConfigPOST();
  void onConfigGET();
  void onUpdatePOST();
  void onUpdateGET();
  void onStatusGET();
  void onRoot();

public:
  char deviceName[100];
  int timeOut;
  String SENSOR;



  String securityToken;
  String appVersion;
  String apSsid;
  String apPass;
  ESP8266WebServer server;
  WiFiClient client;
  // PubSubClient mqClient;

  PubSubClient getMqClient();
  void mqPublish(String msg);

  String getDeviceId();
  void enableVccMeasure();

  Espiot();
  void init(String appVer);
  void init();
  void loop();

  void blink();
  void blink(int times);
  void blink(int times, int milisec);
  void blink(int times, int himilisec, int lowmilisec);
};

#endif
