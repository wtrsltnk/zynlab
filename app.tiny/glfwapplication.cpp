#include "glfwapplication.h"

GLFWApplication::GLFWApplication() {}

GLFWApplication::~GLFWApplication() {}

int GLFWApplication::Run()
{
    if (glfwInit() == GLFW_FALSE)
    {
        return false;
    }

    _window = glfwCreateWindow(1024, 768, "zynlab", nullptr, nullptr);
    if (_window == nullptr)
    {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(_window, static_cast<void *>(this));
    glfwSetKeyCallback(_window, GLFWApplication::KeyActionCallback);
    glfwSetWindowSizeCallback(_window, GLFWApplication::ResizeCallback);
    glfwMakeContextCurrent(_window);

    gladLoadGL();

    if (!OnInit())
    {
        return -1;
    }

    while (Tick())
    {
        glfwMakeContextCurrent(_window);
        glClear(GL_COLOR_BUFFER_BIT);

        this->OnRender();

        glfwSwapBuffers(_window);
    }

    glfwSetWindowUserPointer(_window, nullptr);

    this->OnDestroy();

    return 0;
}

bool GLFWApplication::Tick()
{
    if (glfwWindowShouldClose(_window) != 0)
    {
        return false;
    }

    glfwPollEvents();

    return true;
}

void GLFWApplication::KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<GLFWApplication *>(glfwGetWindowUserPointer(window));

    if (app != nullptr)
    {
        app->OnKeyAction(key, scancode, action, mods);
    }
}

void GLFWApplication::ResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = static_cast<GLFWApplication *>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->OnResize(width, height);
}
