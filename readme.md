# SLAM Extruder V2 Firmware

This code is designed for use with the SLAM Extruder Board V1.0

## Features: Regulation and control of a KUKA arm mounted 3D printing tool.

- Controls stepper motor extrusion rate
- Regulates hotend temperature in a PID loop
- Controls servo and extruder angle position
- Regulates fan speeds for hotend and plastic cooling.
- Reads two digital inputs from the KUKA Arm
- Provides a rotary encoder, LED, and 7 segment display interface to control all features manually.

## Dependencies:
- Teensyduino (Teensy addon for Arduino)
- Bounce2 library (Not included by default but available here: https://github.com/thomasfredericks/Bounce2)
