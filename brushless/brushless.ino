#include <Servo.h>

Servo esc;

void setup() {
  esc.attach(9);

  // Stop
  esc.writeMicroseconds(1000);
  delay(5000);
}

void loop() {
  // Forward
  esc.writeMicroseconds(2000);
 



}