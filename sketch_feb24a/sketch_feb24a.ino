#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Create software serial on pins 2 (RX) and 3 (TX)
SoftwareSerial mySerial(2, 3); 

// Create fingerprint object using software serial
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int ledPin = 6;
uint8_t id = 1;  // Default ID for enrollment

// Function Prototypes
void enrollFingerprint(uint8_t id);
void verifyFingerprint();
void showMenu();

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(9600);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor ready!");
  } else {
    Serial.println("Sensor not found!");
    while (1);
  }

  showMenu();
}

void loop() {

  if (Serial.available()) {

    char choice = Serial.read();

    if (choice == 'E' || choice == 'e') {
      enrollFingerprint(id);
      showMenu();
    }

    else if (choice == 'V' || choice == 'v') {
      verifyFingerprint();
      showMenu();
    }
  }
}

void showMenu() {
  Serial.println("\n====== MENU ======");
  Serial.println("E - Enroll Finger");
  Serial.println("V - Verify Finger (Turn ON LED)");
  Serial.println("==================");
}

void enrollFingerprint(uint8_t id) {

  int p = -1;

  Serial.println("Place finger...");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Image conversion failed");
    return;
  }

  Serial.println("Remove finger...");
  delay(2000);

  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("Place same finger again...");

  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Second conversion failed");
    return;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprints did not match");
    return;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint Enrolled Successfully!");
  } else {
    Serial.println("Error storing fingerprint");
  }

  while (finger.getImage() != FINGERPRINT_NOFINGER);
}

void verifyFingerprint() {

  Serial.println("Place finger to verify...");

  int p = -1;

  // Wait until finger is placed
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Conversion failed");
    return;
  }

  p = finger.fingerSearch();

  if (p == FINGERPRINT_OK) {
    Serial.println("Access Granted!");
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
  } else {
    Serial.println("Access Denied!");
  }

  while (finger.getImage() != FINGERPRINT_NOFINGER);
}