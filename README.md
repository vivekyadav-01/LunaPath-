# Mars Rover: Low-Cost WiFi Exploration Rover

A low-cost, beginner-friendly rover built from an **Arduino Uno**, an **ESP32**, an **ESP32-CAM**, an **L298N motor driver** and a **servo-scanned ultrasonic sensor**. It is driven from any web browser, streams live video, and shows environmental sensor data (temperature, humidity, soil moisture, gas level) on a single dashboard.

> Built for a hackathon. Replace the placeholders below (team name, demo link, photos) with your own.

---

## Problem

Remote exploration of rough or hazardous terrain (planetary surfaces, disaster zones, farms, hazardous sites) is done by robots such as NASA's Perseverance. Those platforms cost billions, and even the open-source educational replicas need a Raspberry Pi, 3D printing, ROS and a lot of assembly. Students and small teams need a **cheap, simple, hackable** platform to learn and prototype remote exploration.

## Our solution

A modular rover on widely available hobby boards, controlled from a phone or laptop browser with no app and no ROS:

- **Dual-controller design.** The Uno handles real-time motor control and safety, while the ESP32 handles WiFi and the dashboard. If WiFi drops, the Uno stops the rover by itself.
- **Live first-person video** from the ESP32-CAM, in the same browser page.
- **Environment survey payload** with live readings on the dashboard.
- **Servo-scanned obstacle avoidance** and a **radar view** of what is around the rover.

---

## Features

| Feature | Details |
|---|---|
| Remote driving | Forward, back, pivot left/right, speed slider, from a browser |
| Live video | MJPEG stream from the ESP32-CAM |
| Obstacle safety | Manual mode stops at 20 cm; link-loss failsafe stops motors after 0.7 s |
| Auto mode | Drives forward, scans left and right with the servo, turns toward the clearer side |
| Radar scan | SCAN button sweeps the ultrasonic sensor 20-160 degrees and plots distances |
| Environment data | DHT11 temperature and humidity, thermistor temperature, soil moisture, MQ gas level |
| WiFi resilience | Auto-reconnect watchdog on the ESP32 |

---

## Hardware

| Part | Purpose |
|---|---|
| Arduino Uno | Motors, servo, ultrasonic and analog sensors |
| ESP32 DevKit | WiFi, web dashboard, relays commands to the Uno |
| ESP32-CAM (AI Thinker) | Live video |
| L298N motor driver | Drives left and right motor groups |
| DC geared motors and wheels | Locomotion |
| HC-SR04 ultrasonic sensor | Distance measurement |
| SG90 servo | Rotates the ultrasonic sensor |
| DHT11 | Air temperature and humidity |
| Soil moisture sensor | Soil moisture |
| MQ gas sensor | Gas / air-quality level |
| 10k NTC thermistor | Temperature |
| 2x 18650 pack and buck converter (5V, 3A) | Power |
| Resistors 1k and 2k | Voltage divider for the Uno-to-ESP32 serial link |
| Capacitors (1000 uF, 470 uF, 0.1 uF) | Supply and motor noise filtering |

### Pin map (Arduino Uno)

| Function | Pin |
|---|---|
| L298N ENA, IN1, IN2 | D5, D7, D8 |
| L298N ENB, IN3, IN4 | D6, D9, D10 |
| HC-SR04 TRIG, ECHO | D3, D4 |
| Servo signal | D13 |
| DHT11 data | D2 |
| Soil moisture (AO) | A0 |
| MQ gas (AO) | A1 |
| Thermistor divider | A2 |
| Serial link to ESP32 | D11 (RX), D12 (TX) |

### Uno to ESP32 link

- ESP32 GPIO17 (TX2) to Uno D11 (RX)
- Uno D12 (TX) to a **1k / 2k voltage divider** to ESP32 GPIO16 (RX2). The Uno outputs 5V and the ESP32 is 3.3V only.
- All grounds are shared.

---

## System architecture

```
 Browser (phone / laptop)
        |  WiFi (2.4 GHz hotspot or router)
        +--------------------------+
        |                          |
   ESP32-CAM                  ESP32 DevKit
   video :81/stream           dashboard :80
                                   |  UART 9600 (level-shifted)
                              Arduino Uno
              +--------+-----------+-----------+------------+
              |        |           |           |            |
           L298N   Servo+HC-SR04   DHT11   Soil / MQ    Thermistor
           motors   (radar)
```

---

## Repository structure

```
firmware/
  rover_uno_v3/                  Arduino Uno: motors, servo scan, sensors
  rover_esp32_controller_v3/     ESP32 DevKit: web dashboard
  rover_esp32cam/                ESP32-CAM sketch for OV2640 cameras
  rover_esp32cam_gc2145/         ESP32-CAM sketch for GC2145 cameras (software JPEG)
docs/
  circuit_diagram.png
```

