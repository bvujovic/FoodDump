#include <Arduino.h>

const int pinSol = D1;  // Ovde se povremeno pusta kratak HIGH signal.
const int pinHigh = D3; // Na ovom pinu je vazda HIGH.
const int pinLed = LED_BUILTIN;

void statusLedON(bool on) { digitalWrite(pinLed, !on); }

void solenoidON(bool on)
{
    digitalWrite(pinSol, on);
    statusLedON(on);
}

void setup()
{
    pinMode(pinSol, OUTPUT);
    pinMode(pinLed, OUTPUT);
    pinMode(pinHigh, OUTPUT);
    digitalWrite(pinHigh, true);
}

void loop()
{
    solenoidON(false);
    delay(5000);
    solenoidON(true);
    delay(100);
}