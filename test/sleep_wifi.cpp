//* Ovde testiram prebacivanje iz touch (sleep) u mqtt server mod i nazad.
//* RF_DISABLED u ESP.deepSleep() pozivu je bilo bezveze - ne smanjuje bitno potrosnju struje,
//* a smeta pri prelasku na mqtt server (WiFi) mod.

#include <Arduino.h>
#include <WiFiServerBasics.h>

#include "EasyINI.h"
EasyINI ei("/config.ini");

#include "Blinky.h"
Blinky led(LED_BUILTIN, false);

// void statusLedON(bool on) { digitalWrite(LED_BUILTIN, !on); }
// void blinkee()
// {
//     for (uint i = 0; i < 10; i++)
//     {
//         statusLedON(i % 2);
//         delay(1000);
//     }
// }

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    delay(100);

    ei.open(FMOD_READ);
    int count = ei.getInt("count", 0);
    ei.close();
    ei.open(FMOD_WRITE);
    ei.setInt("count", count + 1);
    ei.close();

    Serial.begin(115200);
    Serial.println();
    Serial.println(ESP.getResetReason());
    Serial.println(count);

    if (count % 3 == 0)
    {
        led.blink(3);
        Serial.println("deep sleep");
        ESP.deepSleep(0); // 1.6mA
        // ESP.deepSleep(0, RF_DISABLED);
    }
    else
    {
        led.blink(2);
        WiFi.mode(WIFI_STA);
        // ESP treba resetovati posle promene WiFi mode-a jer ovaj nece odmah da funkcionise
        if (!ESP.getResetReason().startsWith("Software")) 
            ESP.reset(); // Software reset
        ConnectToWiFi();
        Serial.print("WiFi connected, IP address: ");
        Serial.println(WiFi.localIP());
    }
}

void loop()
{
    delay(100);
}
