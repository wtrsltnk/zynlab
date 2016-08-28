#ifndef _APP_THREE_DEE_H_
#define _APP_THREE_DEE_H_

#include "zyn.synth/FFTwrapper.h"
#include "zyn.mixer/Mixer.h"
#include "zyn.common/Util.h"

//Nio System
#include "zyn.nio/Nio.h"

static Mixer* mixer;
SYNTH_T* synth;

void KeyActionCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ResizeCallback(GLFWwindow* window, int width, int height);
bool SetUp();
void Render();
void CleanUp();

#endif // _APP_THREE_DEE_H_
