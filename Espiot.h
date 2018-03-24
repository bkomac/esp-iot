/*
  ESPIoT Library
*/
#ifndef espiot_h
#define espiot_h

#include <Arduino.h>
#include <FS.h>

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <PubSubClient.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <ArduinoJson.h>

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

  void onWiFiGET();
  void onWiFiOPTIONS();
  void onWiFiPOST();
  void onResetGET();
  void onResetDELETE();
  void onSettingPOST();
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
  String deviceName;
  int timeOut;
  int lightThreshold;
  String SENSOR;
  String defaultMODE;
  float adc;

  String securityToken;
  String appVersion;
  String apSsid;
  String apPass;
  ESP8266WebServer server;
  WiFiClient client;
  // PubSubClient mqClient;

  PubSubClient getMqClient();
  void mqPublish(String msg);
  void mqPublishSubTopic(String subTopic, String msg);
  String sendRequest(String data);

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
