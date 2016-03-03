/*
  Heater.h - A class to regulate the SLAMSTRUDER heater
  Created by Mitchell Gu
*/

#ifndef Heater_h
#define Heater_h

#include <arduino.h>

class Heater {
    public: 
        Heater(
            unsigned int heaterPin,
            unsigned int thermPin,
            unsigned int fanPin,
            float SH_A,
            float SH_B,
            float SH_C,
            float kp,
            float ki,
            float kd,
            unsigned int maxPWM = 255,
            unsigned int loopInterval = 10000,
            unsigned int ADCResolution = 12,
            unsigned int ADCAveraging = 4,
            unsigned int thermPullupR = 4700);
        void enable();
        void disable();
        void setTargetTemp(float temp);
        float getTargetTemp();
        void updateTemp();
        void run();
        float curTemp;
        float targetTemp;
    private:
        bool _enabled;
        const unsigned int _heaterPin;
        const unsigned int _thermPin;
        const unsigned int _fanPin;
        const float _SH_A;
        const float _SH_B;
        const float _SH_C;
        const float _kp;
        const float _ki;
        const float _kd;
        int _maxPWM;
        const unsigned int _loopInterval;
        const unsigned int _ADCMax;
        const unsigned int _thermPullupR;
        elapsedMicros lastPIDTime;
        float lastError;
        float iError;
};

#endif