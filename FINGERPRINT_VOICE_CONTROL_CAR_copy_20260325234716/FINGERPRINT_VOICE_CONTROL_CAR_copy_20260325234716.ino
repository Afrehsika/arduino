#include <AFMotor.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h> // Required by some AFMotor versions

// ---------------- VOICE ----------------
String voice = "";

// ---------------- MOTOR ----------------
AF_DCMotor motor1(1, MOTOR12_1KHZ);
AF_DCMotor motor2(2, MOTOR12_1KHZ);
Servo myServo;

// Motor Shield object
AF_DCMotor motor3(3); // placeholder if needed
AF_DCMotor motor4(4); // placeholder if needed

// ---------------- FINGERPRINT ----------------
SoftwareSerial fingerSerial(3, 2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// ---------------- GLOBAL ----------------
bool engineOn = false;
bool readingVoice = false;
bool enrolling = false;
int enrollStep = 0; // 0 = not enrolling, 1 = first scan, 2 = second scan

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  finger.begin(57600);

  myServo.attach(10);
  myServo.write(90);

  Serial.println("Send E to enroll, V to verify fingerprint");
  Serial.println("Then use voice commands");

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor ready");
  } else {
    Serial.println("Fingerprint sensor NOT found");
    while (1);
  }

  // Initialize motor shield
  // If using AFMotor v1: no need for AFMS.begin()
  // For some versions of the library:
  // AFMS.begin();
}

// ---------------- LOOP ----------------
void loop() {
  // -------- SERIAL INPUT --------
  while (Serial.available()) {
    char c = Serial.read();

    if (!readingVoice && !enrolling) {
      if (c == 'E') {
        enrolling = true;
        enrollStep = 1;
        Serial.println("Enroll started. Place finger for first scan.");
        continue;
      }
      if (c == 'V') {
        Serial.println("Place finger to verify...");
        if (verifyFinger()) {
          Serial.println("ACCESS GRANTED - VOICE ENABLED");
          engineOn = true;
        } else {
          Serial.println("ACCESS DENIED");
        }
        continue;
      }
      if (c == 'X') {
        Serial.println("SYSTEM LOCKED");
        engineOn = false;
        stop_car();
        continue;
      }
      if (c == '*') {
        readingVoice = true;
        voice = "*";
        continue;
      }
    }

    // -------- VOICE COMMAND INPUT --------
    if (readingVoice) {
      voice += c;
      if (c == '#') {
        processVoice();
        readingVoice = false;
      }
    }
  }

  // -------- NON-BLOCKING ENROLLMENT --------
  if (enrolling) {
    int p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      finger.image2Tz(enrollStep);

      if (enrollStep == 1) {
        Serial.println("First scan done. Remove finger and place same finger again.");
        enrollStep = 2;
      } else if (enrollStep == 2) {
        if (finger.createModel() == FINGERPRINT_OK && finger.storeModel(1) == FINGERPRINT_OK) {
          Serial.println("Fingerprint Stored Successfully!");
        } else {
          Serial.println("Enroll Failed!");
        }
        enrolling = false;
        enrollStep = 0;
        Serial.println("Enrollment finished. You can now verify (V) or use commands.");
      }
      delay(1000); // small delay to allow finger removal
    }
  }
}

// ---------------- PROCESS VOICE ----------------
void processVoice() {
  voice.trim(); // remove any \r, \n, or spaces
  if (!engineOn) {
    Serial.println("Verify fingerprint first!");
    return;
  }

  Serial.print("Voice Command: ");
  Serial.println(voice);

  if (voice == "*go ahead#") forward_car();
  else if (voice == "*go back#") back_car();
  else if (voice == "*right#") right_car();
  else if (voice == "*left#") left_car();
  else if (voice == "*stop#") stop_car();
  else Serial.println("Unknown command");
}

// ---------------- MOTOR FUNCTIONS ----------------
void forward_car() {
  motor1.setSpeed(200); motor1.run(FORWARD);
  motor2.setSpeed(200); motor2.run(FORWARD);
  Serial.println("forward_car() called");
}

void back_car() {
  motor1.setSpeed(200); motor1.run(BACKWARD);
  motor2.setSpeed(200); motor2.run(BACKWARD);
  Serial.println("back_car() called");
}

void right_car() {
  myServo.write(0); delay(500); myServo.write(90);
  motor1.setSpeed(180); motor1.run(FORWARD);
  motor2.setSpeed(180); motor2.run(BACKWARD);
  Serial.println("right_car() called");
}

void left_car() {
  myServo.write(180); delay(500); myServo.write(90);
  motor1.setSpeed(180); motor1.run(BACKWARD);
  motor2.setSpeed(180); motor2.run(FORWARD);
  Serial.println("left_car() called");
}

void stop_car() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  Serial.println("stop_car() called");
}

// ---------------- VERIFY FUNCTION ----------------
bool verifyFinger() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) return false;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return false;

  p = finger.fingerFastSearch();
  return (p == FINGERPRINT_OK);
}