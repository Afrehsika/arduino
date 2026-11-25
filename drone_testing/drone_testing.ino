/*
 * OLED FPV Drone Simulator using MPU6050
 * Dynamic Horizon + Drone HUD
 *
 * License: MIT
 */

#include <Wire.h>
#include <MPU6050_light.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// =====================================================
// OLED CONFIG
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// =====================================================
// MPU6050
// =====================================================

MPU6050 mpu(Wire);

unsigned long timer = 0;

// Drone stays centered
const int droneX = 64;
const int droneY = 32;

// =====================================================
// DRONE DRAWING FUNCTION
// =====================================================

void drawDrone(int x, int y, float rollAngleDegrees) {

  float rad = rollAngleDegrees * PI / 180.0;

  float cosA = cos(rad);
  float sinA = sin(rad);

  int armLength = 14;
  int bodyRadius = 4;
  int propRadius = 5;

  // -------------------------------------------------
  // Calculate motor positions
  // -------------------------------------------------

  // Front Left
  int x1 = x + (-armLength * cosA - armLength * sinA);
  int y1 = y + (-armLength * sinA + armLength * cosA);

  // Front Right
  int x2 = x + ( armLength * cosA - armLength * sinA);
  int y2 = y + ( armLength * sinA + armLength * cosA);

  // Back Left
  int x3 = x + (-armLength * cosA + armLength * sinA);
  int y3 = y + (-armLength * sinA - armLength * cosA);

  // Back Right
  int x4 = x + ( armLength * cosA + armLength * sinA);
  int y4 = y + ( armLength * sinA - armLength * cosA);

  // -------------------------------------------------
  // Draw Arms
  // -------------------------------------------------

  display.drawLine(x, y, x1, y1, SSD1306_WHITE);
  display.drawLine(x, y, x2, y2, SSD1306_WHITE);
  display.drawLine(x, y, x3, y3, SSD1306_WHITE);
  display.drawLine(x, y, x4, y4, SSD1306_WHITE);

  // -------------------------------------------------
  // Draw Propellers
  // -------------------------------------------------

  display.drawCircle(x1, y1, propRadius, SSD1306_WHITE);
  display.drawCircle(x2, y2, propRadius, SSD1306_WHITE);
  display.drawCircle(x3, y3, propRadius, SSD1306_WHITE);
  display.drawCircle(x4, y4, propRadius, SSD1306_WHITE);

  // -------------------------------------------------
  // Drone Body
  // -------------------------------------------------

  display.fillCircle(x, y, bodyRadius, SSD1306_WHITE);

  // -------------------------------------------------
  // Camera Direction Indicator
  // -------------------------------------------------

  int camX = x + sinA * 10;
  int camY = y - cosA * 10;

  display.drawLine(x, y, camX, camY, SSD1306_WHITE);

  // -------------------------------------------------
  // Animated Propellers
  // -------------------------------------------------

  int spin = millis() / 50;

  display.drawLine(
    x1 - spin % 4,
    y1,
    x1 + spin % 4,
    y1,
    SSD1306_WHITE
  );

  display.drawLine(
    x2,
    y2 - spin % 4,
    x2,
    y2 + spin % 4,
    SSD1306_WHITE
  );

  display.drawLine(
    x3 - spin % 4,
    y3,
    x3 + spin % 4,
    y3,
    SSD1306_WHITE
  );

  display.drawLine(
    x4,
    y4 - spin % 4,
    x4,
    y4 + spin % 4,
    SSD1306_WHITE
  );
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  Wire.begin();

  // OLED INIT
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println(F("OLED not found"));

    while(true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println("DRONE STARTING");
  display.display();

  // MPU INIT
  byte status = mpu.begin();

  while(status != 0) {

    display.clearDisplay();

    display.setCursor(10, 20);
    display.println("MPU6050 ERROR");

    display.display();
  }

  // Calibration
  display.clearDisplay();

  display.setCursor(15, 20);
  display.println("CALIBRATING...");

  display.display();

  delay(1000);

  mpu.calcOffsets();

  display.clearDisplay();

  display.setCursor(40, 20);
  display.println("READY!");

  display.display();

  delay(1000);
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  mpu.update();

  if((millis() - timer) > 15) {

    // ---------------------------------------------
    // Read MPU6050 Angles
    // ---------------------------------------------

    float rollAngle  = mpu.getAngleX();
    float pitchAngle = mpu.getAngleY();

    // Limit values
    rollAngle  = constrain(rollAngle, -45, 45);
    pitchAngle = constrain(pitchAngle, -30, 30);

    // ---------------------------------------------
    // Horizon Position
    // ---------------------------------------------

    int horizonY = map(pitchAngle, -30, 30, 12, 52);

    // Horizon bank effect
    int offset = rollAngle * 0.7;

    // ---------------------------------------------
    // Clear OLED
    // ---------------------------------------------

    display.clearDisplay();

    // =============================================
    // ARTIFICIAL HORIZON
    // =============================================

    display.drawLine(
      0,
      horizonY - offset,
      127,
      horizonY + offset,
      SSD1306_WHITE
    );

    // Horizon markers
    display.drawLine(
      32,
      horizonY - offset,
      32,
      horizonY - offset + 4,
      SSD1306_WHITE
    );

    display.drawLine(
      96,
      horizonY + offset,
      96,
      horizonY + offset + 4,
      SSD1306_WHITE
    );

    // =============================================
    // CENTER TARGET HUD
    // =============================================

    display.drawCircle(droneX, droneY, 20, SSD1306_WHITE);

    display.drawLine(droneX - 24, droneY, droneX - 14, droneY, SSD1306_WHITE);
    display.drawLine(droneX + 14, droneY, droneX + 24, droneY, SSD1306_WHITE);

    display.drawLine(droneX, droneY - 24, droneX, droneY - 14, SSD1306_WHITE);
    display.drawLine(droneX, droneY + 14, droneX, droneY + 24, SSD1306_WHITE);

    // =============================================
    // DRAW DRONE
    // =============================================

    drawDrone(droneX, droneY, rollAngle);

    // =============================================
    // FLIGHT DATA
    // =============================================

    display.setTextSize(1);

    // Roll
    display.setCursor(0, 0);
    display.print("R:");
    display.print((int)rollAngle);
    display.print((char)247);

    // Pitch
    display.setCursor(84, 0);
    display.print("P:");
    display.print((int)pitchAngle);
    display.print((char)247);

    // Fake battery
    display.drawRect(108, 56, 16, 6, SSD1306_WHITE);
    display.fillRect(124, 58, 2, 2, SSD1306_WHITE);
    display.fillRect(110, 58, 10, 2, SSD1306_WHITE);

    // Radar pulse
    int pulse = (millis() / 100) % 12;

    display.drawCircle(droneX, droneY, pulse, SSD1306_WHITE);

    // =============================================
    // DISPLAY OLED
    // =============================================

    display.display();

    timer = millis();
  }
}