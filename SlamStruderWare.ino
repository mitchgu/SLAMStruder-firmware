/* SLAM Extruder V2 Firmware
 * Author: Mitchell Gu
 * This code is designed for use with the SLAM Extruder Board V1.0
 * Features: Regulation and control of a KUKA arm mounted 3D printing tool.
 * - Controls stepper motor extrusion rate
 * - Regulates hotend temperature in a PID loop
 * - Controls servo and extruder angle position
 * - Regulates fan speeds for hotend and plastic cooling.
 * - Reads two digital inputs from the KUKA Arm
 * - Provides a rotary encoder, LED, and 7 segment display interface to control all features manually.
 * DEPENDENCIES:
 * - Teensyduino (Teensy addon for Arduino)
 * - Bounce2 library (Not included by default but available here: https://github.com/thomasfredericks/Bounce2)
 */

#include <array>
#include "Pins.h" // Pin Mapping Configuration
#include "Options.h" // General Options
#include <Encoder.h>
#include <Bounce2.h>
#include "Knob.h" // Local Knob library
#include "Stepper.h" // Local Stepper library
#include "Heater.h" // Local Heater library
#include <Servo.h> // Servo library

// MODE INITIALIZATION
int mode = TEMP_MODE;
std::array<int,4> modeVal = {TEMP_VAL, EXTRUDE_VAL, FAN_VAL, SERVO_VAL};
std::array<int,4> modeMin = {TEMP_MIN, EXTRUDE_MIN, FAN_MIN, SERVO_MIN};
std::array<int,4> modeMax = {TEMP_MAX, EXTRUDE_MAX, FAN_MAX, SERVO_MAX};
std::array<int,4> modeHue = {TEMP_HUE, EXTRUDE_HUE, FAN_HUE, SERVO_HUE};

// Retract duration in us should be retract length (mm) divided by retract speed (mm per s) * 1E3f
const int RETRACT_DURATION = (int) (1E3f * RETRACT_LENGTH / RETRACT_SPEED);

// OBJECTS
Knob knob(
    ENC_PIN_A,
    ENC_PIN_B,
    KNOB_RLED,
    KNOB_GLED,
    KNOB_BLED,
    KNOB_BTN_PIN,
    DEBOUNCE_INTERVAL);
Stepper stepper(
    STEP_PIN,
    DIR_PIN,
    MSTEP_PIN,
    RESET_PIN,
    SLEEP_PIN,
    MM_PER_STEP * MM_OUT_OVER_IN);
Heater heater(
    HEATER_PIN,
    THERM_PIN,
    EXT_FAN_PIN,
    THERM_COEFF_A,
    THERM_COEFF_B,
    THERM_COEFF_C,
    HEAT_KP,
    HEAT_KI,
    HEAT_KD,
    MAX_HEAT_PWM,
    PID_LOOP_INTERVAL,
    ADC_RESOLUTION,
    ADC_AVERAGING,
    THERM_PULLUP_R);
Servo servo = Servo();

// KUKA Inputs
// Treat them as switches to be debounced
Bounce KUKAIN1 = Bounce();
Bounce KUKAIN2 = Bounce();

// STATE VARIABLES
bool extruding = false;
bool retracting = false;
bool heaterOn = false;
bool fanOn = false;
bool btnHeld = false;
long lastBtnPress;
elapsedMillis sinceRetract;
elapsedMillis sinceTempChange;

// Write a number to the seven segment display
void refreshDisp(int _mode = mode) {
  int num = modeVal[_mode];
  char tempString[10];
  switch(_mode) {
    case TEMP_MODE:
        if (sinceTempChange > TEMP_HOLD_TIME) {
            num = (int) round(heater.curTemp);
        }
        sprintf(tempString, "%3dC", num);
        DISP.write(0x77);
        DISP.write(0b00100000);
        break;
    case EXTRUDE_MODE:
      sprintf(tempString, "%3d ", num);
      DISP.write(0x77);
      DISP.write(0b00000010);
      break;
    case FAN_MODE:
      sprintf(tempString, "%3d ", num);
      DISP.write(0x77);
      DISP.write(0b00000100);
      break;
    case SERVO_MODE:
      sprintf(tempString, "%3d ", num);
      DISP.write(0x77);
      DISP.write(0b00100000);
      break;
  }
  DISP.print(tempString);
  int value = (modeVal[_mode] - modeMin[_mode]) / ((float)modeMax[_mode] - modeMin[_mode]) * 255;
  knob.updateColor(modeHue[_mode], 255, value);
}

void updateMode(int _mode = mode) {
  switch (_mode) {
    case TEMP_MODE:
        heater.targetTemp = modeVal[TEMP_MODE];
        sinceTempChange = 0;
        break;
    case EXTRUDE_MODE:
        if (!retracting) {
            // Don't change speed when it's retracting, when it stops retracting then this will be called again.
            stepper.setSpeed(modeVal[EXTRUDE_MODE] / 10.0);
        }
        break;
    case FAN_MODE:
        if (fanOn) {
            analogWrite(AUX_FAN_PIN, modeVal[FAN_MODE]);
            analogWrite(FAN_LED, modeVal[FAN_MODE] * 0.1);
        }
        else {
            analogWrite(AUX_FAN_PIN, 0);
            analogWrite(FAN_LED, LOW);
        }
        break;
    case SERVO_MODE:
        servo.write(modeVal[SERVO_MODE]);
        break;
  }
}

