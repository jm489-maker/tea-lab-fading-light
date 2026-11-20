# Tea Lab: Fading Light

An interactive LED sign shaped like a beaker. A distance sensor changes the light depending on how close people are:

| Distance | What the beaker does |
|---|---|
| Nobody within 1 m | Bright, steady warm glow |
| 1 m to 30 cm | Fades the closer you get, with more and more flicker |
| Under 30 cm | Glitches: dead pixels, colour sparks, torn blocks, brief blackouts |
| Stepping away | Slowly comes back, about 4 seconds from dim to full |

The nearer you get, the less light you get. Final project for Useless Machines and Intro to Python. The behaviour runs as an Arduino sketch for now and will later be rewritten in Python.

## Files

| File | What it is |
|---|---|
| `fading_light/fading_light.ino` | The Fading Light sketch (current version) |
| `presence_chase/presence_chase.ino` | Earlier version: a rainbow chase when someone comes near, held for 10 seconds |
| `wiring-record.md` | Every wire, breadboard row and pin, checked one by one |

## Hardware

- Arduino Nano 33 IoT
- Adafruit VL53L0X time-of-flight distance sensor (I2C: SDA on A4, SCL on A5)
- WS2812E LED strip, 5V, 80 pixels used, data on pin 6 through a 220 Ω resistor
- 5V power adapter for the strip, with a 1000 µF capacitor across + and −
- The strip, the Arduino and the sensor share one ground

See `wiring-record.md` for the full wiring.

## Uploading

1. In the Arduino IDE, install the libraries **Adafruit NeoPixel** and **Adafruit_VL53L0X** (accept all dependencies).
2. Plug in the Arduino over USB, and choose **Arduino NANO 33 IoT** in Select Board.
3. Open `fading_light/fading_light.ino`, click Verify, then Upload.
4. Open the Serial Monitor at 115200 baud to see the distance and light level.
5. Check the adapter is set to 5V, then plug it in to power the strip.

To shut down: unplug the adapter first, then the USB.

## Tuning

All the settings are at the top of `fading_light.ino`: colour (`BASE_R`, `BASE_G`, `BASE_B`), reaction distance (`FAR_MM`), glitch distance (`GLITCH_MM`), recovery speed (`RECOVER_SPEED`), dimmest level (`LOWEST_LEVEL`) and overall brightness (`MAX_BRIGHTNESS`).
