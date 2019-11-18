#ifndef APP_TINY_H
#define APP_TINY_H

#include "glfwapplication.h"
#include <zyn.nio/RtMidi.h>

class AppTiny : public GLFWApplication
{
    RtMidiOut *midiout;
public:
    AppTiny();

    virtual bool OnInit();
    virtual void OnTick(double timeInMs);
    virtual void OnRender();
    virtual void OnRenderUi();
    virtual void OnDestroy();
    virtual void OnKeyAction(int key, int scancode, int action, int mods);
    virtual void OnResize(int width, int height);

    void RefreshMidiChannels();
    std::vector<std::string> _portNames;
    int _selectedPort;
};

#endif // APP_TINY_H