---

## Software and libraries

- Arduino IDE 2.x
- ESP32 board package by Espressif (Boards Manager)
- Libraries: **DHT sensor library** (Adafruit) with Adafruit Unified Sensor. `Servo`, `SoftwareSerial`, `WiFi`, `WebServer` and `esp_camera` come with the Arduino core.

## Setup

1. **ESP32-CAM.** Enter your WiFi name and password, select board **AI Thinker ESP32-CAM**, put GPIO0 to GND to flash, then remove it and reset. Note the IP printed in Serial Monitor (115200 baud). Use `rover_esp32cam_gc2145` if the monitor reports a sensor PID of `0x2145`, or if you see "JPEG format is not supported on this sensor".
2. **ESP32 DevKit.** In `rover_esp32_controller_v3.ino` set `WIFI_SSID`, `WIFI_PASS` and `CAM_IP`, select **ESP32 Dev Module**, and upload. Note the dashboard IP.
3. **Arduino Uno.** Upload `rover_uno_v3.ino`. Disconnect the buck converter 5V from the Uno while it is connected to USB.
4. Power the rover from the battery and connect your phone or laptop to the **same 2.4 GHz network**.
5. Open `http://<ESP32-DevKit-IP>` in a browser.

> **Do not commit your WiFi password.** Keep real credentials out of the repository.

### Calibration

- **Soil moisture:** read the raw value in dry air and in water (shown on the dashboard and in the Uno Serial Monitor), then set `SOIL_DRY` and `SOIL_WET` in the Uno sketch.
- **Thermistor:** set `THERM_TO_GND` to match your wiring (see comments in the sketch).
- **MQ sensor:** warm it up for several minutes. Values are relative readings, not calibrated ppm.
- **Radar direction:** if left and right look reversed, swap `LEFT_ANGLE` and `RIGHT_ANGLE` in the Uno sketch, or set `MIRROR` in the dashboard script.

---

## How it differs from existing solutions

| | NASA Perseverance | JPL Open Source Rover | Sawppy | This project |
|---|---|---|---|---|
| Purpose | Real Mars science | Educational scaled-down rover | Low-cost hobby model | Low-cost remote exploration platform |
| Brain | Flight computers | Raspberry Pi | Servo controller / optional ROS | Arduino Uno + ESP32 + ESP32-CAM |
| Control | Ground team | Controller / browser | RC / software | Any browser, no app |
| Skill needed | Advanced | Linux, ROS, soldering | 3D printing, CAD | Beginner to intermediate |

Our differences are the dual-controller failsafe design, the very low cost, and the single browser dashboard that combines video, drive control and sensor data. We do **not** claim the mechanical capability of rocker-bogie rovers.

---

## Project status

Update this section to match what you have actually built and tested.

- [x] Dual-controller architecture and serial link
- [x] WiFi dashboard with drive controls
- [x] Live video stream (ESP32-CAM)
- [x] Obstacle stop and link-loss failsafe
- [ ] Servo-scanned auto avoidance and radar (tick when tested)
- [ ] DHT11, thermistor, soil moisture and MQ readings on dashboard (tick when tested)

## Known limitations

- Rigid chassis (no rocker-bogie suspension), so terrain capability is limited.
- Obstacle detection uses a single ultrasonic sensor with a narrow cone.
- WiFi range limits the operating distance.
- GC2145 cameras use software JPEG conversion, giving a lower frame rate (about 3-8 fps at QVGA).
- The MQ sensor gives uncalibrated relative values, and DHT11 accuracy is about +/-2 C and +/-5 % RH.
- The L298N drops about 2V and runs hot. A TB6612FNG would be more efficient.
- Not weatherproof.

## Future work

- Rocker-bogie suspension
- Wheel encoders and an IMU for tilt warning and path return
- Solar charging
- Data logging and charts on the dashboard
- On-device object detection for rocks and samples
- Sealed enclosure and more efficient motor driver

## References

1. NASA JPL, Open Source Rover: https://github.com/nasa-jpl/open-source-rover
2. NASA Science, Perseverance rover components: https://science.nasa.gov/mission/mars-2020-perseverance/rover-components/
3. R. Cheng, Sawppy the Rover: https://github.com/Roger-random/Sawppy_Rover
4. STMicroelectronics, L298 dual full-bridge driver: https://www.st.com/en/product/L298
5. ElecFreaks, HC-SR04 ultrasonic ranging module datasheet: https://cse.unl.edu/~jfalkinburg/cse_courses/2019/236/HC-SR04.pdf
6. Espressif, esp32-camera driver: https://github.com/espressif/esp32-camera

## Team

- Team name:Cofee2Code
- Members :Vivek Yadav, Atieksh Pandey
  

## License

MIT License (add a `LICENSE` file), or choose another license.
