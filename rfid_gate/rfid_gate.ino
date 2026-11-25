#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 6

MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;

// Authorized card UID
byte allowedUID[] = {0xB7, 0xFB, 0x43, 0x02};

void setup() {
  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  doorServo.attach(SERVO_PIN);
  doorServo.write(0);      // Closed position

  Serial.println("RFID Door Lock Ready");
  Serial.println("Scan your card...");
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.print("Card UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10)
      Serial.print("0");

    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();

  bool access = true;

  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != allowedUID[i]) {
      access = false;
    }
  }

  if (access) {
    Serial.println("Access Granted");

    // Open servo
    doorServo.write(90);
    delay(3000);

    // Close servo
    doorServo.write(0);

    Serial.println("Door Closed");
  }
  else {
    Serial.println("Access Denied");
  }

  Serial.println("---------------------");

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(500);
}