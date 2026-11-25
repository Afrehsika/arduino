// Juan A. Villalpando
// Modified to support 3 servos on pins 9, 10, and 7 + DC motors

#include <Servo.h>
#include <AFMotor.h>

// === Servo Setup ===
Servo servo3;   // Pin 9
Servo servo5;   // Pin 10
Servo servo7;   // Pin 7

const int pwmPin3 = 9;   // Servo 1
const int pwmPin5 = 10;  // Servo 2
const int pwmPin7 = 7;   // Servo 3

char caracter;
String palabra;
String angulo_1;
String angulo_2;
String angulo_3;

int k1, k2;

// === Motor Setup ===
AF_DCMotor motor1(1); // Front-left
AF_DCMotor motor2(2); // Front-right
AF_DCMotor motor3(3); // Back-left
AF_DCMotor motor4(4); // Back-right

void setup() {
  Serial.begin(9600);

  // Servo initialization
  pinMode(pwmPin3, OUTPUT); 
  pinMode(pwmPin5, OUTPUT);
  pinMode(pwmPin7, OUTPUT);
  
  servo3.attach(pwmPin3);
  servo5.attach(pwmPin5);
  servo7.attach(pwmPin7);

  // Motor initialization
  motor1.setSpeed(250);
  motor2.setSpeed(250);
  motor3.setSpeed(250);
  motor4.setSpeed(250);
}

void loop() {
  if (Serial.available()) {
    caracter = Serial.read();  
    palabra += caracter;

    // === If servo data ===
    if (caracter == '*') {
      Serial.println(palabra);
      palabra.remove(palabra.length() - 1); // Remove '*'

      // Parse angles
      k1 = palabra.indexOf(',');
      k2 = palabra.indexOf(',', k1 + 1);

      angulo_1 = palabra.substring(0, k1);
      angulo_2 = palabra.substring(k1 + 1, k2);
      angulo_3 = palabra.substring(k2 + 1);

      // Write angles to servos
      servo3.write(angulo_1.toInt());
      servo5.write(angulo_2.toInt());
      servo7.write(angulo_3.toInt());

      palabra = ""; // Clear
    }

    // === If motor command ===
    else if (palabra.length() == 1 && isMotorCommand(caracter)) {
      handleMotorCommand(caracter);
      palabra = ""; // Clear after single motor command
    }
  }
}

bool isMotorCommand(char c) {
  return (c == 'F' || c == 'B' || c == 'L' || c == 'R' || c == 'S');
}

void handleMotorCommand(char command) {
  switch (command) {
    case 'F': // Forward
       motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor3.run(BACKWARD);
      motor4.run(FORWARD);
      break;
    case 'B': // Backward
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(FORWARD);
      motor4.run(BACKWARD);
      break;
    case 'L': // Turn Left
        motor1.run(BACKWARD);
        motor2.run(FORWARD);
        motor3.run(BACKWARD);
        motor4.run(FORWARD);
      break;
    case 'R': // Turn Right
        motor1.run(FORWARD);
        motor2.run(BACKWARD);
        motor3.run(FORWARD);
        motor4.run(BACKWARD);
      break;
    case 'S': // Stop
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      motor3.run(RELEASE);
      motor4.run(RELEASE);
      break;
  }
}