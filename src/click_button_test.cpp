// #include <Arduino.h>
// #include "ClickButton.h"

// const int pinBtn = D4;
// ClickButton btn(pinBtn, LOW, CLICKBTN_PULLUP);

// int shortClicks = 0, longClicks = 0;
// ulong lastDisplay;

// void setup()
// {
//     pinMode(LED_BUILTIN, OUTPUT);
//     digitalWrite(LED_BUILTIN, true);
//     Serial.begin(115200);
//     Serial.println();
//     lastDisplay = millis();
//     btn.debounceTime = 50;    // Debounce timer in ms
//     btn.multiclickTime = 500; // Time limit for multi clicks
//     btn.longClickTime = 1000;   // time until "held-down clicks" register
// }

// void loop()
// {
//     btn.Update();

//     if (btn.clicks == 1)
//         shortClicks++;
//     if (btn.clicks == -1)
//         longClicks++;
//     if (btn.clicks > 1)
//         Serial.println(btn.clicks);

//     if (millis() > lastDisplay + 1000)
//     {
//         lastDisplay = millis();
//         Serial.print(shortClicks);
//         Serial.print('\t');
//         Serial.println(longClicks);
//     }

//     delay(10);
// }