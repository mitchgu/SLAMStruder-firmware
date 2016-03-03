/*
  Stepper.h - A custom class to drive a stepper extruder motor
  Created by Mitchell Gu
*/

#ifndef Stepper_h
#define Stepper_h

#include <array>
#include <arduino.h>

class Stepper {
    public: 
        Stepper(
            int stepPin,
            int dirPin,
            int mStepPin,
            int reset_pin,
            int sleep_pin,
            float mm_per_step,
            int initial_speed = 1,
            bool initial_dir = true);
        void enable();
        void disable();
        void setSpeed(float speed, bool run = true);
        void setDir(bool dir);
    private:
        const int _stepPin;
        const int _dirPin;
        const int _mStepPin;
        const int _resetPin;
        const int _sleepPin;
        const float _MM_PER_STEP;
        int usPerStep;
        IntervalTimer stepTimer;
        void setTimer();
};

#endif