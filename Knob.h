/*
  Knob.h - A class for Sparkfun's rotary encoder RGB button knob.
  Created by Mitchell Gu
*/

#ifndef Knob_h
#define Knob_h

#include <array>
#include <Encoder.h> // Quadrature Encoder Library
#include <Bounce2.h> // Mechanical button debouncing library

class Knob {
    public: 
        Knob(
            int encPinA,
            int encPinB,
            int RLED,
            int GLED,
            int BLED,
            int btnPin,
            int debounceInterval = 5);
        ~Knob();
        bool btnChanged();
        bool btnPushed();
        int encRead();
        void encWrite(int pos);
        void updateColor(int hue, int sat, int val);
    private:
        Encoder *enc;
        Bounce *btn;
        const int _RLED;
        const int _GLED;
        const int _BLED;
        std::array<int,3> HSV2RGB(int hue, int sat, int val);
};

#endif