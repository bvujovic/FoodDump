//* Resetovanje ESPa ide automatski na svakih 10sec ili sa ESP.deepSleep(0) se aparat ne resetuje automatski.
//* Dogadjaj koji moze resetovati ESP je PIR signal (spustanje sa 1 na 0) ili neki kontakt (Vibration Sensor npr).

#include <Arduino.h>

const int pinLed = LED_BUILTIN;
const int pinSensor = D2;

// enum DeviceMode
// {
//     SimpleMode,
//     MQTT_Mode
// };

void blink(ulong itv = 100)
{
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, true); // gasenje LEDa posle automatskog blinka (pri budjenju ESPa)
  delay(itv);
  digitalWrite(pinLed, false);
  delay(itv);
  digitalWrite(pinLed, true);
}

void setup()
{
  pinMode(pinSensor, INPUT);
  pinMode(D0, INPUT); //* ovo mozda ne mora
  if (digitalRead(pinSensor))
    // ESP jednom blinkne kad se probudi i onda ova funkcija jos jednom blinkne ako je 1 na senzoru
    blink();
  ESP.deepSleep(10e6, RF_DISABLED); // budjenje i provera senzora je na 10sec
}

void loop()
{
  delay(100); // ovaj kôd se ne izvrsava - sve je u setup-u
}
