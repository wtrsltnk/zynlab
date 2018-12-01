#include "vst.instrument.h"

#define NUM_PROGRAMS 2
#define NUM_PARAMS 0

ZynEditor::ZynEditor(AudioEffect *effect)
    : AEffEditor(effect)
{}

bool ZynEditor::getRect(ERect **rect)
{
    *rect = &_rect;

    short w, h;
    if (_main.getSize(w, h))
    {
        _rect.left = 0;
        _rect.right = w;
        _rect.top = 0;
        _rect.bottom = h;

        return true;
    }

    return false;
}

bool ZynEditor::open(void *ptr)
{
    _hWnd = static_cast<HWND>(ptr);

    if (!_main.init(this, 400, 200))
    {
        return false;
    }

    knob.init(&_main, 48, 48);

    return true;
}

void ZynEditor::close()
{
    _main.close();
}

bool ZynEditor::isOpen()
{
    return false;
}

void ZynEditor::idle() {}
