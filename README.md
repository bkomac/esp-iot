#ESP8266 IoT base lib

Supports:
- REST API for configuration (saved to ESP file system)
- MQTT support
- WiFi SSID setup
- mDNS support (avahi)

#CONFIG#
```json
{
  "securityToken": "",
  "timeOut": 10000,
  "relleyPin": 500,
  "sensorInPin": 400,
  "buttonPin": 100,
  "statusLed": 2,
  "lightTreshold": 50,
  "defaultMode": "",
  "mqttAddress": "192.168.1.2",
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
  "deviceName": ""
}
```
#STATUS#
```json
{
  "rc": 200,
  "mode": "",
  "status": "off",
  "meta": {
    "espIotVersion": "1.0.3",
    "appVersion": "1.0.1",
    "sensor": "DS18B20",
    "id": "ESP5ccf7ff0e88",
    "deviceName": "",
    "adc_vcc": 3.082031,
    "ssid": "iottest",
    "rssi": -66,
    "mdns": "ESP5ccf7ff0e88",
    "freeHeap": 27264,
    "upTimeSec": 175564
  }
}
```
