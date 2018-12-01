#include "vstcontrol.h"

VstControl::VstControl()
    : _className("VstControl")
{
    WNDCLASSEX wcex{sizeof(WNDCLASSEX)};
    wcex.lpfnWndProc = DefWindowProc;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.lpszClassName = _className;
    RegisterClassEx(&wcex);
    _isClassRegistered = true;
}

VstControl::VstControl(char const *className)
    : _className(className), _isClassRegistered(false)
{}

VstControl::~VstControl()
{
    if (_isClassRegistered)
    {
        UnregisterClass(_className, GetModuleHandle(nullptr));
    }
}

bool VstControl::init(VstControl *parent, int width, int height)
{
    _hParent = parent;
    _width = width;
    _height = height;

    _hWnd = CreateWindowEx(0, _className, "VstControl",
                           WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
                           0, 0, width, height,
                           _hParent->_hWnd,
                           nullptr, nullptr,
                           static_cast<void *>(this));

    if (_hWnd == nullptr)
    {
        return false;
    }

    ShowWindow(_hWnd, SW_SHOW);

    return true;
}

void VstControl::close()
{
    DestroyWindow(_hWnd);
    _hWnd = nullptr;
}

bool VstControl::getSize(short &width, short &height)
{
    RECT rect;

    if (GetWindowRect(_hWnd, &rect) == FALSE)
    {
        return false;
    }

    width = static_cast<short>(rect.right - rect.left);
    height = static_cast<short>(rect.bottom - rect.top);

    return true;
}

bool VstControl::isOpen() const
{
    return _hWnd != nullptr;
}
