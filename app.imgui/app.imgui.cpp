#include "app.imgui.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "font-icons.h"
#include <zyn.mixer/Instrument.h>
#include <algorithm>

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

#define UNUSED(a) (void)a
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a) / sizeof(a)[0])

struct
{
    bool show_library = false;
    bool show_inspector = true;
    bool show_smartcontrols = false;
    bool show_mixer = false;
    bool show_editors = false;
    bool show_listeditor = false;
    bool show_notes = false;
    bool show_loops = false;
    bool show_browser = false;
    float channel_height = 70.0f;
    float w = 200.0f;
    float h = 300.0f;
    float splitter1 = 255.0f;

} windowConfig;

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _window(window), _mixer(mixer)
{
    glfwGetWindowSize(window, &_display_w, &_display_h);
    glfwSetWindowUserPointer(this->_window, static_cast<void *>(this));
}

AppThreeDee::~AppThreeDee()
{
    glfwSetWindowUserPointer(this->_window, nullptr);
}

void AppThreeDee::KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<AppThreeDee *>(glfwGetWindowUserPointer(window));

    if (app != nullptr)
        app->onKeyAction(key, scancode, action, mods);
}

void AppThreeDee::ResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = static_cast<AppThreeDee *>(glfwGetWindowUserPointer(window));

    if (app != nullptr)
        app->onResize(width, height);
}

