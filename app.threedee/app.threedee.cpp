#include "app.threedee.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "imgui_impl_glfw_gl3.h"
#include "font-icons.h"

struct {
    bool show_library = false;
    bool show_inspector = true;
    bool show_smartcontrols = false;
    bool show_editors = false;
    bool show_mixer = false;
    bool show_listeditor = false;
    bool show_notes = false;
    bool show_loops = false;
    bool show_browser = false;
    float w = 200.0f;
    float h = 300.0f;

} windowConfig;

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

bool Knob(const char* label, float* value_p, float minv, float maxv)
{
    ImGuiStyle& style = ImGui::GetStyle();
    float line_height = ImGui::GetTextLineHeight();

    ImVec2 p = ImGui::GetCursorScreenPos();
    float sz = 36.0f;
    float radio =  sz*0.5f;
    ImVec2 center = ImVec2(p.x + radio, p.y + radio);
    float val1 = (value_p[0] - minv)/(maxv - minv);
    char textval[32];
    ImFormatString(textval, IM_ARRAYSIZE(textval), "%04.1f", value_p[0]);

    ImVec2 textpos = p;
    float gamma = M_PI/4.0f;//0 value in knob
    float alpha = (M_PI-gamma)*val1*2.0f+gamma;

    float x2 = -sinf(alpha)*radio + center.x;
    float y2 = cosf(alpha)*radio + center.y;

    ImGui::InvisibleButton(label,ImVec2(sz, sz + line_height + style.ItemInnerSpacing.y));

    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemHovered();
    bool touched = false;

    if (is_active)
    {
        touched = true;
        ImVec2 mp = ImGui::GetIO().MousePos;
        alpha = atan2f(mp.x - center.x, center.y - mp.y) + M_PI;
        alpha = ImMax(gamma,ImMin(2.0f*M_PI-gamma,alpha));
        float value = 0.5f*(alpha-gamma)/(M_PI-gamma);
        value_p[0] = value*(maxv - minv) + minv;
    }

    ImU32 col32 = ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImU32 col32line = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
    ImU32 col32text = ImGui::GetColorU32(ImGuiCol_Text);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCircleFilled(center, radio, col32, 16);
    draw_list->AddLine(center, ImVec2(x2, y2), col32line, 1);
    draw_list->AddText(textpos, col32text, textval);
    draw_list->AddText(ImVec2(p.x, p.y + sz + style.ItemInnerSpacing.y), col32text, label);

    return touched;
}

