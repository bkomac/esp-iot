# ESP8266 IoT base lib

Supports:
- REST API for configuration (saved to ESP file system)
- MQTT support
- WiFi SSID setup
- mDNS support (avahi)

### CONFIG [GET,POST]
```json
{
  "securityToken": "",
  "timeOut": 10000,
  "statusLed": 2,
  "lightTreshold": 50,
  "defaultMode": "",
  "mqttAddress": "my.mqtt.broker.net",
  "mqttPort": 1883,
  "mqttUser": "esp",
  "mqttPassword": "******",
  "mqttPublishTopic": "esp/thermal/ESP5ccf7ff0e88",
  "mqttSuscribeTopic": "",
  "restApiServer": "",
  "restApiSSL": 0,
  "restApiPath": "",
  "restApiPort": 80,
  "restApiToken": "",
  "restApiPayload": "",
  "deviceName": "Temperature sensor"
}
```
### STATUS [GET]
```json
{
  "id": "ESP5ccf7ff0e88",
  "deviceName": "Temperature sensor",
  "meta": {
    "espIotVersion": "1.0.3",
    "appVersion": "1.0.1",
    "sensor": "DS18B20", 
    "adc_vcc": 3.082031,
    "ssid": "myWiFi",
    "rssi": -66,
    "mdns": "ESP5ccf7ff0e88",
    "freeHeap": 27264,
    "upTimeSec": 175564
  }
}
```
### SSID CONFIG [GET,POST]
```json
{
  "ssid": "myWiFi",
  "password": "myFiWiPass",
  "connectedTo": "MyWiFi",
  "localIP": "192.168.1.187",
  "softAPIP": "192.168.4.1"
}
```
