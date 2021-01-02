#include <Arduino.h>
#include <WiFiServerBasics.h>
#include <CredWiFi.h>
#include <PubSubClient.h>
#include <CredAdafruit.h>

WiFiClient wiFiClient;
PubSubClient mqtt(wiFiClient);
void callback(char *topic, byte *data, uint length);

#define FEED_SHORT_DUMP "food-dump-short"
#define FEED_LONG_DUMP "food-dump-long"
const int pinSolenoid = D1;
const int pinModeSwitch = D2; // Prekidac za izbor moda: MQTT ili touch.
const int SHORT_PULL = 75;        // Kratko aktiviranje solenoida (ispustanje hrane).
const int LONG_PULL = 300;        // Dugo aktiviranje solenoida (ispustanje hrane).
const int MAX_SOLENOID_ON = 2000; // Maksimalno trajanje (u ms) aktiviranog/povucenog solenoida.
uint itvPrepareToDump;            // Trajanje (u ms) cekanja pred sipanje hrane.

// Moguce vrednosti za dugmice u AIO FoodDump dashboard-u.
enum ButtonStatus
{
    BTN_RELEASE = '0',
    BTN_SHORT = '1',
    BTN_LONG = '2'
};

enum DeviceMode
{
    MQTT_Mode, // Hrana se prosipa komandom sa AIO dashboard-a
    TouchMode  // Zekica pipne Touch/Vibration sensor, hrana se prospe, deep sleep
} deviceMode;

#include "Blinky.h"
Blinky led(LED_BUILTIN, false, 1000, 3);

void solenoidON(bool on)
{
    digitalWrite(pinSolenoid, on);
    led.ledOn(on);
}

void reconnect()
{
    if (!WiFi.isConnected())
    {
        led.blink();
        ESP.reset();
    }

    // Loop until we're reconnected
    while (!mqtt.connected())
    {
        Serial.println("MQTT connecting...");
        if (mqtt.connect("", AIO_USER, AIO_KEY))
        {
            Serial.println("Connected");
            mqtt.subscribe(AIO_USER AIO_FEEDS FEED_SHORT_DUMP, 0);
            mqtt.subscribe(AIO_USER AIO_FEEDS FEED_LONG_DUMP, 0);
            led.ledOn(false);
        }
        else
        {
            Serial.print("Failed. MQTT state: ");
            Serial.print(mqtt.state());
            led.blink();
        }
    }
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    led.ledOn(false);
    pinMode(pinSolenoid, OUTPUT);
    pinMode(pinModeSwitch, INPUT_PULLUP);
    delay(100);
    deviceMode = digitalRead(pinModeSwitch) ? MQTT_Mode : TouchMode;

    if (deviceMode == TouchMode)
    {
        itvPrepareToDump = 1000; // cekanje da bi se izbegao burst ispaljivanja granula zbog nestabilnog signala
        delay(itvPrepareToDump);
        solenoidON(true);
        delay(SHORT_PULL);
        solenoidON(false);
        ESP.deepSleep(0);
    }
    if (deviceMode == MQTT_Mode)
    {
        Serial.begin(115200);
        itvPrepareToDump = 5000; // cekanja da se korisnik spremi za posmatranje sipanja hrane
        led.ledOn(true);
        WiFi.mode(WIFI_STA);
        ConnectToWiFi();
        Serial.print("\nWiFi connected, IP address: ");
        Serial.println(WiFi.localIP());
        mqtt.setServer(AIO_SERVER, AIO_SERVERPORT);
        mqtt.setCallback(callback);
    }
}

void loop()
{
    if (deviceMode == MQTT_Mode)
    {
        if (!mqtt.connected())
            reconnect();
        mqtt.loop();
    }
    delay(100);
}

void callback(char *topic, byte *data, uint length)
{
    //T Serial.println(topic);
    //T Serial.println((int)data[0]);
    int ms = -1; // trajanje (u ms) povlacenja poluge solenoida
    if (data[0] == BTN_SHORT)
        ms = SHORT_PULL;
    if (data[0] == BTN_LONG)
        ms = LONG_PULL;
    if (ms > 0 && ms <= MAX_SOLENOID_ON)
    {
        // cekanje da se korisnik pripremi za gledanje prosipanja hrane
        led.ledOn(true);
        delay(itvPrepareToDump);
        led.ledOn(false);
        delay(1000);
        // prosipanje hrane omalenom kunikulusu
        solenoidON(true);
        delay(ms);
        solenoidON(false);
    }
}
