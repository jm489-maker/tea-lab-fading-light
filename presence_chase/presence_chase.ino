// Presence-triggered NeoPixel chase
//
// Behaviour (from Lucia's notes):
//   1. Nobody there          -> strip is dark.
//   2. Person sensed         -> rainbow chase fills the strip, pixel by pixel, to LED_COUNT (80).
//   3. Strip fully lit       -> holds for 10 seconds, ignoring the sensor (no flicker / no restart).
//   4. After 10 seconds      -> strip turns off and the piece re-arms for the next person.
//
// Debounce: a person must be seen on DEBOUNCE_READINGS consecutive sensor readings
// before the chase starts, so a single noisy reading can't trigger it.
//
// Hardware notes: 1000 uF cap across the strip's +/-, 300-500 ohm resistor on DATA-IN,
// and a level shifter if the board is 3.3V and the strip is 5V.

#include <Adafruit_NeoPixel.h>
#include "Adafruit_VL53L0X.h"

// ---------------- Settings you can tune ----------------
#define LED_PIN     6
#define LED_COUNT   80     // number of pixels in the chase
#define BRIGHTNESS  60     // 0-255. Lower if the strip browns out / resets at full white.

const uint16_t      TRIGGER_DISTANCE_MM = 800;    // person counts as "present" closer than this
const uint8_t       DEBOUNCE_READINGS   = 3;      // consecutive readings needed to trigger / to count as gone
const unsigned long SENSOR_INTERVAL_MS  = 50;     // how often to read the sensor while waiting
const unsigned long CHASE_STEP_MS       = 25;     // time between pixels (80 x 25 ms = 2 s chase)
const unsigned long HOLD_MS             = 10000;  // stay fully lit this long

// false: after the 10 s hold, a person still standing there will trigger a new chase.
// true:  after the 10 s hold, the area must be empty before it can trigger again.
const bool WAIT_FOR_EMPTY_BEFORE_REARM = true;
// --------------------------------------------------------

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

enum State { IDLE, CHASING, HOLDING };
State state = IDLE;

unsigned long lastSensorRead = 0;
unsigned long lastChaseStep  = 0;
unsigned long holdStart      = 0;
uint16_t      chaseIndex     = 0;

uint8_t presentCount = 0;   // consecutive "someone is there" readings
uint8_t absentCount  = 0;   // consecutive "nobody there" readings
bool    armed        = true;

// Reads the sensor once and updates the debounce counters.
void updatePresence() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  bool valid = (measure.RangeStatus != 4);  // status 4 = out of range / bad reading
  bool close = valid && measure.RangeMilliMeter > 0 &&
               measure.RangeMilliMeter < TRIGGER_DISTANCE_MM;

  if (valid) {
    Serial.print(F("Distance (mm): "));
    Serial.print(measure.RangeMilliMeter);
  } else {
    Serial.print(F("out of range"));
  }
  Serial.println(close ? F("  -> present") : F(""));

  if (close) {
    if (presentCount < 255) presentCount++;
    absentCount = 0;
  } else {
    if (absentCount < 255) absentCount++;
    presentCount = 0;
  }

  if (!armed && absentCount >= DEBOUNCE_READINGS) {
    armed = true;
    Serial.println(F("Area empty - re-armed"));
  }
}

void setup() {
  Serial.begin(115200);
  // Wait at most 2 s for the Serial Monitor. A bare `while (!Serial)` hangs forever
  // on USB-native boards when no computer is attached (e.g. in the installation).
  unsigned long start = millis();
  while (!Serial && millis() - start < 2000) {}

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X - check wiring (SDA/SCL/VIN/GND)"));
    // Visible error: first pixel red, so you can tell without a computer.
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    while (1) {}
  }

  Serial.println(F("Ready - waiting for someone"));
}

void loop() {
  unsigned long now = millis();

  switch (state) {

    case IDLE:
      // Only read the sensor while waiting (and during the hold if we need to see the person leave).
      if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
        lastSensorRead = now;
        updatePresence();
      }
      if (armed && presentCount >= DEBOUNCE_READINGS) {
        Serial.println(F("Person detected - starting chase"));
        strip.clear();
        strip.show();
        chaseIndex    = 0;
        lastChaseStep = now;
        state         = CHASING;
      }
      break;

    case CHASING:
      // Non-blocking chase: light one more pixel each CHASE_STEP_MS.
      if (now - lastChaseStep >= CHASE_STEP_MS) {
        lastChaseStep = now;
        uint16_t hue = (uint32_t)chaseIndex * 65536L / LED_COUNT;
        strip.setPixelColor(chaseIndex, strip.gamma32(strip.ColorHSV(hue)));
        strip.show();
        chaseIndex++;
        if (chaseIndex >= LED_COUNT) {
          Serial.println(F("Strip full - holding 10 s"));
          holdStart = now;
          state     = HOLDING;
        }
      }
      break;

    case HOLDING:
      // Sensor is ignored here, so nobody walking in/out can flicker or restart it.
      if (now - holdStart >= HOLD_MS) {
        Serial.println(F("Hold finished - resetting"));
        strip.clear();
        strip.show();
        presentCount = 0;  // need a fresh, debounced detection to trigger again
        absentCount  = 0;
        armed        = !WAIT_FOR_EMPTY_BEFORE_REARM;
        state        = IDLE;
      }
      break;
  }
}
