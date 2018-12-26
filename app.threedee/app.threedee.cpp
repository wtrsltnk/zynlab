#include "app.threedee.h"

#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_addons/imgui_checkbutton.h"
#include "imgui_addons/imgui_knob.h"
#include "stb_image.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>

static char const *const channels[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
};

static const char *notes[] = {
    "A",
    "A#",
    "B",
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
};

static char const *instrumentCategoryIconFileNames[] = {
    "default.png",              // 0
    "Piano.png",                // 1
    "Chromatic_Percussion.png", // 2
    "Organ.png",                // 3
    "Guitar.png",               // 4
    "Bass.png",                 // 5
    "Solo_Strings.png",         // 6
    "Ensemble.png",             // 7
    "Brass.png",                // 8
    "Reed.png",                 // 9
    "Pipe.png",                 // 10
    "Synth_Lead.png",           // 11
    "Synth_Pad.png",            // 12
    "Synth_Effects.png",        // 13
    "Ethnic.png",               // 14
    "Percussive.png",           // 15
    "Sound_Effects.png",        // 16
};

static ImVec4 clear_color = ImColor(90, 90, 100);

static int openSelectInstrument = -1;
static ImVec2 trackSize = ImVec2(150, 0);
static float sliderBaseHeight = 150.0f;
static float const largeModeTreshold = 4.5f;

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _mixer(mixer), _window(window), _stepper(&_sequencer, mixer),
      _iconImagesAreLoaded(false), _showInstrumentEditor(false), _showPatternEditor(false), _showMixer(true),
      _display_w(800), _display_h(600), _currentBank(0),
      _showADNoteEditor(true), _showSUBNoteEditor(true), _showPADNoteEditor(true)
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

    if (app != nullptr)
    {
        app->onKeyAction(key, scancode, action, mods);
        ImGui_ImplGlfw_KeyCallback(app->_window, key, scancode, action, mods);
    }
}

void AppThreeDee::ResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = static_cast<AppThreeDee *>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onResize(width, height);
}

void AppThreeDee::onResize(int width, int height)
{
    this->_display_w = width;
    this->_display_h = height;

    glViewport(0, 0, width, height);
}

bool AppThreeDee::Setup()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    io.ConfigDockingWithShift = false;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsClassic();

    _mixer->GetBankManager()->RescanForBanks();
    _mixer->GetBankManager()->LoadBank(_currentBank);

    _stepper.Setup();
    _sequencer.ActiveInstrument(0);

    LoadInstrumentIcons();

    return true;
}

