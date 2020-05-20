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
const int pinSolenoid = D1;
const uint RECONNECT = 5 * 60;                       // Trajanje (u sec) cekanja na ponovni rekonekt na MQTT server.
const uint PREPARE_TO_DUMP = (TEST ? 2 : 10) * 1000; // Trajanje (u ms) cekanja da se korisnik spremi za posmatranje sipanja hrane.
const int SHORT_PULL = 50;                          // Kratko aktiviranje solenoida (ispustanje hrane).
const int LONG_PULL = 250;                          // Dugo aktiviranje solenoida (ispustanje hrane).
const int MAX_SOLENOID_ON = 2000;                    // Maksimalno trajanje (u ms) aktiviranog/povucenog solenoida.

// Moguce vrednosti za dugmice u AIO FoodDump dashboard-u.
enum ButtonStatus
{
    BTN_RELEASE = '0',
    BTN_SHORT = '1',
    BTN_LONG = '2'
};

void statusLed(bool on) { digitalWrite(LED_BUILTIN, !on); }

void setup()
{
    Serial.begin(115200);
    delay(100);
    pinMode(pinSolenoid, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    statusLed(true);
    ConnectToWiFi();
    Serial.print("\nWiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
    mqtt.setServer(AIO_SERVER, AIO_SERVERPORT);
    mqtt.setCallback(callback);

    Serial.println("MQTT connecting...");
    if (mqtt.connect("", AIO_USER, AIO_KEY))
    {
        Serial.println("Connected");
        mqtt.subscribe(AIO_USER AIO_FEEDS FEED_SHORT_DUMP, 0);
        mqtt.subscribe(AIO_USER AIO_FEEDS FEED_LONG_DUMP, 0);
        statusLed(false);
    }
    else
    {
        Serial.print("Failed. MQTT state: ");
        Serial.print(mqtt.state());
        for (uint i = 0; i < RECONNECT; i++)
        {
            statusLed(i % 2);
            delay(1000);
        }
        ESP.reset();
    }
}

void loop()
{
    mqtt.loop();
    delay(100);
}

void callback(char *topic, byte *data, uint length)
{
    Serial.println(topic);
    int ms = -1; // trajanje (u ms) povlacenja poluge solenoida
    if (data[0] == BTN_SHORT)
        ms = SHORT_PULL;
    if (data[0] == BTN_LONG)
        ms = LONG_PULL;
    if (ms > 0 && ms <= MAX_SOLENOID_ON)
    {
        // cekanje da se korisnik pripremi za gledanje prosipanja hrane
        statusLed(true);
        delay(PREPARE_TO_DUMP);
        statusLed(false);
        delay(1000);
        // prosipanje hrane omalenom kunikulusu
        statusLed(true);
        digitalWrite(pinSolenoid, true);
        delay(ms);
        digitalWrite(pinSolenoid, false);
        statusLed(false);
    }
}