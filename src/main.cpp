#include <Arduino.h>
#include <WiFiServerBasics.h>
#include <CredWiFi.h>
#include <PubSubClient.h>
#include <CredAdafruit.h>

WiFiClient wiFiClient;
PubSubClient mqtt(wiFiClient);
typedef unsigned int uint;
void callback(char *topic, byte *data, uint length);

#define FEED_SHORT_DUMP "food-dump-short"
#define FEED_LONG_DUMP "food-dump-long"
#define FEED_LONG_DELAYED "food-dump-time"
const uint SEC = 1000;
const uint MIN = 60 * SEC;
const byte pinSolenoid = D1;
const byte pinModeSwitch = D2;       // Prekidac za izbor moda: MQTT ili touch.
const byte pinWiFi = D8;             // LED indikator WiFi-a (MQTT_Mode);
const int SHORT_PULL = 75;           // Kratko aktiviranje solenoida (ispustanje hrane).
const int LONG_PULL = 300;           // Dugo aktiviranje solenoida (ispustanje hrane).
const int MAX_SOLENOID_ON = SEC; // Maksimalno trajanje (u ms) aktiviranog/povucenog solenoida.
uint itvPrepareToDump;               // Trajanje (u ms) cekanja pred sipanje hrane.
uint msFoodDrop = 0;                 // Vreme za prosipanje hrane kada se koristi time/delayed MQTT opcija.

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
Blinky led(LED_BUILTIN, false, SEC, 3);
Blinky ledWiFi(pinWiFi, true);

void solenoidON(bool on)
{
    digitalWrite(pinSolenoid, on);
    led.ledOn(on);
}

// Priprema (paljenje LEDa) i pull solenoida (trajanje zadato sa ms).
void solenoidPull(uint ms)
{
    // cekanje da se korisnik pripremi za gledanje prosipanja hrane
    led.on();
    delay(itvPrepareToDump);
    led.off();
    delay(SEC);
    Serial.println("Solenoid pull");
    // prosipanje hrane omalenom kunikulusu
    solenoidON(true);
    delay(ms);
    solenoidON(false);
}

void reconnect()
{
    if (!WiFi.isConnected())
    {
        led.blink();
        ESP.reset();
    }

    while (!mqtt.connected())
    {
        Serial.println("MQTT connecting...");
        if (mqtt.connect("", AIO_USER, AIO_KEY))
        {
            Serial.println("Connected");
            mqtt.subscribe(AIO_USER AIO_FEEDS FEED_SHORT_DUMP, 0);
            mqtt.subscribe(AIO_USER AIO_FEEDS FEED_LONG_DUMP, 0);
            mqtt.subscribe(AIO_USER AIO_FEEDS FEED_LONG_DELAYED, 0);
            led.off();
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
    pinMode(pinSolenoid, OUTPUT);
    pinMode(pinModeSwitch, INPUT_PULLUP);
    delay(100);
    deviceMode = digitalRead(pinModeSwitch) ? MQTT_Mode : TouchMode;

    if (deviceMode == TouchMode)
    {
        itvPrepareToDump = SEC; // cekanje da bi se izbegao burst ispaljivanja granula zbog nestabilnog signala
        delay(itvPrepareToDump);
        solenoidON(true);
        delay(SHORT_PULL);
        solenoidON(false);
        ESP.deepSleep(0);
    }
    if (deviceMode == MQTT_Mode)
    {
        Serial.begin(115200);
        itvPrepareToDump = 5 * SEC; // cekanja da se korisnik spremi za posmatranje sipanja hrane
        led.on();
        ledWiFi.on();
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

        if (msFoodDrop != 0 && millis() >= msFoodDrop)
        {
            solenoidPull(LONG_PULL);
            msFoodDrop = 0;
        }
    }
    delay(100);
}

uint dataToInt(byte *data, uint length)
{
    uint x = 0;
    for (size_t i = 0; i < length; i++)
        x = x * 10 + data[i] - '0';
    return x;
}

void callback(char *topic, byte *data, uint length)
{
    Serial.println(topic);
    for (size_t i = 0; i < length; i++)
        Serial.printf("%c", data[i]);
    Serial.println();

    msFoodDrop = 0;
    bool isDelayedPull = strstr(topic, "time");
    if (isDelayedPull)
    {
        uint itv = dataToInt(data, length) * MIN;
        msFoodDrop = millis() + itv;
    }

    uint ms = 0; // trajanje (u ms) povlacenja poluge solenoida
    if (data[0] == BTN_SHORT)
        ms = SHORT_PULL;
    if (data[0] == BTN_LONG)
        ms = LONG_PULL;
    if (ms > 0 && ms <= MAX_SOLENOID_ON && !isDelayedPull)
        solenoidPull(ms);
}
