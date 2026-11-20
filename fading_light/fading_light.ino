// Tea Lab: Fading Light
//
// The beaker glows bright when nobody is near.
// As someone walks closer, the light flickers and fades.
// When they are very close, the light glitches.
// When they step away, the light slowly comes back.
//
// Hardware: Arduino Nano 33 IoT, VL53L0X distance sensor (SDA = A4, SCL = A5),
// WS2812E LED strip on pin 6 through a 220 ohm resistor, 5V adapter for the strip.

#include <Adafruit_NeoPixel.h>
#include "Adafruit_VL53L0X.h"

// ---------------- Settings you can tune ----------------
#define LED_PIN     6
#define LED_COUNT   80

const uint8_t MAX_BRIGHTNESS = 80;      // brightest the sign ever gets (0-255)

// Base colour of the glow (warm tea amber). Change to taste.
const uint8_t BASE_R = 255;
const uint8_t BASE_G = 140;
const uint8_t BASE_B = 40;

const int   FAR_MM       = 1000;  // farther than this = nobody near, full glow
const int   GLITCH_MM    = 300;   // closer than this = glitch
const int   MIN_VALID_MM = 30;    // readings under this are sensor noise, ignored

const float SMOOTHING      = 0.3;   // 0-1: how quickly the distance reading follows the person
const float FADE_SPEED     = 0.08;  // how fast the light drops as someone approaches (per frame)
const float RECOVER_SPEED  = 0.01;  // how fast it comes back when they leave (per frame, ~4 s)
const float LOWEST_LEVEL   = 0.10;  // dimmest the glow fades to (0-1)
// --------------------------------------------------------

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

float smoothedMM = FAR_MM;   // filtered distance to the nearest person
float level      = 1.0;      // how much light is showing right now (1 = full glow)
unsigned long lastPrint = 0;

// Reads the sensor and updates smoothedMM.
void readDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  float raw;
  if (measure.RangeStatus == 4 || measure.RangeMilliMeter > 2000) {
    raw = FAR_MM + 200;                       // out of range = nobody there
  } else if (measure.RangeMilliMeter < MIN_VALID_MM) {
    return;                                   // noise: keep the last value
  } else {
    raw = measure.RangeMilliMeter;
  }

  smoothedMM += (raw - smoothedMM) * SMOOTHING;
}

// Moves `level` toward where it should be: fast going down, slow coming back.
void updateLevel() {
  // 0 when far away, 1 at the edge of the glitch zone
  float closeness = (FAR_MM - smoothedMM) / (float)(FAR_MM - GLITCH_MM);
  closeness = constrain(closeness, 0.0, 1.0);

  float target = 1.0 - closeness * (1.0 - LOWEST_LEVEL);

  if (target < level) {
    level -= min(FADE_SPEED, level - target);
  } else {
    level += min(RECOVER_SPEED, target - level);
  }
}

// Scales the base colour by brightness b (0-1) and applies gamma for smooth fades.
uint32_t glowColor(float b) {
  float s = b * MAX_BRIGHTNESS / 255.0;
  return strip.gamma32(strip.Color(BASE_R * s, BASE_G * s, BASE_B * s));
}

uint32_t randomSpark() {
  float s = MAX_BRIGHTNESS / 255.0;
  return strip.Color(random(256) * s, random(256) * s, random(256) * s);
}

void render() {
  bool  glitching = smoothedMM < GLITCH_MM;
  float disturb   = 1.0 - level;      // 0 = calm, 1 = most disturbed

  // Flicker: the more disturbed, the more pixels randomly dip.
  for (int i = 0; i < LED_COUNT; i++) {
    float b = level;
    if (random(1000) < disturb * 400) {
      b *= random(0, 100) / 100.0;
    }
    strip.setPixelColor(i, glowColor(b));
  }

  if (glitching) {
    // Scattered dead pixels and colour sparks
    for (int i = 0; i < LED_COUNT; i++) {
      int roll = random(100);
      if (roll < 6)       strip.setPixelColor(i, randomSpark());
      else if (roll < 30) strip.setPixelColor(i, 0);
    }
    // A torn block of one wrong colour
    if (random(100) < 25) {
      int start = random(LED_COUNT);
      int len   = random(5, 16);
      uint32_t c = randomSpark();
      for (int i = start; i < start + len && i < LED_COUNT; i++) {
        strip.setPixelColor(i, c);
      }
    }
    // Occasional full blackout frame
    if (random(100) < 6) strip.clear();
  }

  strip.show();
}

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && millis() - start < 2000) {}   // never wait forever

  randomSeed(analogRead(A0));

  strip.begin();
  strip.clear();
  strip.show();

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X - check wiring (SDA/SCL/VIN/GND)"));
    strip.setPixelColor(0, strip.Color(255, 0, 0));   // visible error
    strip.show();
    while (1) {}
  }

  Serial.println(F("Fading Light ready"));
}

void loop() {
  readDistance();   // about 30 ms per reading, so the loop runs ~25 times a second
  updateLevel();
  render();

  if (millis() - lastPrint >= 250) {
    lastPrint = millis();
    Serial.print(F("distance mm: "));
    Serial.print((int)smoothedMM);
    Serial.print(F("  level: "));
    Serial.print(level, 2);
    Serial.println(smoothedMM < GLITCH_MM ? F("  GLITCH") : F(""));
  }
}
