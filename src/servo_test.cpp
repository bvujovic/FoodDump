// // https://diyi0t.com/servo-motor-tutorial-for-arduino-and-esp8266/

// #include <Servo.h>

// Servo myservo; //initialize a servo object
// int angle = 0;

// void setup()
// {
//   myservo.attach(D3); // connect the servo at pin9
// }

// void loop()
// { // move from 0 to 180 degrees with a positive angle of 1
//   for (angle = 0; angle < 180; angle += 1)
//   {
//     myservo.write(angle);
//     delay(15);
//   }
//   delay(1000); // move from 180 to 0 degrees with a negative angle of 5
//   for (angle = 180; angle >= 1; angle -= 1)
//   {
//     myservo.write(angle);
//     delay(5);
//   }

//   delay(1000);
// }