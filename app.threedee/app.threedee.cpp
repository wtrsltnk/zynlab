#include "app.threedee.h"

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _mixer(mixer), _window(window), _display_w(800), _display_h(600)
{
    glfwSetWindowUserPointer(this->_window, static_cast<void *>(this));
}

AppThreeDee::~AppThreeDee()
{
    glfwSetWindowUserPointer(this->_window, nullptr);
}

void AppThreeDee::KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<AppThreeDee *>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onKeyAction(key, scancode, action, mods);
}

void AppThreeDee::ResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = static_cast<AppThreeDee *>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onResize(width, height);
}

void AppThreeDee::onKeyAction(int key, int scancode, int action, int mods)
{}

void AppThreeDee::onResize(int width, int height)
{
    this->_display_w = width;
    this->_display_h = height;

    glViewport(0, 0, width, height);
}

bool AppThreeDee::SetUp()
{
    _mixer->bank.rescanforbanks();

    return true;
}

void AppThreeDee::Render()
{
}

void AppThreeDee::CleanUp()
{}
