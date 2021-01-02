//* Testiranje MQTT pubilsh opcije (Arduino Hannover).

#include <Arduino.h>
#include <WiFiServerBasics.h>
#include <CredWiFi.h>
#include <PubSubClient.h>
#include <CredArduinoHannover.h>

WiFiClient wiFiClient;
PubSubClient mqtt(wiFiClient);

#define FEED_SHORT_DUMP "food-dump-short"

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, true);
    Serial.begin(115200);
    Serial.println();
    WiFi.mode(WIFI_STA);
    ConnectToWiFi();
    // mqtt.setServer(AIO_SERVER, AIO_SERVERPORT);
    mqtt.setServer(AHAN_SERVER, AHAN_PORT);

    // if (mqtt.connect("", AIO_USER, AIO_KEY))
    if (mqtt.connect("", AHAN_USER, AHAN_PASS))
    {
        Serial.println("mqtt connected");
        //B bool res = mqtt.publish("bvujovic/food-dump-short", "1");
        bool res = mqtt.publish(AHAN_USER FEED_SHORT_DUMP, "1");
        Serial.println("mqtt publish: " + String(res));
    }
    else
        Serial.println("mqtt NOT connected");
}

void loop()
{
    delay(100);
}