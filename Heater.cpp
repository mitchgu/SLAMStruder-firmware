/*
  Heater.h - A class to regulate the SLAMSTRUDER heater
  Created by Mitchell Gu
*/

#include <math.h>
#include "Heater.h"

Heater::Heater(
    unsigned int heaterPin,
    unsigned int thermPin,
    unsigned int fanPin,
    float SH_A,
    float SH_B,
    float SH_C,
    float kp,
    float ki,
    float kd,
    unsigned int maxPWM,
    unsigned int loopInterval,
    unsigned int ADCResolution,
    unsigned int ADCAveraging,
    unsigned int thermPullupR):
    _heaterPin(heaterPin),
    _thermPin(thermPin),
    _fanPin(fanPin),
    _SH_A(SH_A),
    _SH_B(SH_B),
    _SH_C(SH_C),
    _kp(kp),
    _ki(ki),
    _kd(kd),
    _maxPWM(maxPWM),
    _loopInterval(loopInterval),
    _ADCMax(pow(2,ADCResolution)),
    _thermPullupR(thermPullupR) {

    // ADC Settings
    analogReadRes(ADCResolution);          // Teensy: set ADC resolution to this many bits
    analogReadAveraging(ADCAveraging);    // average this many readings

    pinMode(_heaterPin, OUTPUT);
    pinMode(_fanPin, OUTPUT);

    digitalWrite(_heaterPin, LOW);
    digitalWrite(_fanPin, LOW);
    
    updateTemp();
}

void Heater::enable() {
    _enabled = true;
    digitalWrite(_fanPin, HIGH);
}

void Heater::disable() {
    _enabled = false;
    analogWrite(_heaterPin, 0); // Turn off heater
    digitalWrite(_fanPin, LOW); // Turn off fan
}

void Heater::updateTemp() {
    int analogVal = analogRead(_thermPin);
    // Logarithm of the resistance, derived from voltage divider equation
    float logR = log((_thermPullupR * analogVal) / (_ADCMax - analogVal));
    curTemp = 1 / (_SH_A + _SH_B * logR + _SH_C * pow(logR,3)) - 273.15;
}

void Heater::run() {
    if (lastPIDTime > _loopInterval) {
        lastPIDTime -= _loopInterval;
        updateTemp();
        if (_enabled) {
            // PID Magic
            float error = targetTemp - curTemp;
            float newIError = max(min(iError + error * _loopInterval / 1E6f, 10), 0);
            float dError = (error - lastError) / _loopInterval * 1E6f;
            int PIDOUT =(_kp * error + _ki * iError + _kd * dError);
            int PWMOUT = min(max(PIDOUT,0), _maxPWM);
            if (PWMOUT == PIDOUT) {
                // Only update integral error if output isn't saturated
                iError = newIError;
            }
            Serial.print(micros());
            Serial.print(" ");
            Serial.print(curTemp);
            Serial.print(" ");
            Serial.print(error);
            Serial.print(" ");
            Serial.print(iError);
            Serial.print(" ");
            Serial.print(PWMOUT);
            analogWrite(_heaterPin, PWMOUT);
            lastError = error;
            Serial.print("\n");
        }
    }
}