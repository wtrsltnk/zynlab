#ifndef VSTCONTROL_H
#define VSTCONTROL_H

#include <windows.h>

class VstControl
{
protected:
    char const *_className;
    bool _isClassRegistered;
    VstControl *_hParent;
    HWND _hWnd;
    int _width;
    int _height;

public:
    VstControl();
    VstControl(char const *className);
    virtual ~VstControl();

    bool init(VstControl *parent, int width, int height);
    void close();
    bool getSize(short &width, short &height);
    bool isOpen() const;

    HWND handle() const { return _hWnd; }
};

#endif // VSTCONTROL_H
