/*
  Knob.cpp - A class for Sparkfun's rotary encoder RGB button knob
  Created by Mitchell Gu
*/

#include "Knob.h"
#include <Arduino.h> // Base Arduino library

Knob::Knob(
        int encPinA,
        int encPinB,
        int RLED,
        int GLED,
        int BLED,
        int btnPin,
        int debounceInterval): 
        _RLED(RLED),
        _GLED(GLED),
        _BLED(BLED) {

    // Initialize encoder and knob button
    enc = new Encoder(encPinA, encPinB);

    // Initialize button with debouncing
    pinMode(btnPin, INPUT_PULLUP);
    btn = new Bounce();
    btn->attach(btnPin); // Attach knob button
    btn->interval(debounceInterval); // Debounce interval

    // Initialize RGB LED outputs
    pinMode(_RLED, OUTPUT);
    pinMode(_GLED, OUTPUT);
    pinMode(_BLED, OUTPUT);
}

Knob::~Knob() {
    delete enc;
    delete btn;
}

// Returns true if the button state has changed since last check
bool Knob::btnChanged() {
    return btn->update();
}

// Returns true if button is currently depressed
bool Knob::btnPushed() {
    return btn->read();
}

// Returns the encoder position divided by four to get one offset per detent
// The plus two is to make it so that the transition is in the middle of adjacent knob detents. (For quadrature encoders there are four positions between detents)
int Knob::encRead() {
    return (enc->read() + 2) / 4;
}

// Sets the encoder position after scaling by four first
void Knob::encWrite(int pos) {
    enc->write(pos * 4);
}

void Knob::updateColor(int hue, int sat, int val) {
    std::array<int,3> rgb = HSV2RGB(hue, sat, val);
    analogWrite(_RLED, (255 - rgb[0]));
    analogWrite(_GLED, (255 - rgb[1]));
    analogWrite(_BLED, (255 - rgb[2]));
}

// Helper function to translate HSL values into RGB
std::array<int,3> Knob::HSV2RGB(int hue, int sat, int val) {
  // hue: 0-259, sat: 0-255, val (lightness): 0-255
  int r = 0, g = 0, b = 0, base;
  if (sat == 0) { // Achromatic color (gray).
    r = val;
    g = val;
    b = val;
  } else  {
    base = ((255 - sat) * val)>>8;
    switch(hue/60) {
      case 0:
        r = val;
        g = (((val-base)*hue)/60)+base;
        b = base;
        break;
      case 1:
        r = (((val-base)*(60-(hue%60)))/60)+base;
        g = val;
        b = base;
        break;
      case 2:
        r = base;
        g = val;
        b = (((val-base)*(hue%60))/60)+base;
        break;
      case 3:
        r = base;
        g = (((val-base)*(60-(hue%60)))/60)+base;
        b = val;
        break;
      case 4:
        r = (((val-base)*(hue%60))/60)+base;
        g = base;
        b = val;
        break;
      case 5:
        r = val;
        g = base;
        b = (((val-base)*(60-(hue%60)))/60)+base;
        break;
    }
  }
  return {r, g, b};
}