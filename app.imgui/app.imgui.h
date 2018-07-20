#ifndef _APP_THREE_DEE_H_
#define _APP_THREE_DEE_H_

#include <GL/glextl.h>
#include <GLFW/glfw3.h>

#include "../zyn.mixer/Mixer.h"
#include "../zyn.synth/FFTwrapper.h"
#include <zyn.common/Util.h>

//Nio System
#include "../zyn.nio/Nio.h"

class AppThreeDee
{
private:
    Mixer *_mixer;
    GLFWwindow *_window;

public:
    static void KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void ResizeCallback(GLFWwindow *window, int width, int height);

protected:
    int _display_w, _display_h;

    void onKeyAction(int key, int scancode, int action, int mods);
    void onResize(int width, int height);

public:
    AppThreeDee(GLFWwindow *window, Mixer *mixer);
    virtual ~AppThreeDee();

    void NoteOn();
    void NoteOff();

    bool SetUp();
    void Render(double dt);
    void CleanUp();
};

#endif // _APP_THREE_DEE_H_
