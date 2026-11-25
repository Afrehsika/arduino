
#include <SPI.h>
#include <Wire.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

RF24 radio(7, 8);

const byte address[6] = "00001";

#define BUTTON_PIN 2
#define JOYSTICK_X A2
#define JOYSTICK_Y A1

#define MIN_THROTTLE 1000
#define MAX_THROTTLE 2000

struct JoystickData {
  uint16_t x;
  uint16_t y;
  uint8_t button;
};

JoystickData data;

bool displayReady = false;

unsigned long previousDisplayUpdate = 0;

void setup() {

  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin();

  // OLED initialization
  displayReady = display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

  if (displayReady) {

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);

    display.println("DRONE CONTROLLER");
    display.println("----------------");
    display.println("INITIALIZING...");

    display.display();
  }

  // Radio initialization
  if (!radio.begin()) {

    Serial.println("NRF NOT DETECTED!");

    while (1);
  }

  radio.openWritingPipe(address);

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(5, 15);

  radio.stopListening();

  Serial.println("TRANSMITTER READY");

  if (displayReady) {

    display.clearDisplay();

    display.setCursor(0, 0);

    display.println("DRONE CONTROLLER");
    display.println("----------------");
    display.println("RADIO: READY");
    display.println("JOYSTICK: READY");

    display.display();
  }

  delay(1000);
}

void loop() {

  // Read joystick
  data.x = analogRead(JOYSTICK_X);
  data.y = analogRead(JOYSTICK_Y);

  // Read button
  data.button = (digitalRead(BUTTON_PIN) == LOW);

  // Send data
  bool success = radio.write(
    &data,
    sizeof(data)
  );

  // Calculate throttle
  int throttle = map(
    data.y,
    0,
    1023,
    MIN_THROTTLE,
    MAX_THROTTLE
  );

  // Serial Monitor
  Serial.print("X: ");
  Serial.print(data.x);

  Serial.print(" | Y: ");
  Serial.print(data.y);

  Serial.print(" | THROTTLE: ");
  Serial.print(throttle);

  Serial.println(
    success ? " | TX OK" : " | TX FAILED"
  );

  // OLED update
  if (displayReady &&
      millis() - previousDisplayUpdate >= 100) {

    previousDisplayUpdate = millis();

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 0);

    display.println("DRONE CONTROLLER");

    display.drawLine(
      0, 11, 127, 11,
      SSD1306_WHITE
    );

    display.setCursor(0, 16);

    display.print("X: ");
    display.print(data.x);

    display.setCursor(68, 16);

    display.print("Y: ");
    display.print(data.y);

    display.setCursor(0, 29);

    display.print("THROTTLE: ");
    display.print(throttle);

    display.setCursor(0, 42);

    display.print("BUTTON: ");

    display.print(
      data.button ? "PRESSED" : "RELEASED"
    );

    display.setCursor(0, 54);

    display.print(
      success ? "RADIO: TX SUCCESS" : "RADIO: NO ACK"
    );

    display.display();
  }

  delay(20);
}
