#include "../app.threedee.h"

#include "../imgui_addons/imgui_checkbutton.h"
#include "../imgui_addons/imgui_knob.h"
#include "../stb_image.h"

static ImVec2 trackSize = ImVec2(150, 0);
static float sliderBaseHeight = 150.0f;
static float const largeModeTreshold = 4.5f;
static int mostInsertEffectsPerChannel = 0;

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

static char const *instrumentCategoryNames[] = {
    "Default",              // 0
    "Piano",                // 1
    "Chromatic Percussion", // 2
    "Organ",                // 3
    "Guitar",               // 4
    "Bass",                 // 5
    "Solo Strings",         // 6
    "Ensemble",             // 7
    "Brass",                // 8
    "Reed",                 // 9
    "Pipe",                 // 10
    "Synth Lead",           // 11
    "Synth Pad",            // 12
    "Synth Effects",        // 13
    "Ethnic",               // 14
    "Percussive",           // 15
    "Sound Effects",        // 16
};

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

        int c[NUM_MIXER_CHANNELS] = {0};
        mostInsertEffectsPerChannel = 0;
        for (int i = 0; i < NUM_INS_EFX; i++)
        {
            if (_mixer->Pinsparts[i] == -1)
            {
                continue;
            }
            c[_mixer->Pinsparts[i]] = c[_mixer->Pinsparts[i]] + 1;
            if (c[_mixer->Pinsparts[i]] > mostInsertEffectsPerChannel)
            {
                mostInsertEffectsPerChannel = c[_mixer->Pinsparts[i]];
            }
        }

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
            ImGui::PushID(fx);
            ImGui::PushStyleColor(ImGuiCol_Button, _mixer->sysefx[fx].geteffect() == 0 ? ImVec4(0.5f, 0.5f, 0.5f, 0.2f) : io.Colors[ImGuiCol_Button]);
            if (ImGui::Button(effectNames[_mixer->sysefx[fx].geteffect()], ImVec2(width, 0)))
            {
                _currentSystemEffect = fx;
                ImGui::SetWindowFocus(SystemFxEditorID);
            }
            ImGui::PopStyleColor(1);
            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Spacing();
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
            _openSelectInstrument = track;
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

        // System effect sends
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
                _mixer->setPsysefxvol(send, track, static_cast<unsigned char>(send1));
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

        // Insertion effects
        if (useLargeMode)
        {
            ImGui::TextCentered(ImVec2(width, 20), "Insert FX");
        }
        int fillCount = mostInsertEffectsPerChannel;
        for (int fx = 0; fx < NUM_INS_EFX; fx++)
        {
            ImGui::PushID(fx);
            if (_mixer->Pinsparts[fx] == track)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, io.ItemSpacing.y));
                if (ImGui::Button(effectNames[_mixer->insefx[fx].geteffect()], ImVec2(width - 22, 20)))
                {
                    _currentInsertEffect = fx;
                    _sequencer.ActiveInstrument(track);
                    ImGui::SetWindowFocus(InsertionFxEditorID);
                }
                ImGui::SameLine();
                if (ImGui::Button("x", ImVec2(20, 20)))
                {
                    RemoveInsertFxFromTrack(fx);
                }
                ImGui::ShowTooltipOnHover("Remove insert effect from track");
                fillCount--;
                ImGui::PopStyleVar(1);
            }
            ImGui::PopID();
        }
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        for (int i = 0; i < fillCount; i++)
        {
            ImGui::PushID(i);
            ImGui::Button("##empty", ImVec2(width, 20));
            ImGui::PopID();
        }
        ImGui::PopStyleColor(3);

        if (ImGui::Button("+", ImVec2(width, 20)))
        {
            AddInsertFx(track);
        }
        ImGui::ShowTooltipOnHover("Add insert effect to track");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Channel effects
        if (useLargeMode)
        {
            ImGui::TextCentered(ImVec2(width, 20), "Audio FX");
        }

        for (int fx = 0; fx < NUM_CHANNEL_EFX; fx++)
        {
            ImGui::PushID(fx);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, io.ItemSpacing.y));
            ImGui::PushStyleColor(ImGuiCol_Button, channel->partefx[fx]->geteffect() == 0 ? ImVec4(0.5f, 0.5f, 0.5f, 0.2f) : io.Colors[ImGuiCol_Button]);
            if (ImGui::Button(effectNames[channel->partefx[fx]->geteffect()], ImVec2(width - (channel->partefx[fx]->geteffect() == 0 ? 0 : 22), 20)))
            {
                _sequencer.ActiveInstrument(track);
                _currentInstrumentEffect = fx;
                ImGui::SetWindowFocus(InstrumentFxEditorID);
            }
            if (channel->partefx[fx]->geteffect() != 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("x", ImVec2(20, 20)))
                {
                    _sequencer.ActiveInstrument(track);
                    _currentInstrumentEffect = fx;
                    channel->partefx[fx]->changeeffect(0);
                }
                ImGui::ShowTooltipOnHover("Remove effect from track");
            }
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(1);
            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (useLargeMode && _iconImagesAreLoaded)
        {
            ImGui::Spacing();
            ImGui::SameLine(0.0f, (width - 64) / 2);
            if (ImGui::ImageButton(reinterpret_cast<void *>(_iconImages[channel->info.Ptype]), ImVec2(64, 64)))
            {
                _openChangeInstrumentType = track;
                _sequencer.ActiveInstrument(track);
            }
            ImGui::ShowTooltipOnHover(instrumentCategoryNames[channel->info.Ptype]);

            ImGui::Spacing();
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
    if (_openSelectInstrument >= 0)
    {
        ImGui::OpenPopup("Select Instrument");
    }

    ImGui::SetNextWindowSize(ImVec2(900, 850));
    if (ImGui::BeginPopupModal("Select Instrument"))
    {
        static bool autoClose = false;
        ImGui::SameLine();
        ImGui::Checkbox("Auto close", &autoClose);

        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            _openSelectInstrument = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 200);
        ImGui::SetColumnWidth(1, 700);
        auto count = _mixer->GetBankManager()->GetBankCount();
        auto const &bankNames = _mixer->GetBankManager()->GetBankNames();
        for (int i = 0; i < count; i++)
        {
            bool selected = _currentBank == i;
            if (ImGui::Selectable(bankNames[static_cast<size_t>(i)], &selected))
            {
                _currentBank = i;
                _mixer->GetBankManager()->LoadBank(_currentBank);
            }
        }
        ImGui::NextColumn();

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
                    _openSelectInstrument = -1;
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

void AppThreeDee::ImGuiChangeInstrumentTypePopup()
{
    if (!_iconImagesAreLoaded)
    {
        return;
    }

    if (_openChangeInstrumentType >= 0)
    {
        ImGui::OpenPopup("Select Instrument Type");
    }

    ImGui::SetNextWindowSize(ImVec2(570, 500));
    if (ImGui::BeginPopupModal("Select Instrument Type"))
    {
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            _openChangeInstrumentType = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::BeginChild("types", ImVec2(0, -20));

        for (int i = 0; i < int(InstrumentCategories::COUNT); i++)
        {
            if (i % 5 != 0)
            {
                ImGui::SameLine();
            }
            if (ImGui::ImageButton(reinterpret_cast<void *>(_iconImages[i]), ImVec2(96, 96)))
            {
                auto channel = _mixer->GetChannel(_openChangeInstrumentType);
                if (channel != nullptr)
                {
                    channel->info.Ptype = static_cast<unsigned char>(i);
                    _openChangeInstrumentType = -1;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::ShowTooltipOnHover(instrumentCategoryNames[i]);
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

void AppThreeDee::AddInsertFx(int track)
{
    for (int i = 0; i < NUM_INS_EFX; i++)
    {
        if (_mixer->Pinsparts[i] == -1)
        {
            _mixer->Pinsparts[i] = static_cast<short>(track);
            return;
        }
    }
}

void AppThreeDee::RemoveInsertFxFromTrack(int fx)
{
    _mixer->Pinsparts[fx] = -1;
}