void AppThreeDee::LoadInstrumentIcons()
{
    std::string rootDir = "./icons/Instruments/";

    _iconImagesAreLoaded = false;

    for (int i = 0; i < int(InstrumentCategories::COUNT); i++)
    {
        GLuint my_opengl_texture;
        glGenTextures(1, &my_opengl_texture);

        auto filename = rootDir + instrumentCategoryIconFileNames[i];
        int x, y, n;
        unsigned char *data = stbi_load(filename.c_str(), &x, &y, &n, 0);
        if (data == nullptr)
        {
            std::cout << "Failed to load instrument category " << i << " from file " << filename << std::endl;
            _iconImages[i] = 0;
            continue;
        }
        _iconImages[i] = my_opengl_texture;
        _iconImagesAreLoaded = true;

        glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, n, x, y, 0, n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
}

void AppThreeDee::HitNote(int trackIndex, int note, int durationInMs)
{
    _stepper.HitNote(static_cast<unsigned char>(trackIndex), static_cast<unsigned char>(note), durationInMs);
}

void AppThreeDee::EditSelectedPattern()
{
    _showPatternEditor = true;
}

std::vector<char const *> toCharVector(std::set<std::string> const &strings)
{
    std::vector<char const *> result;
    for (auto &s : strings)
    {
        result.push_back(s.c_str());
    }

    return result;
}

void AppThreeDee::ImGuiMixer()
{
    if (_showMixer)
    {
        ImGui::Begin("Mixer", &_showMixer, ImGuiWindowFlags_AlwaysHorizontalScrollbar);

        for (int track = 0; track <= NUM_MIXER_CHANNELS; track++)
        {
            auto highlightTrack = _sequencer.ActiveInstrument() == track;
            ImGui::PushID(track);
            ImGuiTrack(track, highlightTrack);
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::End();
    }
}

void AppThreeDee::ImGuiSelectedTrack()
{
    ImGui::Begin("Selected Track", nullptr, ImVec2(trackSize.x * 2, 0), -1.0f, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    {
        ImGuiMasterTrack();
        ImGui::SameLine();
        ImGuiTrack(_sequencer.ActiveInstrument(), false);
    }
    ImGui::End();
}

void AppThreeDee::ImGuiMasterTrack()
{
    auto io = ImGui::GetStyle();

    ImGui::BeginChild("Master Track", trackSize, true);
    {
        auto availableRegion = ImGui::GetContentRegionAvail();
        auto width = availableRegion.x;
        auto useLargeMode = availableRegion.y > sliderBaseHeight * largeModeTreshold;

        ImGui::TextCentered(ImVec2(width, 20), "master");

        // Output devices
        auto sinks = toCharVector(Nio::GetSinks());
        int selectedSink = 0;
        ImGui::PushItemWidth(width);
        if (ImGui::Combo("##Sinks", &selectedSink, &sinks[0], static_cast<int>(sinks.size())))
        {
            Nio::SelectSource(sinks[static_cast<size_t>(selectedSink)]);
        }
        ImGui::ShowTooltipOnHover("Ouput device");

        // Input devices
        auto sources = toCharVector(Nio::GetSources());
        int selectedSource = 0;
        ImGui::PushItemWidth(width);
        if (ImGui::Combo("##Sources", &selectedSource, &sources[0], static_cast<int>(sources.size())))
        {
            Nio::SelectSource(sources[static_cast<size_t>(selectedSource)]);
        }
        ImGui::ShowTooltipOnHover("Midi device");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Enable/disable NRPN
        auto nrpn = _mixer->ctl.NRPN.receive == 1;
        if (ImGui::Checkbox("##nrpn", &nrpn))
        {
            _mixer->ctl.NRPN.receive = nrpn ? 1 : 0;
        }
        ImGui::ShowTooltipOnHover("Receive NRPNs");
        ImGui::SameLine();
        ImGui::Text("NRPN");
        ImGui::ShowTooltipOnHover("Receive NRPNs");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Enable/disable Portamento
        auto portamento = _mixer->ctl.portamento.portamento == 1;
        if (ImGui::Checkbox("##portamento", &portamento))
        {
            _mixer->ctl.portamento.portamento = portamento ? 1 : 0;
        }
        ImGui::ShowTooltipOnHover("Enable/Disable the portamento");
        ImGui::SameLine();
        ImGui::Text("Portamento");
        ImGui::ShowTooltipOnHover("Enable/Disable the portamento");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // System effects
        if (useLargeMode)
        {
            ImGui::TextCentered(ImVec2(width, 20), "System FX");
        }
        for (int fx = 0; fx < NUM_SYS_EFX; fx++)
        {
            char fxButton[32] = {0};
            sprintf(fxButton, "sys fx %d", fx + 1);
            ImGui::Button(fxButton, ImVec2(width, 0));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Fine detune
        auto fineDetune = _mixer->microtonal.Pglobalfinedetune;
        if (ImGui::KnobUchar("fine detune", &fineDetune, 0, 128, ImVec2(width, 40)))
        {
            _mixer->microtonal.Pglobalfinedetune = fineDetune;
        }
        ImGui::ShowTooltipOnHover("Global fine detune");

        ImGui::Spacing();

        auto faderHeight = ImGui::GetWindowContentRegionMax().y - ImGui::GetCursorPos().y - io.ItemSpacing.y;

        // Master volume
        if (faderHeight < (40 + ImGui::GetTextLineHeight()))
        {
            auto v = _mixer->Pvolume;
            if (ImGui::KnobUchar("volume", &v, 0, 128, ImVec2(width, 40)))
            {
                _mixer->setPvolume(v);
            }
        }
        else
        {
            ImGui::Spacing();
            ImGui::SameLine(0.0f, (width - 20) / 2);

            int v = static_cast<int>(_mixer->Pvolume);
            if (ImGui::VSliderInt("##mastervol", ImVec2(20, faderHeight), &v, 0, 128))
            {
                _mixer->setPvolume(static_cast<unsigned char>(v));
            }
        }
        ImGui::ShowTooltipOnHover("Master volume");
    }
    ImGui::EndChild();
}

void AppThreeDee::ImGuiTrack(int track, bool highlightTrack)
{
    if (track < 0 || track >= NUM_MIXER_CHANNELS)
    {
        return;
    }

    auto channel = _mixer->GetChannel(track);

    if (channel == nullptr)
    {
        return;
    }

    auto io = ImGui::GetStyle();

    auto hue = track * 0.05f;
    if (highlightTrack)
    {
        ImGui::PushStyleColor(ImGuiCol_Border, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));
    }

    ImGui::BeginChild("Track", trackSize, true);
    {
        auto availableRegion = ImGui::GetContentRegionAvail();
        auto width = availableRegion.x;
        auto useLargeMode = availableRegion.y > sliderBaseHeight * largeModeTreshold;

        char tmp[32] = {0};
        sprintf(tmp, "track %d", track + 1);
        ImGui::TextCentered(ImVec2(width, 20), tmp);

        auto trackEnabled = channel->Penabled == 1;

        ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.0f, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.0f, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

        // Enable/disable channel
        if (ImGui::Checkbox("##trackEnabled", &trackEnabled))
        {
            _sequencer.ActiveInstrument(track);
            channel->Penabled = trackEnabled ? 1 : 0;
        }
        ImGui::ShowTooltipOnHover(trackEnabled ? "This track is enabled" : "This track is disabled");

        ImGui::SameLine();

        // Change channel presets
        auto name = std::string(reinterpret_cast<char *>(channel->Pname));
        if (ImGui::Button(name.size() == 0 ? "default" : name.c_str(), ImVec2(width - 20 - io.ItemSpacing.x, 0)))
        {
            _sequencer.ActiveInstrument(track);
            openSelectInstrument = track;
        }
        ImGui::ShowTooltipOnHover("Change instrument preset");

        ImGui::PopStyleColor(8);

        // Select midi channel
        if (useLargeMode)
        {
            ImGui::TextCentered(ImVec2(width, 20), "MIDI channel");
        }
        int midiChannel = static_cast<int>(channel->Prcvchn);
        ImGui::PushItemWidth(width);
        if (ImGui::Combo("##KeyboardChannel", &midiChannel, channels, NUM_MIXER_CHANNELS))
        {
            channel->Prcvchn = static_cast<unsigned char>(midiChannel);
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover("Midi channel");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // AD synth enable/disable + edit button
        auto adEnabled = channel->_instruments[0].Padenabled == 1;
        if (ImGui::Checkbox("##adEnabled", &adEnabled))
        {
            channel->_instruments[0].Padenabled = adEnabled ? 1 : 0;
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover(adEnabled ? "The AD synth is enabled" : "The AD synth is disabled");
        ImGui::SameLine();
        if (ImGui::Button("AD", ImVec2(width - 20 - io.ItemSpacing.x, 20)))
        {
            _showADNoteEditor = true;
            ImGui::SetWindowFocus(ADeditorID);
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover("Edit the AD synth");

        // SUB synth enable/disable + edit button
        auto subEnabled = channel->_instruments[0].Psubenabled == 1;
        if (ImGui::Checkbox("##subEnabled", &subEnabled))
        {
            channel->_instruments[0].Psubenabled = subEnabled ? 1 : 0;
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover(adEnabled ? "The SUB synth is enabled" : "The AD synth is disabled");
        ImGui::SameLine();
        if (ImGui::Button("SUB", ImVec2(width - 20 - io.ItemSpacing.x, 20)))
        {
            _showSUBNoteEditor = true;
            ImGui::SetWindowFocus(SUBeditorID);
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover("Edit the SUB synth");

        // PAD synth enable/disable + edit button
        auto padEnabled = channel->_instruments[0].Ppadenabled == 1;
        if (ImGui::Checkbox("##padEnabled", &padEnabled))
        {
            channel->_instruments[0].Ppadenabled = padEnabled ? 1 : 0;
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover(adEnabled ? "The PAD synth is enabled" : "The AD synth is disabled");
        ImGui::SameLine();
        if (ImGui::Button("PAD", ImVec2(width - 20 - io.ItemSpacing.x, 20)))
        {
            _showPADNoteEditor = true;
            ImGui::SetWindowFocus(PADeditorID);
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover("Edit the PAD synth");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (useLargeMode)
        {
            ImGui::TextCentered(ImVec2(width, 20), "Sys FX sends");
        }

        for (int send = 0; send < NUM_SYS_EFX; send++)
        {
            ImGui::PushID(send);
            auto send1 = static_cast<float>(_mixer->Psysefxvol[send][track]);
            if (ImGui::Knob("", &send1, 0, 128, ImVec2(width / 2, 40)))
            {
                _mixer->Psysefxvol[send][track] = static_cast<unsigned char>(send1);
                _sequencer.ActiveInstrument(track);
            }
            char tmp[64] = {'\0'};
            sprintf(tmp, "Volume for send to system effect %d", (send + 1));
            ImGui::ShowTooltipOnHover(tmp);
            if (send % 2 == 0)
            {
                ImGui::SameLine();
            }
            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (useLargeMode)
        {
            ImGui::TextCentered(ImVec2(width, 20), "Audio FX");
        }

        for (int fx = 0; fx < NUM_CHANNEL_EFX; fx++)
        {
            char fxButton[32] = {0};
            sprintf(fxButton, "fx %d", fx + 1);
            ImGui::Button(fxButton, ImVec2(width, 0));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (useLargeMode && _iconImagesAreLoaded)
        {
            ImGui::Spacing();
            ImGui::SameLine(0.0f, (width - 64) / 2);
            ImGui::Image(reinterpret_cast<void *>(_iconImages[channel->info.Ptype]), ImVec2(64, 64));

            ImGui::Text("%d", channel->info.Ptype);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        auto velsns = channel->Pvelsns;
        if (ImGui::KnobUchar("vel.sns.", &velsns, 0, 128, ImVec2(width / 2, 30)))
        {
            channel->Pvelsns = velsns;
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover("Velocity Sensing Function");

        ImGui::SameLine();

        auto velofs = channel->Pveloffs;
        if (ImGui::KnobUchar("vel.ofs.", &velofs, 0, 128, ImVec2(width / 2, 30)))
        {
            channel->Pveloffs = velofs;
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover("Velocity Offset");

        auto panning = channel->Ppanning;
        if (ImGui::KnobUchar("panning", &panning, 0, 128, ImVec2(width, 40)))
        {
            channel->setPpanning(panning);
            _sequencer.ActiveInstrument(track);
        }
        ImGui::ShowTooltipOnHover("Track panning");

        float peakl, peakr;
        channel->ComputePeakLeftAndRight(channel->Pvolume, peakl, peakr);

        auto start = ImGui::GetCursorPos();
        auto faderHeight = ImGui::GetWindowContentRegionMax().y - start.y - io.ItemSpacing.y;

        ImGui::Spacing();

        if (faderHeight < (40 + ImGui::GetTextLineHeight()))
        {
            auto v = channel->Pvolume;
            if (ImGui::KnobUchar("volume", &v, 0, 128, ImVec2(width, 40)))
            {
                channel->setPvolume(v);
                _sequencer.ActiveInstrument(track);
            }
        }
        else
        {
            ImGui::Spacing();
            ImGui::SameLine(0.0f, (width - 20) / 2);

            int v = static_cast<int>(channel->Pvolume);
            if (ImGui::VSliderInt("##vol", ImVec2(20, faderHeight), &v, 0, 128))
            {
                channel->setPvolume(static_cast<unsigned char>(v));
                _sequencer.ActiveInstrument(track);
            }
        }
        ImGui::ShowTooltipOnHover("Track volume");
    }
    ImGui::EndChild();

    if (highlightTrack)
    {
        ImGui::PopStyleColor();
    }
}

void AppThreeDee::ImGuiSelectInstrumentPopup()
{
    if (openSelectInstrument >= 0)
    {
        ImGui::OpenPopup("Select Instrument");
    }

    ImGui::SetNextWindowSize(ImVec2(700, 850));
    if (ImGui::BeginPopupModal("Select Instrument"))
    {
        auto count = _mixer->GetBankManager()->GetBankCount();
        auto const &bankNames = _mixer->GetBankManager()->GetBankNames();
        if (ImGui::Combo("Bank", &_currentBank, &(bankNames[0]), int(count)))
        {
            _mixer->GetBankManager()->LoadBank(_currentBank);
        }

        static bool autoClose = false;
        ImGui::SameLine();
        ImGui::Checkbox("Auto close", &autoClose);

        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            openSelectInstrument = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::BeginChild("banks", ImVec2(0, -20));
        ImGui::Columns(5);
        if (_currentBank >= 0)
        {
            for (unsigned int i = 0; i < BANK_SIZE; i++)
            {
                auto instrumentName = _mixer->GetBankManager()->GetName(i);

                if (ImGui::Button(instrumentName.c_str(), ImVec2(120, 20)))
                {
                    auto const &instrument = _mixer->GetChannel(_sequencer.ActiveInstrument());
                    instrument->Lock();
                    _mixer->GetBankManager()->LoadFromSlot(i, instrument);
                    instrument->Unlock();
                    instrument->ApplyParameters();
                    openSelectInstrument = -1;
                    if (autoClose)
                    {
                        ImGui::CloseCurrentPopup();
                    }
                }
                if ((i + 1) % 32 == 0)
                {
                    ImGui::NextColumn();
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

void AppThreeDee::ImGuiSequencer()
{
    ImGui::Begin("Sequencer");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));

        static int trackHeight = 30;
        ImGui::BeginChild("scrolling", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int trackIndex = 0; trackIndex < NUM_MIXER_CHANNELS; trackIndex++)
        {
            ImGui::PushID(trackIndex * 1100);
            auto trackEnabled = _mixer->GetChannel(trackIndex)->Penabled == 1;
            if (ImGui::Checkbox("##trackEnabled", &trackEnabled))
            {
                _mixer->GetChannel(trackIndex)->Penabled = trackEnabled ? 1 : 0;
            }
            ImGui::SameLine();

            float hue = trackIndex * 0.05f;
            char trackLabel[32] = {'\0'};
            sprintf(trackLabel, "%02d", trackIndex + 1);
            bool highLight = trackIndex == _sequencer.ActiveInstrument();
            if (highLight)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            }
            if (ImGui::Button(trackLabel, ImVec2(trackHeight, trackHeight)))
            {
                _sequencer.ActiveInstrument(trackIndex);
            }
            if (highLight)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();

            ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

            //if (_mixer->GetChannel(trackIndex)->Pkitmode != 0)
            {
                ImGuiStepSequencer(trackIndex, trackHeight);
            }
            //else
            {
                ImGuiPianoRollSequencer(trackIndex, trackHeight);
            }

            ImGui::PopStyleColor(3);
        }
        float scroll_y = ImGui::GetScrollY();
        ImGui::EndChild();

        ImGui::PopStyleVar(2);

        float scroll_x_delta = 0.0f;
        ImGui::SmallButton("<<");
        if (ImGui::IsItemActive()) scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f;
        ImGui::SameLine();
        ImGui::Text("Scroll from code");
        ImGui::SameLine();
        ImGui::SmallButton(">>");
        if (ImGui::IsItemActive()) scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f;
        if (scroll_x_delta != 0.0f)
        {
            ImGui::BeginChild("scrolling");
            ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
            ImGui::End();
        }

        ImGui::BeginChild("info");
        ImGui::SetScrollY(scroll_y);
        ImGui::EndChild();

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            //if (_sequencer.ActiveInstrument() >= 0 && _mixer->GetChannel(_sequencer.ActiveInstrument())->Pkitmode != 0)
            {
                ImGuiStepSequencerEventHandling();
            }
            //else
            {
                ImGuiPianoRollSequencerEventHandling();
            }
        }
    }
    ImGui::End();
}

void AppThreeDee::ImGuiStepSequencer(int trackIndex, float trackHeight)
{
    auto lastIndex = _sequencer.LastPatternIndex(trackIndex);
    for (int patternIndex = 0; patternIndex <= lastIndex; patternIndex++)
    {
        ImGui::SameLine();
        ImGui::PushID(patternIndex + trackIndex * 1000);
        bool isActive = trackIndex == _sequencer.ActiveInstrument() && patternIndex == _sequencer.ActivePattern();
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        }

        if (_sequencer.DoesPatternExistAtIndex(trackIndex, patternIndex))
        {
            auto &pattern = _sequencer.GetPattern(trackIndex, patternIndex);
            if (ImGui::Button(pattern._name.c_str(), ImVec2(120.0f, trackHeight)))
            {
                _sequencer.ActiveInstrument(trackIndex);
                _sequencer.ActivePattern(patternIndex);
            }
            if (ImGui::IsMouseDoubleClicked(0))
            {
                EditSelectedPattern();
            }
        }
        else if (_mixer->GetChannel(trackIndex)->Penabled)
        {
            if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
            {
                _sequencer.AddPattern(trackIndex, patternIndex, "");
            }
        }
        if (isActive)
        {
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(1);
        }

        ImGui::PopID();
    }

    if (_mixer->GetChannel(trackIndex)->Penabled)
    {
        ImGui::SameLine();
        ImGui::PushID((100 + trackIndex) * 2010);
        if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
        {
            _sequencer.AddPattern(trackIndex, lastIndex + 1, "");
        }
        ImGui::PopID();
    }
}

void AppThreeDee::ImGuiStepSequencerEventHandling()
{
    ImGuiIO &io = ImGui::GetIO();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
    {
        _sequencer.RemoveActivePattern();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _sequencer.MovePatternLeftForced();
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _sequencer.SwitchPatternLeft();
        }
        else
        {
            _sequencer.MovePatternLeftIfPossible();
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _sequencer.MovePatternRightForced();
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _sequencer.SwitchPatternRight();
        }
        else
        {
            _sequencer.MovePatternRightIfPossible();
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home)))
    {
        _sequencer.SelectFirstPatternInTrack();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End)))
    {
        _sequencer.SelectLastPatternInTrack();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
    {
        EditSelectedPattern();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
    {
        if (io.KeyShift)
        {
            _sequencer.SelectPreviousPattern();
        }
        else
        {
            _sequencer.SelectNextPattern();
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && io.KeyCtrl)
    {
        if (_sequencer.DoesPatternExistAtIndex(_sequencer.ActiveInstrument(), _sequencer.ActivePattern()))
        {
            auto pattern = _sequencer.GetPattern(_sequencer.ActiveInstrument(), _sequencer.ActivePattern());
            _clipboardPatterns.push_back(pattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && io.KeyCtrl)
    {
        if (!_clipboardPatterns.empty() && _sequencer.DoesPatternExistAtIndex(_sequencer.ActiveInstrument(), _sequencer.ActivePattern()))
        {
            _sequencer.SetPattern(_sequencer.ActiveInstrument(), _sequencer.ActivePattern(), _clipboardPatterns.back());
        }
    }
}

void AppThreeDee::ImGuiPianoRollSequencer(int /*trackIndex*/, float /*trackHeight*/)
{
}

void AppThreeDee::ImGuiPianoRollSequencerEventHandling()
{
}

void AppThreeDee::ImGuiPatternEditorWindow()
{
    const float noteLabelWidth = 50.0f;
    const float rowHeight = 20.0f;
    if (_showPatternEditor)
    {
        if (!_sequencer.DoesPatternExistAtIndex(_sequencer.ActiveInstrument(), _sequencer.ActivePattern()))
        {
            _sequencer.ActivePattern(-1);
            return;
        }

        auto &style = ImGui::GetStyle();
        auto &selectedPattern = _sequencer.GetPattern(_sequencer.ActiveInstrument(), _sequencer.ActivePattern());

        ImGui::Begin("Pattern editor", &_showPatternEditor);
        char tmp[256];
        strcpy(tmp, selectedPattern._name.c_str());
        if (ImGui::InputText("pattern name", tmp, 256))
        {
            selectedPattern._name = tmp;
        }

        ImGui::BeginChild("Notes");
        auto width = ImGui::GetWindowWidth() - noteLabelWidth - (style.ItemSpacing.x * 2) - style.ScrollbarSize;
        auto itemWidth = (width / 16) - (style.ItemSpacing.x);
        for (int i = 0; i < 88; i++)
        {
            if (i % 12 == 0)
            {
                ImGui::Separator();
            }
            ImGui::PushID(i);
            if (ImGui::Button(notes[i % 12], ImVec2(noteLabelWidth, rowHeight)))
            {
                HitNote(_sequencer.ActiveInstrument(), i, 200);
            }
            for (int j = 0; j < 16; j++)
            {
                ImGui::SameLine();
                ImGui::PushID(j);
                auto found = selectedPattern._notes.find(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j)));
                bool s = found != selectedPattern._notes.end();
                if (j % 4 == 0)
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_FrameBg]);
                }
                if (ImGui::CheckButton("##note", &s, ImVec2(itemWidth, rowHeight)))
                {
                    if (!s)
                    {
                        selectedPattern._notes.erase(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j)));
                    }
                    else
                    {
                        selectedPattern._notes.insert(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j)));
                    }
                    HitNote(_sequencer.ActiveInstrument(), i, 200);
                }
                ImGui::PopStyleColor();
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::End();
    }
}

void AppThreeDee::Tick()
{
    _stepper.Tick();
}

void AppThreeDee::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiIO &io = ImGui::GetIO();

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("TestDockspace", nullptr, window_flags);
    {
        ImGuiID dockspace_id = ImGui::GetID("ZynDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGuiSequencer();
    ImGuiPatternEditorWindow();
    ImGuiSelectedTrack();
    ImGuiSelectInstrumentPopup();
    ImGuiMixer();

    auto channel = _mixer->GetChannel(_sequencer.ActiveInstrument());
    if (channel != nullptr)
    {
        ADNoteEditor(channel->_instruments[0].adpars);
        SUBNoteEditor(channel->_instruments[0].subpars);
        PADNoteEditor(channel->_instruments[0].padpars);
    }

    ImGuiPlayback();

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
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppThreeDee::onKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
}

void AppThreeDee::ImGuiPlayback()
{
    ImGui::Begin("Playback");
    {
        if (ImGui::Button("Stop"))
        {
            _stepper.Stop();
        }

        ImGui::SameLine();

        if (_stepper.IsPlaying())
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_Button]);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
        }

        if (ImGui::Button("Play/Pause"))
        {
            _stepper.PlayPause();
        }

        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        auto bpm = _stepper.Bpm();
        if (ImGui::SliderInt("##BPM", &bpm, 10, 200, "BPM %d"))
        {
            _stepper.Bpm(bpm);
        }

        ImGui::SameLine();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0 / static_cast<double>(ImGui::GetIO().Framerate), static_cast<double>(ImGui::GetIO().Framerate));

        ImGui::End();
    }
}

void AppThreeDee::Cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