void AppThreeDee::onKeyAction(int  /*key*/, int  /*scancode*/, int  /*action*/, int  /*mods*/)
{
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

bool AppThreeDee::SetUp()
{
    return true;
}

// Implementing a simple custom widget using the public API.
// You may also use the <imgui_internal.h> API to get raw access to more data/helpers, however the internal API isn't guaranteed to be forward compatible.
// FIXME: Need at least proper label centering + clipping (internal functions RenderTextClipped provides both but api is flaky/temporary)
static bool MyKnob(const char *label, float *p_value, float v_min, float v_max)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    float radius_outer = 20.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);
    float line_height = ImGui::GetTextLineHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + line_height + style.ItemInnerSpacing.y));
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.x != 0.0f)
    {
        float step = (v_max - v_min) / 200.0f;
        *p_value += io.MouseDelta.x * step;
        if (*p_value < v_min)
            *p_value = v_min;
        if (*p_value > v_max)
            *p_value = v_max;
        value_changed = true;
    }

    float t = (*p_value - v_min) / (v_max - v_min);
    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * t;
    float angle_cos = cosf(angle), angle_sin = sinf(angle);
    float radius_inner = radius_outer * 0.40f;
    draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16);
    draw_list->AddLine(ImVec2(center.x + angle_cos * radius_inner, center.y + angle_sin * radius_inner), ImVec2(center.x + angle_cos * (radius_outer - 2), center.y + angle_sin * (radius_outer - 2)), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
    draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    draw_list->AddText(ImVec2(pos.x, pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - line_height - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        ImGui::Text("%.3f", *p_value);
        ImGui::EndTooltip();
    }

    return value_changed;
}

static bool MyKnobUchar(const char *label, unsigned char *p_value, unsigned char v_min, unsigned char v_max)
{
    float val = (p_value[0]) / 128.0f;

    if (MyKnob(label, &val, v_min / 128.0f, v_max / 128.0f))
    {
        p_value[0] = val * 128;

        return true;
    }

    return false;
}

#define CHANNEL_COUNT 16
const char *channels[] = {
    "Bass Drum",
    "Snare Drum",
    "Low Tom",
    "Mid Tom",
    "Hi Tom",
    "Rim Shot",
    "hand ClaP",
    "Cow Bell",
    "CYmbal",
    "Open Hihat",
    "Closed Hihat",
    "Low Conga",
    "Mid Conga",
    "Hi Conga",
    "CLaves",
    "MAracas",
};

static bool selectedChannel[CHANNEL_COUNT] { true, false };

bool show_channels_window = true;
bool show_another_window = true;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static bool selected[16] = {
    true, false, false, false,
    true, false, false, false,
    true, false, false, false,
    true, false, false, false,
};
static bool noteOn = false;
static float BPM = 140.0f;
static float speed = 1.0f / (BPM / 60.0f);
static double timePast = -0.01f;
static int step = 0;
static bool running = false;

void AppThreeDee::NoteOn()
{
    _mixer->Lock();
    _mixer->NoteOn(0, 60, 200);
    _mixer->Unlock();
    noteOn = true;
}

void AppThreeDee::NoteOff()
{
    _mixer->Lock();
    _mixer->NoteOff(0, 60);
    _mixer->Unlock();
    noteOn = false;
}

void AppThreeDee::Render(double dt)
{
    if (running)
    {
        timePast += dt;
        if (timePast > (speed / 4.0) && !noteOn)
        {
            timePast -= (speed / 4.0);
            step = (step+1) % 16;

            if (selected[step % 16])
            {
                NoteOn();
            }
        }
        else if (timePast > ((speed / 4.0)*0.8) && noteOn)
        {
            NoteOff();
        }
    }
    else
    {
        timePast = speed;
    }

    ImGui_ImplGlfwGL3_NewFrame();

    // 1. Show a simple window.
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
    {
        ImGui::Columns(2, nullptr, false);
        unsigned char vol = _mixer->Pvolume;
        if (MyKnobUchar("Master", &vol, 0, 128))
        {
            _mixer->setPvolume(vol);
        }
        ImGui::NextColumn();

        if (ImGui::Selectable("Play", &running, 0, ImVec2(50.0f, 50.0f)))
        {
            step = 0;
            NoteOff();
        }

        ImGui::Columns(1, nullptr, false);
        if (ImGui::SliderFloat("BPM", &BPM, 50.f, 200.f))
        {
            speed = 1.0f / (BPM / 60.0f);
        }
    }

    char label[32];
    if (show_channels_window)
    {
        ImGui::Begin("Channels Window", &show_channels_window);

        ImGui::Columns(CHANNEL_COUNT, nullptr, false);
        for (int i = 0; i < CHANNEL_COUNT; i++)
        {
            sprintf(label, "child%d", i);
            ImGui::BeginChild(label);

            sprintf(label, "LEVEL %d", i);
            unsigned char vol = _mixer->GetChannel(i)->Pvolume;
            if (MyKnobUchar(label, &vol, 0, 128))
            {
                _mixer->GetChannel(i)->Pvolume = vol;
            }

            if (i < 5 || i == 8 || i == 11 || i == 12 || i == 13)
            {
                if (i == 0 || i == 1 || i == 8)
                {
                    sprintf(label, "TONE %d", i);
                }
                else
                {
                    sprintf(label, "TUNING %d", i);
                }
                unsigned char vol = _mixer->GetChannel(i)->Pvolume;
                if (MyKnobUchar(label, &vol, 0, 128))
                {
                    _mixer->GetChannel(i)->Pvolume = vol;
                }
            }
            if (i < 2 || i == 8 || i == 9)
            {
                if (i == 1)
                {
                    sprintf(label, "SNAPPY %d", i);
                }
                else
                {
                    sprintf(label, "DECAY %d", i);
                }
                unsigned char vol = _mixer->GetChannel(i)->Pvolume;
                if (MyKnobUchar(label, &vol, 0, 128))
                {
                    _mixer->GetChannel(i)->Pvolume = vol;
                }
            }

            if (ImGui::Selectable(channels[i], &selectedChannel[i], 0))
            {
                for (int j = 0; j < CHANNEL_COUNT; ++j) if (j != i) selectedChannel[j] = false;
            }
            ImGui::EndChild();

            ImGui::NextColumn();
        }
        ImGui::Columns(1);

        ImGui::End();
    }

    // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name the window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);

        ImGui::Columns(16, nullptr, false);
        for (int i = 0; i < 16; i++)
        {
            ImGui::TextColored(i == step ? ImVec4(0, 1, 0, 1) : ImVec4(0, 0, 0, 1), "*");
            ImGui::NextColumn();
        }
        for (int i = 0; i < 16; i++)
        {
            sprintf(label, "%d", i);
            if (ImGui::Selectable(label, &selected[i], 0, ImVec2(30.0f, 50.0f)))
            {
            }
            ImGui::NextColumn();
        }
        ImGui::Columns(1);

        ImGui::End();
    }

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(_window, &_display_w, &_display_h);
    glViewport(0, 0, _display_w, _display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
}

void AppThreeDee::CleanUp()
{
}
