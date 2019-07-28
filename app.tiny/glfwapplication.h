#ifndef GLFWAPPLICATION_H
#define GLFWAPPLICATION_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

class GLFWApplication
{
    GLFWwindow *_window;

public:
    GLFWApplication();
    virtual ~GLFWApplication();

    int Run();

protected:
    virtual bool OnInit() = 0;
    virtual void OnTick(double timeInMs) = 0;
    virtual void OnRender() = 0;
    virtual void OnDestroy() = 0;
    virtual void OnKeyAction(int key, int scancode, int action, int mods) = 0;
    virtual void OnResize(int width, int height) = 0;

private:
    bool Tick();
    static void KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void ResizeCallback(GLFWwindow *window, int width, int height);
};

#endif // GLFWAPPLICATION_H
