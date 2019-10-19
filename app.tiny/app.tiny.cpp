#include "app.tiny.h"
#include <iostream>

#include <imgui.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>

AppTiny::AppTiny()
{ }

bool AppTiny::OnInit()
{
    glClearColor(0, 0.5f, 1, 1);

    return true;
}

void AppTiny::OnTick(double /*timeInMs*/)
{
}

void AppTiny::OnRender()
{
}

void AppTiny::OnRenderUi()
{
}

void AppTiny::OnDestroy()
{
}

void AppTiny::OnKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
}

void AppTiny::OnResize(int width, int height)
{
    glViewport(0, 0, width, height);
}
