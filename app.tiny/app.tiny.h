#ifndef APP_TINY_H
#define APP_TINY_H

#include "glfwapplication.h"

class AppTiny : public GLFWApplication
{
    class SceneNode *_sceneRoot;

public:
    AppTiny();

    SceneNode* BuildScene();

    virtual bool OnInit();
    virtual void OnTick(double timeInMs);
    virtual void OnRender();
    virtual void OnDestroy();
    virtual void OnKeyAction(int key, int scancode, int action, int mods);
    virtual void OnResize(int width, int height);
};

#endif // APP_TINY_H
