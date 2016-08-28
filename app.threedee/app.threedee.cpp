
#include <GL/glextl.h>
#include <GLFW/glfw3.h>

void KeyActionCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{ }

void ResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

bool SetUp()
{
    return true;
}

void Render()
{ }

void CleanUp()
{ }
