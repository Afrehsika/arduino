#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Servo.h>

Servo doorturning;

SoftwareSerial fingerSerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

int id = 1; // ID to store fingerprint

void setup() {
  doorturning.attach(9);
  Serial.begin(9600);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor ready");
  } else {
    Serial.println("Fingerprint sensor NOT found");
    while (1);
  }

  Serial.println("Commands:");
  Serial.println("E = Enroll fingerprint");
  Serial.println("V = Verify fingerprint");
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();

    if (command == 'E') {
      enrollFingerprint();
    } 
    else if (command == 'V') {
      verifyFingerprint();
    }
  }
}

// ================= ENROLL =================
void enrollFingerprint() {
  Serial.println("Enroll started. Place finger...");

  while (finger.getImage() != FINGERPRINT_OK);

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Error processing first image");
    return;
  }

  Serial.println("Remove finger...");
  delay(2000);

  Serial.println("Place same finger again...");
  while (finger.getImage() != FINGERPRINT_OK);

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Error processing second image");
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Fingerprints did not match");
    return;
  }

  if (finger.storeModel(id) == FINGERPRINT_OK) {
    Serial.println("Fingerprint Stored Successfully!");
  } else {
    Serial.println("Error storing fingerprint");
  }
}

// ================= VERIFY =================
void verifyFingerprint() {
  Serial.println("Place finger to verify...");

  if (finger.getImage() != FINGERPRINT_OK) {
    Serial.println("No finger detected");
    return;
  }

  if (finger.image2Tz() != FINGERPRINT_OK) {
    Serial.println("Image conversion failed");
    return;
  }

  if (finger.fingerSearch() != FINGERPRINT_OK) {
    Serial.println("ACCESS DENIED");
    return;
  }

  Serial.print("ACCESS GRANTED - ID: ");
  Serial.println(finger.fingerID);
}