#include <Arduino.h>
#include <WiFiServerBasics.h>
#include <CredWiFi.h>
#include <PubSubClient.h>
#include <CredAdafruit.h>

WiFiClient wiFiClient;
PubSubClient mqtt(wiFiClient);
void callback(char *topic, byte *data, uint length);

#define TEST true
#define FEED_SHORT_DUMP "food-dump-short"
#define FEED_LONG_DUMP "food-dump-long"
#define FEED_ONOFF_REQ "food-dump-onoff-req"
#define FEED_ONOFF_RESP "food-dump-onoff-resp"
const int pinSolenoid = D1;
const int pinLed = LED_BUILTIN;
const ulong ONE_SEC = 1000000UL;       // Broj mikrosekundi u 1 sec.
const uint SLEEP = TEST ? 20 : 5 * 60; // Trajanje (u sec) spavanja aparata.
const uint AWAKE_SHORT = 10;           // Trajanje (u sec) budnosti aparata za ispitivanje da li da ostane budan.
const uint AWAKE_LONG = 3 * 60;        // Trajanje (u sec) budnosti aparata dok ceka neku pull komandu.
bool stayAwake = false;                //
ulong msLastAct;                       // Vreme (u ms) poslednje akcije: start, short/long pull.

void sleep()
{
    if (mqtt.connected())
    {
        mqtt.publish(AIO_USER AIO_FEEDS FEED_ONOFF_RESP, "0");
        delay(100);
    }
    ESP.deepSleep(SLEEP * ONE_SEC, RF_DEFAULT);
}

void setup()
{
    Serial.begin(115200);
    delay(100);
    pinMode(pinSolenoid, OUTPUT);
    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, false);

    ConnectToWiFi();
    Serial.print("\nWiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
    mqtt.setServer(AIO_SERVER, AIO_SERVERPORT);
    mqtt.setCallback(callback);

    Serial.println("Attempting MQTT connection...");
    if (mqtt.connect("", AIO_USER, AIO_KEY))
    {
        Serial.println("Connected");
        mqtt.subscribe(AIO_USER AIO_FEEDS FEED_ONOFF_REQ, 0);
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(mqtt.state());
        for (uint i = 0; i < 5; i++)
        {
            digitalWrite(pinLed, i % 2);
            delay(1000);
        }
        sleep();
    }
    msLastAct = millis();
}

void loop()
{
    if (stayAwake && millis() - msLastAct > AWAKE_LONG * 1000)
        sleep();
    if (!stayAwake && millis() - msLastAct > AWAKE_SHORT * 1000)
        sleep();

    mqtt.loop();

    delay(200);
}

void callback(char *topic, byte *data, uint length)
{
    Serial.println(topic);
    //B if(strends(topic, FEED_ONOFF_REQ))
    size_t tlen = strlen(topic);
    if (topic[tlen - 1] == 'q') // ...onoff-req
    {
        stayAwake = length >= 2 && data[1] == 'N'; // stayAwake je true ako je data -> "ON"
        msLastAct = millis();
        mqtt.publish(AIO_USER AIO_FEEDS FEED_ONOFF_RESP, stayAwake ? "1" : "0");
        delay(100);

        //T
        // for (uint i = 0; i < length; i++)
        //     Serial.print((char)data[i]);
        // Serial.println();
    }
}