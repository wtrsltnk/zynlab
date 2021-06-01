#include "vstknob.h"
#include <cmath>
#include <windowsx.h>

#define VST_RGB(r, g, b) (static_cast<COLORREF>((static_cast<BYTE>(r) | (static_cast<WORD>(static_cast<BYTE>(g)) << 8)) | ((static_cast<DWORD>(static_cast<BYTE>(b))) << 16)))

#define VST_LOWORD(l) (static_cast<WORD>((static_cast<DWORD_PTR>(l)) & 0xffff))
#define VST_HIWORD(l) (static_cast<WORD>(((static_cast<DWORD_PTR>(l)) >> 16) & 0xffff))

#define VST_GET_X_LPARAM(lp) (static_cast<int>(static_cast<short>(VST_LOWORD(lp))))
#define VST_GET_Y_LPARAM(lp) (static_cast<int>(static_cast<short>(VST_HIWORD(lp))))

LRESULT VstKnob::HandleMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hBrushGray, hBrushDarkGray;
    HPEN holdBrush;

    switch (uMsg)
    {
        case WM_PAINT:
        {
            hBrushGray = CreateSolidBrush(VST_RGB(155, 155, 155));
            hBrushDarkGray = CreateSolidBrush(VST_RGB(55, 55, 55));
            holdBrush = CreatePen(PS_SOLID, 2, VST_RGB(255, 255, 255));

            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;

            GetClientRect(this->handle(), &rect);

            float ANGLE_MIN = 3.141592f * 0.75f;
            float ANGLE_MAX = 3.141592f * 2.25f;

            float radius_outer = std::fmin(static_cast<float>(_width), static_cast<float>(_height)) / 2.0f;
            float radius_inner = radius_outer * 0.40f;

            float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (Value() - MinValue()) / (MaxValue() - MinValue());
            float angle_cos = std::cos(angle);
            float angle_sin = std::sin(angle);

            hdc = BeginPaint(this->handle(), &ps);
            SelectObject(hdc, hBrushGray);
            Ellipse(hdc, 0, 0, _width, _height);
            SelectObject(hdc, hBrushDarkGray);
            Ellipse(hdc, 0, 0, _width - 1, _height - 1);
            SelectObject(hdc, hBrushGray);
            Ellipse(hdc, _width / 4, _height / 4, _width / 4 + _width / 2, _height / 4 + _height / 2);
            SelectObject(hdc, holdBrush);
            MoveToEx(hdc,
                     static_cast<int>(_width / 2 + angle_cos * (radius_inner - 2)),
                     static_cast<int>(_height / 2 + angle_sin * (radius_inner - 2)),
                     nullptr);
            LineTo(hdc,
                   static_cast<int>(_width / 2 + angle_cos * (radius_outer - 2)),
                   static_cast<int>(_height / 2 + angle_sin * (radius_outer - 2)));
            EndPaint(handle(), &ps);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            _startMouseDrag[0] = VST_GET_X_LPARAM(lParam);
            _startMouseDrag[1] = VST_GET_Y_LPARAM(lParam);
            _startMouseDragValue = Value();

            _mouseDrag = true;
            SetCapture(_hWnd);
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (!_mouseDrag)
            {
                break;
            }
            auto yPos = VST_GET_Y_LPARAM(lParam);

            Value(_startMouseDragValue + (yPos - _startMouseDrag[1]) / 10);
            InvalidateRect(_hWnd, nullptr, FALSE);
            break;
        }
        case WM_LBUTTONUP:
        {
            _mouseDrag = false;
            SetCapture(nullptr);
            break;
        }
    }
    return DefWindowProc(handle(), uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto knob = reinterpret_cast<VstKnob *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (knob != nullptr)
    {
        if (uMsg == WM_DESTROY)
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);

            knob->close();
        }
        return knob->HandleMessages(uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
        case WM_CREATE:
        {
            knob = reinterpret_cast<VstKnob *>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);

            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(knob));

            return knob->HandleMessages(uMsg, wParam, lParam);
        }
        default:
        {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
}

VstKnob::VstKnob()
    : VstControl("VstKnob"), _value(0), _minValue(0), _maxValue(127), _mouseDrag(false)
{
    WNDCLASSEX wcex = {sizeof(wcex)};
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.lpszClassName = _className;
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_HIGHLIGHTTEXT);
    RegisterClassEx(&wcex);
}

VstKnob::~VstKnob()
{
    UnregisterClass("VstKnob", GetModuleHandle(nullptr));
}

unsigned char VstKnob::Value() const
{
    return _value;
}

void VstKnob::Value(unsigned char value)
{
    if (value < _minValue)
    {
        return;
    }

    if (value > _maxValue)
    {
        return;
    }

    _value = value;

    InvalidateRect(_hWnd, nullptr, FALSE);
}

void VstKnob::Value(int value)
{
    Value(static_cast<unsigned char>(value));
}

unsigned char VstKnob::MinValue() const
{
    return _minValue;
}

void VstKnob::MinValue(unsigned char value)
{
    _minValue = value;
}

unsigned char VstKnob::MaxValue() const
{
    return _maxValue;
}

void VstKnob::MaxValue(unsigned char value)
{
    _maxValue = value;
}
