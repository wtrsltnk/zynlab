#include "app.threedee.h"
#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include "font-icons.h"

AppThreeDee::AppThreeDee(GLFWwindow* window, Mixer* mixer)
    : _window(window), _mixer(mixer), _display_w(800), _display_h(600)
{
    glfwSetWindowUserPointer(this->_window, static_cast<void*>(this));
}

AppThreeDee::~AppThreeDee()
{
    glfwSetWindowUserPointer(this->_window, nullptr);
}

void AppThreeDee::KeyActionCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<AppThreeDee*>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onKeyAction(key, scancode, action, mods);
}

void AppThreeDee::ResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = static_cast<AppThreeDee*>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onResize(width, height);
}

void AppThreeDee::onKeyAction(int key, int scancode, int action, int mods)
{ }

static float w = 200.0f;
static float h = 300.0f;
void AppThreeDee::onResize(int width, int height)
{
    float aspectw = float(width) / float(this->_display_w);
    float aspecth = float(height) / float(this->_display_h);
    w *= aspectw;
    h *= aspecth;

    this->_display_w = width;
    this->_display_h = height;

    glViewport(0, 0, width, height);
}
#define ICON_FA_MUSIC u8"\uf001"
bool AppThreeDee::SetUp()
{
    SceneNode::Setup();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../../zynlab/app.threedee/imgui/extra_fonts/Roboto-Medium.ttf", 16.0f);

    ImFontConfig config;
    config.MergeMode = true;

    static const ImWchar icons_ranges_fontawesome[] = { 0xf000, 0xf3ff, 0 }; // will not be copied by AddFont* so keep
    io.Fonts->AddFontFromFileTTF("../../zynlab/app.threedee/fontawesome-webfont.ttf", 18.0f, &config, icons_ranges_fontawesome);

    config.MergeGlyphCenterV = true;
    static const ImWchar icons_ranges_googleicon[] = { 0xe000, 0xeb4c, 0 }; // will not be copied by AddFont* so keep
    io.Fonts->AddFontFromFileTTF("../../zynlab/app.threedee/MaterialIcons-Regular.ttf", 18.0f, &config, icons_ranges_googleicon);

    return true;
}

bool show_test_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImColor(114, 144, 154);

void AppThreeDee::Render()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
    //            ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        static bool show_library = false;
        static bool show_inspector = false;
        static bool show_smartcontrols = false;
        static bool show_editors = false;
        static bool show_mixer = false;
        static bool open = true;

        int leftWidth = (this->_display_w - 210) / 2;
        ImGui::Begin("TOOLBAR_LEFT", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetWindowPos(ImVec2(0, 22));
            ImGui::SetWindowSize(ImVec2(leftWidth, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5,15));

            if (ImGui::Button(FontAwesomeIcons::FA_INBOX, ImVec2(30, 30))) show_library ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(FontAwesomeIcons::FA_INFO, ImVec2(30, 30))) show_inspector ^= 1;

            ImGui::SameLine();
            ImGui::Text("  ");
            ImGui::SameLine();
            if (ImGui::Button(FontAwesomeIcons::FA_TACHOMETER, ImVec2(30, 30))) show_smartcontrols ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(FontAwesomeIcons::FA_SLIDERS, ImVec2(30, 30))) show_mixer ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(FontAwesomeIcons::FA_SCISSORS, ImVec2(30, 30))) show_editors ^= 1;

            ImGui::PopStyleVar();
        }
        ImGui::End();

        ImGui::Begin("TOOLBAR", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetWindowPos(ImVec2(leftWidth, 22));
            ImGui::SetWindowSize(ImVec2(210, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5,15));

            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_FAST_REWIND, ImVec2(30, 30))) show_test_window ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_FAST_FORWARD, ImVec2(30, 30))) show_test_window ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_SKIP_NEXT, ImVec2(30, 30))) show_test_window ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_PLAY_ARROW, ImVec2(30, 30))) show_test_window ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_FIBER_MANUAL_RECORD, ImVec2(30, 30))) show_test_window ^= 1;

            ImGui::PopStyleVar();
        }
        ImGui::End();

        static bool show_listeditor = false;
        static bool show_notes = false;
        static bool show_loops = false;
        static bool show_browser = false;
        ImGui::Begin("TOOLBAR_RIGHT", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetWindowPos(ImVec2(this->_display_w - 150, 22));
            ImGui::SetWindowSize(ImVec2(150, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5,15));

            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_LIST, ImVec2(30, 30))) show_listeditor ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_NOTE, ImVec2(30, 30))) show_notes ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_ALL_INCLUSIVE, ImVec2(30, 30))) show_loops ^= 1;
            ImGui::SameLine();
            if (ImGui::Button(GoogleIcons::GI_LIBRARY_MUSIC, ImVec2(30, 30))) show_browser ^= 1;

            ImGui::PopStyleVar();
        }
        ImGui::End();

        if (show_library)
        {
            ImGui::Begin("library", &show_library, ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            {
                ImGui::SetWindowPos(ImVec2(0, 62));
                ImGui::SetWindowSize(ImVec2(150, this->_display_h));

                ImGui::Text("Library");
            }
            ImGui::End();
        }

        if (show_inspector)
        {
            ImGui::Begin("inspector", &show_inspector, ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            {
                ImGui::SetWindowPos(ImVec2(show_library ? 150 : 0, 62));
                ImGui::SetWindowSize(ImVec2(150, this->_display_h));

                ImGui::Text("Inspector");
            }
            ImGui::End();
        }

        auto v = ImVec2((float)this->_display_w - (show_library ? 150 : 0) - (show_inspector ? 150 : 0), (float)this->_display_h - 62.0f);
        ImGui::Begin("Splitter test2", &open, ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetWindowPos(ImVec2((show_library ? 150 : 0) + (show_inspector ? 150 : 0), 62));
            ImGui::SetWindowSize(v);
            ImGui::SetNextWindowSizeConstraints(v, v);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

            ImGui::BeginChild("child1", ImVec2((show_library ? 150 : 0) + (show_inspector ? 150 : 0), h), true);
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15,15));
                static float f = 0.0f;
                f = this->_mixer->Pvolume / 128.0f;
                ImGui::SliderFloat("Volume", (float*)&(f), 0.0f, 1.0f);
                this->_mixer->setPvolume((unsigned char)(f * 128));
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                if (ImGui::Button("Another Window")) show_another_window ^= 1;
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();

            ImGui::InvisibleButton("hsplitter", ImVec2(-1,8.0f));
            if (ImGui::IsItemActive())
                h += ImGui::GetIO().MouseDelta.y;

            ImGui::BeginChild("child3", ImVec2(0,0), true);
            {

            }
            ImGui::EndChild();

            ImGui::PopStyleVar();
        }
        ImGui::End();
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    // Rendering
    glViewport(0, 0, this->_display_w, this->_display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
}

void AppThreeDee::CleanUp()
{ }