void handleLongPress(int _mode = mode) {
  switch (_mode) {
    case TEMP_MODE:
        heaterOn = !heaterOn;
        digitalWrite(HOT_LED, heaterOn);
        if (heaterOn) { heater.enable(); } else { heater.disable(); }
        break;
    case EXTRUDE_MODE:
        if (extruding) { startRetract(); } else { startExtrude(); }
        break;
    case FAN_MODE:
        fanOn = !fanOn;
        updateMode(FAN_MODE);
        break;
    case SERVO_MODE:
        stopRetract();
        break;
  }
}

void checkBtn() {
  if (knob.btnChanged()) {
    // If button state changed
    if (knob.btnPushed()) {
      // Save when press started
      lastBtnPress = millis();
    }
    else {
      if (btnHeld) {
        // Long press, don't switch modes
        btnHeld = false;
        handleLongPress();
        refreshDisp();
      }
      else {
        // If the button was pressed then released:
        mode = (mode + 1) % 4;
        // Switch knob position to where new mode left off
        knob.encWrite(modeVal[mode]);
        refreshDisp();
      }
    }
  }
  else if (knob.btnPushed() && !btnHeld) {
    // It's being pressed and isn't a long press yet
    if (millis() -  lastBtnPress > LONG_PRESS_TIME) {
      // Long press, change to white and return
      knob.updateColor(0, 0, 255);
      btnHeld = true;
    }
  }
  else if (knob.btnPushed() && btnHeld && mode == SERVO_MODE && !extruding && !retracting) {
    startRetract();
  }
}

void checkKnob() {
  int newVal = knob.encRead();
  if (modeVal[mode] != newVal) {
    if (modeMin[mode] <= newVal && newVal <= modeMax[mode]) {
      // We're in the valid range, write the new value
      modeVal[mode] = newVal;
      updateMode();
      refreshDisp();
    }
    else {
      // We've hit the end of the range, revert knob position
      knob.encWrite(modeVal[mode]);
    }
  }
}

void checkKUKA() {
    if (KUKAIN1.update()) {
        if (KUKAIN1.read()) { startExtrude(); } else { startRetract(); }
    }
    if (KUKAIN2.update()) {
        modeVal[SERVO_MODE] = KUKAIN2.read() ? SERVO_POS_A : SERVO_POS_B;
        if (mode == SERVO_MODE) {
            knob.encWrite(modeVal[mode]);
        }
        updateMode(SERVO_MODE);
        refreshDisp();
    }
}

void startExtrude() {
    stopRetract(); // if at all
    extruding = true;
    digitalWrite(EXT_LED, HIGH);
    stepper.enable();
}

void startRetract() {
    extruding = false;
    retracting = true;
    digitalWrite(EXT_LED, LOW);
    digitalWrite(RET_LED, HIGH);
    stepper.setSpeed(RETRACT_SPEED);
    stepper.setDir(0);
    stepper.enable();
    sinceRetract = 0;
}

void stopRetract() {
    if (retracting) {
        retracting = false;
        digitalWrite(RET_LED, LOW);
        stepper.setDir(1);
        updateMode(EXTRUDE_MODE);
        stepper.disable();
    }
}

void setup() {
    Serial.begin(9600); // USB serial for debugging

    // Setup the display
    // Have to wait up a bit for the 7segment MCU to start lolol
    delay(100);
    DISP.begin(DISPBAUD);
    DISP.write(0x76); // Clear display

    knob.encWrite(modeVal[mode]);

    // Setup LED Indicators
    pinMode(EXT_LED, OUTPUT);
    pinMode(RET_LED, OUTPUT);
    pinMode(HOT_LED, OUTPUT);
    pinMode(FAN_LED, OUTPUT);

    servo.attach(SERVO_PIN);  // Attach the servo

    // Setup auxiliary fan pin
    pinMode(AUX_FAN_PIN, OUTPUT);

    // Setup KUKA inputs
    pinMode(KUKA_PIN_1, INPUT_PULLUP);
    KUKAIN1.attach(KUKA_PIN_1); // Attach knob button
    KUKAIN1.interval(DEBOUNCE_INTERVAL);
    pinMode(KUKA_PIN_2, INPUT_PULLUP);
    KUKAIN2.attach(KUKA_PIN_2); // Attach knob button
    KUKAIN2.interval(DEBOUNCE_INTERVAL);


    updateMode(TEMP_MODE);
    updateMode(EXTRUDE_MODE);
    updateMode(FAN_MODE);
    updateMode(SERVO_MODE);

    refreshDisp();
}

void loop() {
    checkBtn();
    checkKnob();
    checkKUKA();
    if (retracting && (sinceRetract > RETRACT_DURATION) && !(btnHeld && mode == SERVO_MODE)) { stopRetract(); }
    if (mode == TEMP_MODE && sinceTempChange > TEMP_HOLD_TIME) { refreshDisp(); }
    heater.run();
}
