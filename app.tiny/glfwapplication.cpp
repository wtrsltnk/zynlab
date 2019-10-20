 #include "glfwapplication.h"

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    io.ConfigDockingWithShift = false;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    io.Fonts->Clear();
    ImFont *font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    if (font != nullptr)
    {
        io.FontDefault = font;
    }
    else
    {
        io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();

    if (!OnInit())
    {
        return -1;
    }

    while (Tick())
    {
        glfwMakeContextCurrent(_window);
        glClear(GL_COLOR_BUFFER_BIT);

        this->OnRender();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        this->OnRenderUi();

        ImGui::Render();

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        int display_w, display_h;
        glfwMakeContextCurrent(_window);
        glfwGetFramebufferSize(_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
