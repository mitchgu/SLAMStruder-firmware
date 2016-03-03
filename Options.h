/*
  Options.h - General parameters for SLAMWare
  Created by Mitchell Gu
*/

#ifndef Options_h
#define Options_h

// MODE CONFIGURATION
#define TEMP_MODE 0
#define TEMP_VAL 240
#define TEMP_MIN 170
#define TEMP_MAX 300
#define TEMP_HUE 4

#define EXTRUDE_MODE 1
#define EXTRUDE_VAL 50
#define EXTRUDE_MIN 10
#define EXTRUDE_MAX 200
#define EXTRUDE_HUE 120

#define FAN_MODE 2
#define FAN_VAL 128
#define FAN_MIN 0
#define FAN_MAX 255
#define FAN_HUE 224

#define SERVO_MODE 3
#define SERVO_VAL 28
#define SERVO_MIN 28
#define SERVO_MAX 144
#define SERVO_HUE 276

// Servo Positions
#define SERVO_POS_A 28
#define SERVO_POS_B 144

// Display, Interface Options
#define DISPBAUD 9600
#define LONG_PRESS_TIME 300
#define TEMP_HOLD_TIME 750
#define DEBOUNCE_INTERVAL 5 // in ms

// Extrusion Options
#define MM_PER_STEP 0.00345 // Calibrated from several measurements
#define MM_OUT_OVER_IN 1.90 // How many mm come out when 1 mm goes in, depends on nozzle, calibrated from measurement
#define RETRACT_SPEED 10 // mm per s
#define RETRACT_LENGTH 3 // mm

// Thermistor Measurement Options
#define ADC_RESOLUTION 12 // How many bits of resolution the ADC measures with
#define ADC_AVERAGING 8 // How many ADC samples to average together
#define THERM_PULLUP_R 4700 // Pull up resistance of thermistor connector. 4.7k on SLAM Board V1.0

// Thermistor coefficients, according to the Steinhart-Hart model
// https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation
// The following should be accurate for the Semitec 104GT-2 thermistor.
// http://www.atcsemitec.co.uk/gt-2-glass-thermistors.html
#define THERM_COEFF_A 0.000811290160145459F
#define THERM_COEFF_B 0.000211355789144265F
#define THERM_COEFF_C 7.17614730463848e-08F

// Heater PID Parameters
// PWM Parameters from the Ziegler-Nichols method, done manually: https://en.wikipedia.org/wiki/Ziegler%E2%80%93Nichols_method
// Seems to work well, though aggressive
#define HEAT_KP 30
#define HEAT_KI 26.9
#define HEAT_KD 8.3625
#define MAX_HEAT_PWM 220 // A duty cycle that's too high might damage thermistor and nichrome. 
#define PID_LOOP_INTERVAL 10000

#endif