void AppThreeDee::onResize(int width, int height)
{
    float aspectw = float(width) / float(this->_display_w);
    float aspecth = float(height) / float(this->_display_h);
    windowConfig.w *= aspectw;
    windowConfig.h *= aspecth;

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

        static bool open = true;
        const int inspectorWidth = 200;
        const int libraryWidth = 200;
        const ImVec4 checkedButtonColor = ImVec4(0.47f, 0.20f, 0.20f, 0.60f);

        int leftWidth = (this->_display_w - 210) / 2;
        ImGui::Begin("TOOLBAR_LEFT", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetWindowPos(ImVec2(0, 22));
            ImGui::SetWindowSize(ImVec2(leftWidth, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5,15));

            bool tmp = windowConfig.show_library;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(FontAwesomeIcons::FA_INBOX, ImVec2(30, 30))) windowConfig.show_library ^= 1;
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::SameLine();
            tmp = windowConfig.show_inspector;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(FontAwesomeIcons::FA_INFO, ImVec2(30, 30))) windowConfig.show_inspector ^= 1;
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::SameLine();
            ImGui::Text("  ");

            ImGui::SameLine();
            tmp = windowConfig.show_smartcontrols;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(FontAwesomeIcons::FA_TACHOMETER, ImVec2(30, 30))) { windowConfig.show_smartcontrols ^= 1; windowConfig.show_mixer = false; windowConfig.show_editors = false; }
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::SameLine();
            tmp = windowConfig.show_mixer;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(FontAwesomeIcons::FA_SLIDERS, ImVec2(30, 30))) { windowConfig.show_mixer ^= 1; windowConfig.show_editors = false; windowConfig.show_smartcontrols = false; }
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::SameLine();
            tmp = windowConfig.show_editors;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(FontAwesomeIcons::FA_SCISSORS, ImVec2(30, 30))) { windowConfig.show_editors ^= 1; windowConfig.show_smartcontrols = false; windowConfig.show_mixer = false; }
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::PopStyleVar();
        }
        ImGui::End();

        ImGui::Begin("TOOLBAR_CENTER", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
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

        ImGui::Begin("TOOLBAR_RIGHT", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetWindowPos(ImVec2(this->_display_w - 150, 22));
            ImGui::SetWindowSize(ImVec2(150, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5,15));

            ImGui::SameLine();
            bool tmp = windowConfig.show_listeditor;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(GoogleIcons::GI_LIST, ImVec2(30, 30)))
            {
                windowConfig.show_listeditor ^= 1;
                windowConfig.show_notes = false;
                windowConfig.show_loops = false;
                windowConfig.show_browser = false;
            }
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::SameLine();
            tmp = windowConfig.show_notes;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(GoogleIcons::GI_NOTE, ImVec2(30, 30)))
            {
                windowConfig.show_notes ^= 1;
                windowConfig.show_listeditor = false;
                windowConfig.show_loops = false;
                windowConfig.show_browser = false;
            }
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::SameLine();
            tmp = windowConfig.show_loops;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(GoogleIcons::GI_ALL_INCLUSIVE, ImVec2(30, 30)))
            {
                windowConfig.show_loops ^= 1;
                windowConfig.show_listeditor = false;
                windowConfig.show_notes = false;
                windowConfig.show_browser = false;
            }
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::SameLine();
            tmp = windowConfig.show_browser;
            if (tmp) ImGui::PushStyleColor(ImGuiCol_Button, checkedButtonColor);
            if (ImGui::Button(GoogleIcons::GI_LIBRARY_MUSIC, ImVec2(30, 30)))
            {
                windowConfig.show_browser ^= 1;
                windowConfig.show_listeditor = false;
                windowConfig.show_notes = false;
                windowConfig.show_loops = false;
            }
            if (tmp) ImGui::PopStyleColor(1);

            ImGui::PopStyleVar();
        }
        ImGui::End();

        if (windowConfig.show_library)
        {
            ImGui::Begin("library", &(windowConfig.show_library), ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            {
                ImGui::SetWindowPos(ImVec2(0, 62));
                ImGui::SetWindowSize(ImVec2(libraryWidth, this->_display_h));

                ImGui::Text("Library");
            }
            ImGui::End();
        }

        if (windowConfig.show_inspector)
        {
            ImGui::Begin("inspector", &(windowConfig.show_inspector), ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            {
                static bool region_mute = true;
                static bool region_loop = false;
                ImGui::SetWindowPos(ImVec2(windowConfig.show_library ? libraryWidth : 0, 62));
                ImGui::SetWindowSize(ImVec2(inspectorWidth, this->_display_h - 62));
                if (ImGui::CollapsingHeader("Region"))
                {
                    ImGui::Checkbox("Mute", &region_mute);
                    ImGui::Checkbox("Loop", &region_loop);
                }
                if (ImGui::CollapsingHeader("Groups"))
                {
                }
                if (ImGui::CollapsingHeader("Track"))
                {
                }

                ImGui::BeginChild("selected_track");
                {
                    ImGui::SetWindowPos(ImVec2((windowConfig.show_library ? libraryWidth : 0) + 15, this->_display_h - 70));
                    static float trackPan = 0.0f;
                    static bool trackMute = false;
                    static bool trackSolo = false;
                    if (ImGui::Button("Instrument")) windowConfig.show_library = true;
//                    Knob("Pan", &trackPan, -1.0f, 1.0f);
                    if (ImGui::Button("M", ImVec2(24, 24))) trackMute ^= 1;
                    ImGui::SameLine();
                    if (ImGui::Button("S", ImVec2(24, 24))) trackSolo ^= 1;
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }

        auto v = ImVec2((float)this->_display_w - (windowConfig.show_library ? libraryWidth : 0) - (windowConfig.show_inspector ? inspectorWidth : 0),
                        (float)this->_display_h - 62.0f);
        ImGui::Begin("Splitter test2", &open, ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoMove |  ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetWindowPos(ImVec2((windowConfig.show_library ? libraryWidth : 0) + (windowConfig.show_inspector ? inspectorWidth : 0), 62));
            ImGui::SetWindowSize(v);
            ImGui::SetNextWindowSizeConstraints(v, v);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

            ImGui::BeginChild("child1", ImVec2(0, (windowConfig.show_smartcontrols | windowConfig.show_editors | windowConfig.show_mixer ? windowConfig.h : 0)), true);
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

            if (windowConfig.show_smartcontrols | windowConfig.show_editors | windowConfig.show_mixer)
            {
                ImGui::InvisibleButton("hsplitter", ImVec2(-1,8.0f));
                if (ImGui::IsItemActive())
                    windowConfig.h += ImGui::GetIO().MouseDelta.y;

                ImGui::BeginChild("child2", ImVec2(0,0), true);
                {
                    if (windowConfig.show_smartcontrols)
                    {
                        ImGui::Text("Smart controls");
                    }
                    if (windowConfig.show_editors)
                    {
                        ImGui::Text("Editors");
                    }
                    if (windowConfig.show_mixer)
                    {
                        ImGui::Text("Mixer");
                    }
                }
                ImGui::EndChild();
            }

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
