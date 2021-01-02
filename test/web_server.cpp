#include <Arduino.h>

#include <WiFiServerBasics.h>
ESP8266WebServer server(80);

const uint pinLed = LED_BUILTIN;
const uint pinSolenoid = D1;
const uint MAX_SOLENOID_ON = 2000;

// test json
void handleTest()
{
    //T Serial.println("/test");
    server.send(200, "application/json", "{ 'app': 'FoodDump' }");
}

// Povlacenje poluge solenoida ms milisekundi. Primer: /pull?ms=100
void handlePull()
{
    uint ms = server.arg("ms").toInt();
    if (ms > 0 && ms < MAX_SOLENOID_ON)
    {
        //T Serial.println("pali");
        digitalWrite(pinLed, false);
        digitalWrite(pinSolenoid, true);
        delay(ms);
        digitalWrite(pinSolenoid, false);
        digitalWrite(pinLed, true);
        //T Serial.println("gasi\n");
    }
    SendEmptyText(server);
}

void setup()
{
    Serial.begin(115200);
    pinMode(pinSolenoid, OUTPUT);
    digitalWrite(pinSolenoid, false);
    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, false); // LED upaljen
    ConnectToWiFi();
    SetupIPAddress(60);
    server.on("/pull", handlePull);
    server.on("/test", handleTest);
    server.begin();
    digitalWrite(pinLed, true); // LED ugasen
}

void loop()
{
    server.handleClient();
    delay(10);
}