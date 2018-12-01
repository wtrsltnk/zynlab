#ifndef VSTKNOB_H
#define VSTKNOB_H

#include "vstcontrol.h"

class VstKnob : public VstControl
{
    unsigned char _value;
    unsigned char _minValue;
    unsigned char _maxValue;
    LRESULT HandleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool _mouseDrag;
    unsigned char _startMouseDragValue;
    int _startMouseDrag[2];
public:
    VstKnob();
    virtual ~VstKnob();

    unsigned char Value() const;
    void Value(unsigned char value);
    void Value(int value);

    unsigned char MinValue() const;
    void MinValue(unsigned char value);

    unsigned char MaxValue() const;
    void MaxValue(unsigned char value);
};

#endif // VSTKNOB_H
