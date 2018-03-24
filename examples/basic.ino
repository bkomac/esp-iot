#include <Espiot.h>

// Vcc measurement
ADC_MODE(ADC_VCC);

Espiot espiot;

PubSubClient mq;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup ...");

  delay(300);

  // init with version set
  espiot.init("1.0.1");

  // must be set above: ADC_MODE(ADC_VCC);
  // you can't use this feature with Sonoff switch!!!
  espiot.enableVccMeasure();

  // set sensor description
  espiot.SENSOR = "DS18B20";

  espiot.server.on("/switch", HTTP_GET, []() {
    // using internal blink function: blink(numBlinks, interval)
    espiot.blink();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root['id'] = espiot.getDeviceId();
    root["adc"] = espiot.adc;

    String content;
    root.printTo(content);
    espiot.server.send(200, "application/json", content);
  });

  mq = espiot.getMqClient();
  mq.setCallback(mqCallback2);
}

void loop() {

  // Send mqtt msg. Topic is equal to deviceId.
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root['id'] = espiot.getDeviceId();
  root["adc"] = espiot.adc;

  String content;
  root.printTo(content);

  espiot.mqPublish(content);

  // you can send mqtt msg to subTopic
  espiot.mqPublishSubTopic("subtopic", content);

  // do internal stuff
  espiot.loop();

  delay(300);
}

void mqCallback2(char *topic, byte *payload, unsigned int length) {
  Serial.print(F("**Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  String msg = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg = msg + (char)payload[i];
  }
  Serial.println();
  espiot.blink(3, 50);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &rootMqtt = jsonBuffer.parseObject(msg);
}
