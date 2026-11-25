#include <AFMotor.h>
#include <Servo.h>

// ---------------- MOTORS ----------------
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

// ---------------- SERVO ----------------
Servo nozzleServo;

// ---------------- FLAME SENSORS ----------------
#define FLAME_LEFT   A1
#define FLAME_CENTER A3
#define FLAME_RIGHT  A0

// ---------------- RELAY ----------------
#define RELAY_PIN 9

// 🔥 Threshold
int flameThreshold = 500;

// 🎲 Random movement timing
unsigned long lastMoveChange = 0;
int moveState = 0;

void setup() {
  Serial.begin(9600);

  nozzleServo.attach(10); 
  nozzleServo.write(90);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  randomSeed(analogRead(A5)); // random generator

  Serial.println("🔥 Fire Robot + Random Search");
}

void loop() {

  int leftVal   = analogRead(FLAME_LEFT);
  int centerVal = analogRead(FLAME_CENTER);
  int rightVal  = analogRead(FLAME_RIGHT);

  Serial.println("------ SENSOR READINGS ------");
  Serial.print("LEFT   : "); Serial.println(leftVal);
  Serial.print("CENTER : "); Serial.println(centerVal);
  Serial.print("RIGHT  : "); Serial.println(rightVal);

  bool leftFire   = leftVal   < flameThreshold;
  bool centerFire = centerVal < flameThreshold;
  bool rightFire  = rightVal  < flameThreshold;

  Serial.print("Detected -> ");
  Serial.print("L:"); Serial.print(leftFire);
  Serial.print(" C:"); Serial.print(centerFire);
  Serial.print(" R:"); Serial.println(rightFire);

  // 🔥 FIRE DETECTED
  if (leftFire || centerFire || rightFire) {

    Serial.println("🔥 FLAME DETECTED!");

    stopAll();

    if (leftFire) {
      Serial.println("➡️ Flame LEFT");
      nozzleServo.write(140);
    }
    else if (rightFire) {
      Serial.println("➡️ Flame RIGHT");
      nozzleServo.write(40);
    }
    else {
      Serial.println("➡️ Flame CENTER");
      nozzleServo.write(90);
    }

    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("💧 Pump ON");

  } 

  // 🔄 NO FLAME → RANDOM MOVEMENT
  else {

    Serial.println("❌ No flame → Random search");
    digitalWrite(RELAY_PIN, LOW);

    // Change movement every 2–4 seconds
    if (millis() - lastMoveChange > 2000) {
      moveState = random(0, 4); // 0–3
      lastMoveChange = millis();

      Serial.print("🎲 New Move: ");
      Serial.println(moveState);
    }

    setSpeedAll(180);

    switch (moveState) {

      case 0: // Forward
        Serial.println("🚗 Forward");
        motor1.run(FORWARD);
        motor2.run(FORWARD);
        motor3.run(FORWARD);
        motor4.run(FORWARD);
        break;

      case 1: // Left
        Serial.println("↩️ Left");
        motor1.run(BACKWARD);
        motor3.run(BACKWARD);
        motor2.run(FORWARD);
        motor4.run(FORWARD);
        break;

      case 2: // Right
        Serial.println("↪️ Right");
        motor1.run(FORWARD);
        motor3.run(FORWARD);
        motor2.run(BACKWARD);
        motor4.run(BACKWARD);
        break;

      case 3: // Stop
        Serial.println("⏹ Stop");
        stopAll();
        break;
    }
  }

  Serial.println("-----------------------------\n");
  delay(300);
}

// ---------------- HELPERS ----------------
void setSpeedAll(int speed) {
  motor1.setSpeed(speed);
  motor2.setSpeed(speed);
  motor3.setSpeed(speed);
  motor4.setSpeed(speed);
}

void stopAll() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}