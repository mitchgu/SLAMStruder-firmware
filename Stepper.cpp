/*
  Stepper.h - A custom class to drive a stepper extruder motor
  Created by Mitchell Gu
*/

#include "Stepper.h"

Stepper::Stepper(
            int stepPin,
            int dirPin,
            int mStepPin,
            int resetPin,
            int sleepPin,
            float mmPerStep,
            int initialSpeed,
            bool initialDir):
            _stepPin(stepPin),
            _dirPin(dirPin),
            _mStepPin(mStepPin),
            _resetPin(resetPin),
            _sleepPin(sleepPin),
            _MM_PER_STEP(mmPerStep) {  

    // Initialize Pins
    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);
    pinMode(_mStepPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);
    pinMode(_sleepPin, OUTPUT);

    setSpeed(initialSpeed, false);

    digitalWrite(_dirPin, initialDir);
    digitalWrite(_mStepPin, HIGH); // 1/32 microstepping
    digitalWrite(_resetPin, HIGH); // Never reset
    digitalWrite(_sleepPin, LOW); // Start disabled
}

void Stepper::enable() {
    digitalWrite(_sleepPin, HIGH);
    setTimer();
}

void Stepper::disable() {
    digitalWrite(_sleepPin, LOW);
    stepTimer.end();
}

void Stepper::setSpeed(float speed, bool run) {
    // speed is mm per second
    // MM_PER_STEP is mm per step
    // I want microseconds per step
    usPerStep = (int) (_MM_PER_STEP / speed * 1E6f);
    if (run) {
        setTimer();
    }
}

bool stepOn;
void STEPISR() {
    stepOn = !stepOn;
    digitalWrite(16, stepOn);
}

void Stepper::setTimer() {
    stepTimer.end();
    stepTimer.begin(STEPISR, usPerStep / 2);
}

void Stepper::setDir(bool dir) {
    digitalWrite(_dirPin, dir);
}