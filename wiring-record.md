# Beaker LED project: wiring record

## Parts

| Part | Details |
|---|---|
| Microcontroller | Arduino **Nano 33 IoT** (3.3V logic, built-in USB) |
| LED strip | SEZO **WS2812E**, DC **5V**, 60 LEDs/m, 5 m (300 LEDs), 90 W max. Wires: **Red = VCC, White = GND, Green = DATA** |
| Distance sensor | Adafruit VL53L0X time-of-flight (I2C) |
| Power | Universal AC adapter, 36 W, dial set to **5V** |
| Power jack | Barrel jack to screw terminal: **+ top (red)**, **− bottom (black)** |
| Capacitor | 1000 µF / 16 V across the jack's + and − (striped leg on −) |
| Resistor | 220 Ω (red-red-brown-gold) on the data line |
| Connectors | 2 lever connectors ("clip extenders"): one for GND, one for +5V |

## Power and ground

| # | Wire | From | To | Checked |
|---|---|---|---|---|
| 1 | Red | Jack screw terminal **+** | +5V lever connector | ✅ |
| 2 | Black | Jack screw terminal **−** | GND lever connector | ✅ |
| 3 | Capacitor | Jack **+** and **−** (stripe on −) | — | ✅ |
| 4 | White | GND lever connector | Breadboard power rail, **− (blue) column, row 47** | ✅ |
| 5 | White (strip GND) | GND lever connector, other side | Strip GND | ✅ |
| 6 | Red (strip VCC) | +5V lever connector, other side | Strip +5V | ✅ |
| 7 | Black jumper | Left rail **−**, bottom end (around row 60) | Right rail **−** (bridges the two ground rails) | ✅ |
| 8 | Pink jumper | Left rail **+**, bottom end (around row 59) | Right rail **+** (bridges the two + rails) | ✅ |
| 8a | Red jumper | Left rail **+** column (red line), top end | **Row 2, column a**, the Arduino's **3V3** pin. Feeds 3.3V to both + rails | ✅ |

## Data

| # | Wire / part | From | To | Checked |
|---|---|---|---|---|
| 9 | Strip DATA (green) | Strip → black plug → lever connector | Thin blue/green wire → **row 18, column i** | ✅ |
| 10 | Resistor 220 Ω | **Row 18, column j** (red-band end) | **Row 7, column j** (gold-band end) | ✅ |
| 11 | Arduino **D6** | Arduino pin in **row 7, column h** | Same row as the resistor (columns f–j are connected) | ✅ |
| 12 | Short black jumper (Arduino GND) | **Row 14, column c**, the Arduino's left-side **GND** | Left rail **−** | ✅ |

## Distance sensor (Adafruit VL53L0X, I2C address 0x29, range about 30–1000 mm)

| # | Sensor pin | Wire colour | Should go to | Where it is now | Checked |
|---|---|---|---|---|---|
| 13 | VIN | **Red** (sensor's own wire, not the short 3V3 jumper) | + rail (3.3V) | Left rail **+** column (red line), about **row 13**. Gets 3.3V through the short red jumper (8a) | ✅ |
| 14 | GND | Black | − rail | Left rail **−** column, around row 25 (from photo) | ✅ (confirm the exact row if you want) |
| 15 | SCL | Yellow | **Pin A5**, left side, **row 9** | Breadboard **c9**, next to the blue wire | ✅ |
| 16 | SDA | Blue | **Pin A4**, left side, **row 8** | Breadboard **c8**, pin A4 (confirmed) | ✅ |
| — | GPIO, XSHUT | — | Not connected (not needed) | Not connected | ✅ |

## Nano 33 IoT pins (USB end = row 1)

| Row | Left side | Right side |
|---|---|---|
| 1 | D13 | D12 |
| 2 | **3V3** | D11 |
| 3 | AREF | D10 |
| 4–11 | A0–A7 (A4 = SDA in row 8, A5 = SCL in row 9) | D9, D8, D7, **D6 (row 7)**, D5, D4, D3, D2 |
| 12 | VUSB (+5V) | GND |
| 13 | RST | RST |
| 14 | **GND** | RX |
| 15 | VIN | TX |

Confirm against the labels printed on the board; this assumes the board's first pins sit in row 1.

## Still to document

- Confirm nothing connects the adapter's 5V to either breadboard + rail

## Power for the Arduino

Micro-USB cable into the Nano 33 IoT (from a laptop or USB power). The Arduino's 3.3V comes from USB; the LED strip's 5V comes only from the AC adapter. They share ground through the − rails and the GND lever connector.

## First power-on test

1. Upload `presence_chase.ino` with the USB cable connected.
2. Open the Serial Monitor at **115200 baud**. You should see "Ready - waiting for someone", then distance readings.
3. Plug in the AC adapter (5V).
4. Hold a hand about 30–60 cm in front of the sensor for a moment.

| What you see | What it means |
|---|---|
| Distance readings, and the strip chases when your hand is close | Everything works |
| Distance readings, but the strip stays dark | The 3.3V data signal is too weak: try a level shifter, or the adapter at 4.5V as a test |
| "Failed to boot VL53L0X" and pixel 0 turns red | The sensor isn't answering: reseat the four sensor wires |
| Nothing in the Serial Monitor | Wrong board or port selected in the Arduino IDE, or the upload failed |

Note: "a2" in this record means **breadboard row 2, column a**, not the Arduino's analog pin A2.

## ⚠️ Do not power on until confirmed

The two + rails are joined (jumper 8) and fed from the Arduino's 3V3 pin (jumper 8a). If the adapter's 5V reaches either + rail, 5V is pushed into the board's 3.3V pin and can destroy the Nano 33 IoT. The + rails must carry **3.3V from the Arduino only**. The adapter's 5V goes only to the LED strip, through the lever connector.

## Notes

- Code: `LED_COUNT 80`, `LED_PIN 6`, `NEO_GRB + NEO_KHZ800`, brightness 60.
- Only the pixels the code turns on draw much power. 80 pixels at brightness 60 draw about 1 A or less, well within the adapter.
- If the strip doesn't light, or flickers, at 5V: the Nano 33 IoT sends a 3.3V data signal. Fixes are a **level shifter** (74AHCT125) or, for testing, the adapter at 4.5V.
