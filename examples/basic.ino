#include <Espiot.h>

Espiot espiot;

PubSubClient mq;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup ...");

  delay(300);
  espiot.init();

  espiot.server.on("/switch", HTTP_GET, []() {
    espiot.blink();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["rc"] = 200;

    String content;
    root.printTo(content);
    espiot.server.send(200, "application/json", content);
  });

  mq = espiot.getMqClient();
  mq.setCallback(mqCallback2);
}

void loop() {
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
