#include "ui.mixer.h"

#include "../imgui_addons/imgui_checkbutton.h"
#include "../imgui_addons/imgui_knob.h"
#include "../stb_image.h"
#include <glad/glad.h>
#include <iostream>
#include <set>
#include <zyn.common/globals.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/Nio.h>
#include <zyn.serialization/SaveToFileSerializer.h>

zyn::ui::Mixer::Mixer(AppState *appstate)
    : _state(appstate), _dialogs(appstate), _iconImagesAreLoaded(false)
{}

zyn::ui::Mixer::~Mixer() = default;

bool zyn::ui::Mixer::Setup()
{
    LoadInstrumentIcons();

    _dialogs.Setup();

    return true;
}

void zyn::ui::Mixer::Render()
{
    ImGuiMixer();
//    ImGuiChangeInstrumentTypePopup();
}

#define MIN_DB (-48)

static ImVec2 trackSize = ImVec2(TRACK_WIDTH, 0);
static float sliderBaseHeight = 140.0f;
static float const largeModeTreshold = 4.5f;
static int mostInsertEffectsPerTrack = 0;

static char const *const trackNames[] = {
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

void zyn::ui::Mixer::LoadInstrumentIcons()
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

void zyn::ui::Mixer::ImGuiMixer()
{
    if (ImGui::BeginChild("Mixer", ImVec2(), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));

        int c[NUM_MIXER_TRACKS] = {0};
        mostInsertEffectsPerTrack = 0;
        for (int i = 0; i < NUM_INS_EFX; i++)
        {
            if (_state->_mixer->Pinsparts[i] == -1)
            {
                continue;
            }
            c[_state->_mixer->Pinsparts[i]] = c[_state->_mixer->Pinsparts[i]] + 1;
            if (c[_state->_mixer->Pinsparts[i]] > mostInsertEffectsPerTrack)
            {
                mostInsertEffectsPerTrack = c[_state->_mixer->Pinsparts[i]];
            }
        }

        ImGuiMasterTrack();
        ImGui::SameLine();

        for (int track = 0; track <= NUM_MIXER_TRACKS; track++)
        {
            auto highlightTrack = _state->_currentTrack == track;
            ImGui::PushID(track);
            ImGuiTrack(track, highlightTrack);
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
}

void zyn::ui::Mixer::RenderInspector()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
    ImGui::BeginChild("Inspector", ImVec2(INSPECTOR_WIDTH, 0));
    {
        if (ImGui::CollapsingHeader("Region"))
        {
            if (_state->_currentTrack >= 0 && _state->_currentTrack < NUM_MIXER_TRACKS)
            {
                if (_state->_regions.DoesRegionExist(_state->_currentTrack, _state->_currentPattern))
                {
                    ImGui::SliderInt("repeat", &_state->_regions.GetRegion(_state->_currentTrack, _state->_currentPattern).repeat, 0, 16);
                }
            }
            ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent eget nunc eu lectus auctor fermentum in in diam. Donec luctus laoreet tortor, ut placerat eros bibendum sed. Mauris rhoncus ipsum sit amet molestie feugiat. Mauris augue ante, tempus non viverra eu, ornare quis sapien. Fusce faucibus ornare libero vitae tincidunt. Nunc eget tellus mi. Phasellus nisi dui, rhoncus tincidunt placerat vitae, volutpat ut mi. Nullam vestibulum metus est, id vestibulum sem malesuada eu. ");
        }

        if (ImGui::CollapsingHeader("Track"))
        {
            if (ImGui::Button("Reset to defaults"))
            {
                _state->_mixer->GetTrack(_state->_currentTrack)->Defaults();
            }
            ImGui::InputText("Track name", reinterpret_cast<char *>(_state->_mixer->GetTrack(_state->_currentTrack)->Pname), TRACK_MAX_NAME_LEN);
            if (ImGui::Button("Save track to .xiz"))
            {
                _dialogs.SaveFileDialog("Select Save Track file");
            }
            if (_dialogs.RenderSaveFileDialog() == DialogResults::Ok)
            {
                SaveToFileSerializer()
                    .SaveTrack(_state->_mixer->GetTrack(_state->_currentTrack), _dialogs.GetSaveFileName());
                ImGui::CloseCurrentPopup();
            }
            ImGui::TextWrapped("Etiam vitae condimentum justo. Duis et orci diam. Morbi rhoncus finibus augue, eget auctor eros aliquet rhoncus. Etiam felis enim, fringilla tincidunt pulvinar nec, lacinia non nibh. In eget dui porttitor, commodo odio in, interdum neque. Quisque neque neque, finibus non gravida ac, porttitor non odio. Proin magna urna, finibus vitae erat id, pulvinar elementum sapien. Morbi luctus, ex at commodo mattis, libero enim vestibulum lectus, non ornare justo tellus non mi. Nulla dictum arcu eros, sed posuere purus ultricies vitae. ");
        }

        ImGuiMasterTrack();
        ImGui::SameLine();
        ImGuiTrack(_state->_currentTrack, false);
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::SameLine();
}

unsigned char indexOf(std::vector<char const *> const &values, std::string const &selectedValue)
{
    for (size_t i = 0; i < values.size(); i++)
    {
        if (std::string(values[i]) == selectedValue)
        {
            return static_cast<unsigned char>(i);
        }
    }
    return 0;
}

void zyn::ui::Mixer::ImGuiMasterTrack()
{
    auto io = ImGui::GetStyle();
    auto lineHeight = ImGui::GetTextLineHeightWithSpacing();

    auto availableRegion = ImGui::GetContentRegionAvail();
    auto useLargeMode = availableRegion.y > sliderBaseHeight * largeModeTreshold;

    ImGui::BeginChild("Master Track", trackSize);
    {
        auto availableRegion = ImGui::GetContentRegionAvail();
        auto width = availableRegion.x;

        if (ImGui::Button("STEREO OUT", ImVec2(width, 0)))
        {
        }

        // Output devices
        auto sinks = toCharVector(Nio::GetSinks());
        auto selectedSink = indexOf(sinks, Nio::GetSelectedSink());
        ImGui::PushItemWidth(width);
        if (ImGui::DropDown("##Sinks", selectedSink, &sinks[0], sinks.size(), "Ouput device"))
        {
            Nio::SelectSink(sinks[static_cast<size_t>(selectedSink)]);
        }

        // Input devices
        auto sources = toCharVector(Nio::GetSources());
        auto selectedSource = indexOf(sources, Nio::GetSelectedSource());
        ImGui::PushItemWidth(width);
        if (ImGui::DropDown("##Sources", selectedSource, &sources[0], sources.size(), "Midi device"))
        {
            Nio::SelectSource(sources[static_cast<size_t>(selectedSource)]);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 2));

        // Enable/disable NRPN
        auto nrpn = _state->_mixer->ctl.NRPN.receive == 1;
        if (ImGui::Checkbox("NRPN", &nrpn))
        {
            _state->_mixer->ctl.NRPN.receive = nrpn ? 1 : 0;
        }
        ImGui::ShowTooltipOnHover("Receive NRPNs");

        // Enable/disable Portamento
        auto portamento = _state->_mixer->ctl.portamento.portamento == 1;
        if (ImGui::Checkbox("Portamento", &portamento))
        {
            _state->_mixer->ctl.portamento.portamento = portamento ? 1 : 0;
        }
        ImGui::ShowTooltipOnHover("Enable/Disable the portamento");

        ImGui::PopStyleVar();

        ImGui::Spacing();

        // System effects
        if (useLargeMode || ImGui::CollapsingHeader("Sys FX"))
        {
            if (useLargeMode) ImGui::Text("Sys FX");

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
            for (int fx = 0; fx < NUM_SYS_EFX; fx++)
            {
                ImGui::PushID(fx);
                ImGui::PushStyleColor(ImGuiCol_Button, _state->_mixer->sysefx[fx].geteffect() == 0 ? ImVec4(0.5f, 0.5f, 0.5f, 0.2f) : io.Colors[ImGuiCol_Button]);

                if (ImGui::Button(EffectNames[_state->_mixer->sysefx[fx].geteffect()], ImVec2(width - (_state->_mixer->sysefx[fx].geteffect() == 0 ? 0 : lineHeight), 0)))
                {
                    _state->_currentSystemEffect = fx;
                    ImGui::SetWindowFocus(SystemFxEditorID);
                }
                ImGui::OpenPopupOnItemClick("MasterSystemEffectSelection", 0);
                if (ImGui::BeginPopupContextItem("MasterSystemEffectSelection"))
                {
                    for (unsigned int i = 0; i < EffectNameCount; i++)
                    {
                        if (ImGui::Selectable(EffectNames[i]))
                        {
                            _state->_mixer->sysefx[fx].changeeffect(i);
                            _state->_currentSystemEffect = fx;
                            ImGui::SetWindowFocus(SystemFxEditorID);
                        }
                    }
                    ImGui::PushItemWidth(-1);
                    ImGui::PopItemWidth();
                    ImGui::EndPopup();
                }

                if (_state->_mixer->sysefx[fx].geteffect() != 0)
                {
                    ImGui::SameLine();
                    if (ImGui::Button("x", ImVec2(lineHeight, 0)))
                    {
                        _state->_currentSystemEffect = fx;
                        _state->_mixer->sysefx[fx].changeeffect(0);
                    }
                    ImGui::ShowTooltipOnHover("Remove system effect");
                }
                ImGui::PopStyleColor(1);
                ImGui::PopID();
            }

            ImGui::PopStyleVar();

            ImGui::Spacing();
        }

        // Fine detune
        auto fineDetune = _state->_mixer->microtonal.Pglobalfinedetune;
        if (ImGui::KnobUchar("fine detune", &fineDetune, 0, 127, ImVec2(width, 40), "Global fine detune"))
        {
            _state->_mixer->microtonal.Pglobalfinedetune = fineDetune;
        }

        auto faderHeight = ImGui::GetWindowContentRegionMax().y - ImGui::GetCursorPos().y - io.ItemSpacing.y - 30;

        // Master volume
        if (faderHeight < (40 + ImGui::GetTextLineHeight()))
        {
            auto v = _state->_mixer->Pvolume;
            if (ImGui::KnobUchar("volume", &v, 0, 127, ImVec2(width, 40), "Master Volume"))
            {
                _state->_mixer->setPvolume(v);
            }
        }
        else
        {
            static float olddbl = 1.0e-12f;
            static float olddbr = 1.0e-12f;
            static float oldrmsdbl = 1.0e-12f;
            static float oldrmsdbr = 1.0e-12f;

            vuData data = _state->_mixer->GetMeter()->GetVuData();

            float dbl = rap2dB(data.outpeakl);
            float dbr = rap2dB(data.outpeakr);
            float rmsdbl = rap2dB(data.rmspeakl);
            float rmsdbr = rap2dB(data.rmspeakr);

            dbl = (MIN_DB - dbl) / MIN_DB;
            if (dbl < 0.0f)
                dbl = 0.0f;
            else if (dbl > 1.0f)
                dbl = 1.0f;

            dbr = (MIN_DB - dbr) / MIN_DB;
            if (dbr < 0.0f)
                dbr = 0.0f;
            else if (dbr > 1.0f)
                dbr = 1.0f;

            dbl = dbl * 0.4f + olddbl * 0.6f;
            dbr = dbr * 0.4f + olddbr * 0.6f;

            olddbl = dbl;
            olddbr = dbr;

            //compute RMS - start
            rmsdbl = (MIN_DB - rmsdbl) / MIN_DB;
            if (rmsdbl < 0.0f)
                rmsdbl = 0.0f;
            else if (rmsdbl > 1.0f)
                rmsdbl = 1.0f;

            rmsdbr = (MIN_DB - rmsdbr) / MIN_DB;
            if (rmsdbr < 0.0f)
                rmsdbr = 0.0f;
            else if (rmsdbr > 1.0f)
                rmsdbr = 1.0f;

            rmsdbl = rmsdbl * 0.4f + oldrmsdbl * 0.6f;
            rmsdbr = rmsdbr * 0.4f + oldrmsdbr * 0.6f;

            oldrmsdbl = rmsdbl;
            oldrmsdbr = rmsdbr;

            rmsdbl *= faderHeight;
            rmsdbr *= faderHeight;

            int irmsdbl = static_cast<int>(rmsdbl);
            int irmsdbr = static_cast<int>(rmsdbr);
            //compute RMS - end

            ImGui::Spacing();
            ImGui::SameLine(0.0f, (width - (60.0f + (4 * io.ItemSpacing.x))) / 2.0f);

            ImGui::UvMeter("##mastervoll", ImVec2(20, faderHeight), &irmsdbl, 0, static_cast<int>(faderHeight));
            ImGui::SameLine();

            ImGui::Spacing();
            ImGui::SameLine();
            int v = static_cast<int>(_state->_mixer->Pvolume);
            if (ImGui::Fader("##mastervol", ImVec2(20, faderHeight), &v, 0, 127, "%d", 100.0f / 127.0f))
            {
                _state->_mixer->setPvolume(static_cast<unsigned char>(v));
            }
            ImGui::SameLine();
            ImGui::Spacing();

            ImGui::SameLine();
            ImGui::UvMeter("##mastervolr", ImVec2(20, faderHeight), &irmsdbr, 0, static_cast<int>(faderHeight));
        }
        ImGui::ShowTooltipOnHover("Master volume");

        ImGui::TextCentered(ImVec2(width, 20), "stereo out");
    }
    ImGui::EndChild();
}

void zyn::ui::Mixer::ImGuiTrack(int trackIndex, bool highlightTrack)
{
    if (trackIndex < 0 || trackIndex >= NUM_MIXER_TRACKS)
    {
        return;
    }

    auto track = _state->_mixer->GetTrack(trackIndex);

    if (track == nullptr)
    {
        return;
    }

    auto io = ImGui::GetStyle();

    auto hue = trackIndex * 0.05f;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, highlightTrack ? 0.05f : 0.01f));

    std::stringstream trackTooltip;
    trackTooltip << "[" << instrumentCategoryNames[track->info.Ptype] << "]";
    if (track->Pname[0])
    {
        trackTooltip << " " << track->Pname;
    }
    if (track->info.Pauthor[0])
    {
        trackTooltip << " by " << track->info.Pauthor;
    }
    if (track->info.Pcomments[0])
    {
        trackTooltip << "\n---\n"
                     << track->info.Pcomments;
    }

    if (ImGui::BeginChild("MixerTrack", trackSize))
    {
        auto availableRegion = ImGui::GetContentRegionAvail();
        auto width = availableRegion.x;
        auto lineHeight = ImGui::GetTextLineHeight();

        auto useLargeMode = availableRegion.y > sliderBaseHeight * largeModeTreshold;
        auto noCollapsingHeader = availableRegion.y > sliderBaseHeight * largeModeTreshold + 11 * lineHeight;

        auto trackEnabled = track->Penabled == 1;

        ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.0f, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.0f, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

        // Enable/disable Track
        if (ImGui::Checkbox("##MixerTrackEnabled", &trackEnabled))
        {
            _state->_currentTrack = trackIndex;
            track->Penabled = trackEnabled ? 1 : 0;
        }
        ImGui::ShowTooltipOnHover(trackEnabled ? "This track is enabled" : "This track is disabled");

        ImGui::SameLine();

        // Change Track presets
        auto name = std::string(reinterpret_cast<char *>(track->Pname));
        if (ImGui::Button(name.size() == 0 ? "default" : name.c_str(), ImVec2(width - 20 - io.ItemSpacing.x, 0)))
        {
            _state->_currentTrack = trackIndex;
            ImGui::SetWindowFocus(LibraryID);
        }
        ImGui::ShowTooltipOnHover("Change Track preset");

        ImGui::PopStyleColor(8);

        if (useLargeMode)
        {
            ImGui::TextCentered(ImVec2(width, lineHeight), "Midi channel");
        }

        // Select midi channel
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
        ImGui::PushItemWidth(width);
        if (ImGui::DropDown("##MidiChannel", track->Prcvchn, trackNames, NUM_MIXER_TRACKS, "Midi channel"))
        {
            _state->_currentTrack = trackIndex;
        }
        if (ImGui::IsItemClicked())
        {
            _state->_currentTrack = trackIndex;
        }

        ImGui::PopStyleVar();

        ImGui::Spacing();

        if (noCollapsingHeader || ImGui::CollapsingHeader("Settings"))
        {
            if (noCollapsingHeader) ImGui::TextCentered(ImVec2(width, lineHeight), "Settings");

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2));
            {
                // AD synth enable/disable + edit button
                auto adEnabled = track->Instruments[0].Padenabled == 1;
                if (ImGui::Checkbox("##adEnabled", &adEnabled))
                {
                    track->Instruments[0].Padenabled = adEnabled ? 1 : 0;
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover(adEnabled ? "The AD synth is enabled" : "The AD synth is disabled");
                ImGui::SameLine();
                if (ImGui::Button("AD", ImVec2(width - lineHeight - io.ItemSpacing.x, 0)))
                {
                    ImGui::SetWindowFocus(AdSynthEditorID);
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover("Edit the AD synth");
            }

            {
                // SUB synth enable/disable + edit button
                auto subEnabled = track->Instruments[0].Psubenabled == 1;
                if (ImGui::Checkbox("##subEnabled", &subEnabled))
                {
                    track->Instruments[0].Psubenabled = subEnabled ? 1 : 0;
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover(subEnabled ? "The SUB synth is enabled" : "The SUB synth is disabled");
                ImGui::SameLine();
                if (ImGui::Button("SUB", ImVec2(width - lineHeight - io.ItemSpacing.x, 0)))
                {
                    ImGui::SetWindowFocus(SubSynthEditorID);
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover("Edit the SUB synth");
            }

            {
                // SMPL synth enable/disable + edit button
                auto smplEnabled = track->Instruments[0].Psmplenabled == 1;
                if (ImGui::Checkbox("##smplEnabled", &smplEnabled))
                {
                    track->Instruments[0].Psmplenabled = smplEnabled ? 1 : 0;
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover(smplEnabled ? "The SMPL synth is enabled" : "The SMPL synth is disabled");
                ImGui::SameLine();
                if (ImGui::Button("SMPL", ImVec2(width - lineHeight - io.ItemSpacing.x, 0)))
                {
                    ImGui::SetWindowFocus(SmplSynthEditorID);
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover("Edit the SMPL synth");
            }

            {
                // PAD synth enable/disable + edit button
                auto padEnabled = track->Instruments[0].Ppadenabled == 1;
                if (ImGui::Checkbox("##padEnabled", &padEnabled))
                {
                    track->Instruments[0].Ppadenabled = padEnabled ? 1 : 0;
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover(padEnabled ? "The PAD synth is enabled" : "The PAD synth is disabled");
                ImGui::SameLine();
                if (ImGui::Button("PAD", ImVec2(width - lineHeight - io.ItemSpacing.x, 0)))
                {
                    ImGui::SetWindowFocus(PadSynthEditorID);
                    _state->_currentTrack = trackIndex;
                }
                ImGui::ShowTooltipOnHover("Edit the PAD synth");
            }
            ImGui::PopStyleVar();

            ImGui::Spacing();
        }

        // System effect sends
        if (noCollapsingHeader || ImGui::CollapsingHeader("Sys FX sends"))
        {
            if (noCollapsingHeader) ImGui::TextCentered(ImVec2(width, lineHeight), "Sys FX sends");

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

            for (int fx = 0; fx < NUM_SYS_EFX; fx++)
            {
                ImGui::PushID(fx);
                ImGui::PushStyleColor(ImGuiCol_Button, _state->_mixer->sysefx[fx].geteffect() == 0 ? ImVec4(0.5f, 0.5f, 0.5f, 0.2f) : io.Colors[ImGuiCol_Button]);

                const float square_sz = ImGui::GetFrameHeight();
                char label[64] = {'\0'};
                sprintf(label, "##send_%d", fx);
                char tooltip[64] = {'\0'};
                sprintf(tooltip, "Volume for send to system effect %d", (fx + 1));
                if (ImGui::KnobUchar(label, &(_state->_mixer->Psysefxvol[fx][trackIndex]), 0, 127, ImVec2(square_sz + io.ItemInnerSpacing.x, square_sz), tooltip))
                {
                    _state->_mixer->setPsysefxvol(trackIndex, fx, _state->_mixer->Psysefxvol[fx][trackIndex]);
                }
                if (ImGui::IsItemClicked())
                {
                    _state->_currentTrack = trackIndex;
                }

                ImGui::SameLine();

                if (ImGui::Button(EffectNames[_state->_mixer->sysefx[fx].geteffect()], ImVec2(width - lineHeight - 1, 0)))
                {
                    _state->_currentTrack = trackIndex;
                    _state->_currentSystemEffect = fx;
                    ImGui::SetWindowFocus(SystemFxEditorID);
                }
                ImGui::OpenPopupOnItemClick("SystemEffectSelection", 0);
                if (ImGui::BeginPopupContextItem("SystemEffectSelection"))
                {
                    for (unsigned int i = 0; i < EffectNameCount; i++)
                    {
                        if (ImGui::Selectable(EffectNames[i]))
                        {
                            _state->_mixer->sysefx[fx].changeeffect(int(i));
                            _state->_currentTrack = trackIndex;
                            _state->_currentSystemEffect = fx;
                            ImGui::SetWindowFocus(SystemFxEditorID);
                        }
                    }
                    ImGui::PushItemWidth(-1);
                    ImGui::PopItemWidth();
                    ImGui::EndPopup();
                }

                ImGui::PopStyleColor(1);
                ImGui::PopID();
            }

            ImGui::PopStyleVar(1);

            ImGui::Spacing();
        }
        if (ImGui::IsItemClicked())
        {
            _state->_currentTrack = trackIndex;
        }

        // Insertion effects
        if (noCollapsingHeader || ImGui::CollapsingHeader("Insert FX"))
        {
            if (noCollapsingHeader) ImGui::TextCentered(ImVec2(width, lineHeight), "Insert FX");

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

            int fillCount = mostInsertEffectsPerTrack;
            for (int fx = 0; fx < NUM_INS_EFX; fx++)
            {
                ImGui::PushID(100 + fx);
                if (_state->_mixer->Pinsparts[fx] == trackIndex)
                {
                    if (ImGui::Button(EffectNames[_state->_mixer->insefx[fx].geteffect()], ImVec2(width - 22, 0)))
                    {
                        _state->_currentInsertEffect = fx;
                        _state->_currentTrack = trackIndex;
                        ImGui::SetWindowFocus(InsertionFxEditorID);
                    }
                    ImGui::OpenPopupOnItemClick("InsertEffectSelection", 0);
                    if (ImGui::BeginPopupContextItem("InsertEffectSelection"))
                    {
                        for (unsigned int i = 0; i < EffectNameCount; i++)
                        {
                            if (ImGui::Selectable(EffectNames[i]))
                            {
                                _state->_mixer->insefx[fx].changeeffect(int(i));
                                _state->_currentInsertEffect = fx;
                                _state->_currentTrack = trackIndex;
                                ImGui::SetWindowFocus(InsertionFxEditorID);
                            }
                        }
                        ImGui::PushItemWidth(-1);
                        ImGui::PopItemWidth();
                        ImGui::EndPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("x", ImVec2(20, 0)))
                    {
                        RemoveInsertFxFromTrack(fx);
                        _state->_currentTrack = trackIndex;
                    }
                    ImGui::ShowTooltipOnHover("Remove insert effect from track");
                    fillCount--;
                }
                ImGui::PopID();
            }
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            for (int i = 0; i < fillCount; i++)
            {
                ImGui::PushID(i);
                ImGui::Button("##empty", ImVec2(width, 0));
                ImGui::PopID();
            }
            ImGui::PopStyleColor(3);

            if (ImGui::Button("+", ImVec2(width, 0)))
            {
                AddInsertFx(trackIndex);
                _state->_currentTrack = trackIndex;
            }
            ImGui::ShowTooltipOnHover("Add insert effect to track");

            ImGui::PopStyleVar(1);

            ImGui::Spacing();
        }
        if (ImGui::IsItemClicked())
        {
            _state->_currentTrack = trackIndex;
        }

        // Track effects
        if (noCollapsingHeader || ImGui::CollapsingHeader("Audio FX"))
        {
            if (noCollapsingHeader) ImGui::TextCentered(ImVec2(width, lineHeight), "Audio FX");

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

            for (int fx = 0; fx < NUM_TRACK_EFX; fx++)
            {
                ImGui::PushID(200 + fx);
                ImGui::PushStyleColor(ImGuiCol_Button, track->partefx[fx]->geteffect() == 0 ? ImVec4(0.5f, 0.5f, 0.5f, 0.2f) : io.Colors[ImGuiCol_Button]);

                if (ImGui::Button(EffectNames[track->partefx[fx]->geteffect()], ImVec2(width - (track->partefx[fx]->geteffect() == 0 ? 0 : 22), 0)))
                {
                    _state->_currentTrack = trackIndex;
                    _state->_currentTrackEffect = fx;
                    ImGui::SetWindowFocus(TrackFxEditorID);
                }
                ImGui::OpenPopupOnItemClick("TrackEffectSelection", track->partefx[fx]->geteffect() == 0 ? 0 : 1);
                if (ImGui::BeginPopupContextItem("TrackEffectSelection"))
                {
                    for (unsigned int i = 0; i < EffectNameCount; i++)
                    {
                        if (ImGui::Selectable(EffectNames[i]))
                        {
                            track->partefx[fx]->changeeffect(int(i));
                            _state->_currentTrack = trackIndex;
                            _state->_currentTrackEffect = fx;
                            ImGui::SetWindowFocus(TrackFxEditorID);
                        }
                    }
                    ImGui::PushItemWidth(-1);
                    ImGui::PopItemWidth();
                    ImGui::EndPopup();
                }

                if (track->partefx[fx]->geteffect() != 0)
                {
                    ImGui::SameLine();
                    if (ImGui::Button("x", ImVec2(20, 0)))
                    {
                        _state->_currentTrack = trackIndex;
                        _state->_currentTrackEffect = fx;
                        track->partefx[fx]->changeeffect(0);
                    }
                    ImGui::ShowTooltipOnHover("Remove effect from track");
                }
                ImGui::PopStyleColor(1);
                ImGui::PopID();
            }
            ImGui::PopStyleVar(1);

            ImGui::Spacing();
        }
        if (ImGui::IsItemClicked())
        {
            _state->_currentTrack = trackIndex;
        }

        if (ImGui::CollapsingHeader("Velocity"))
        {
            if (ImGui::KnobUchar("vel.sns.", &track->Pvelsns, 0, 127, ImVec2(width / 2, 30), "Velocity Sensing Function"))
            {
                _state->_currentTrack = trackIndex;
            }

            ImGui::SameLine();

            if (ImGui::KnobUchar("vel.ofs.", &track->Pveloffs, 0, 127, ImVec2(width / 2, 30), "Velocity Offset"))
            {
                _state->_currentTrack = trackIndex;
            }
        }
        if (ImGui::IsItemClicked())
        {
            _state->_currentTrack = trackIndex;
        }

        if (useLargeMode && _iconImagesAreLoaded)
        {
            ImGui::Spacing();

            ImGui::SameLine(0.0f, (width - 64 - io.ItemSpacing.x) / 2);
            if (ImGui::ImageButton(reinterpret_cast<void *>(_iconImages[track->info.Ptype]), ImVec2(64, 64)))
            {
                _state->_showTrackTypeChanger = trackIndex;
                _state->_currentTrack = trackIndex;
            }
            ImGui::ShowTooltipOnHover(trackTooltip.str().c_str());
        }

        auto panning = track->Ppanning;
        if (ImGui::KnobUchar("panning", &panning, 0, 127, ImVec2(width, 40), "Track panning"))
        {
            track->setPpanning(panning);
            _state->_currentTrack = trackIndex;
        }
        if (ImGui::IsItemClicked())
        {
            _state->_currentTrack = trackIndex;
        }

        auto start = ImGui::GetCursorPos();
        auto faderHeight = ImGui::GetWindowContentRegionMax().y - start.y - io.ItemSpacing.y - 30;

        if (faderHeight < (40 + ImGui::GetTextLineHeight()))
        {
            auto v = track->Pvolume;
            if (ImGui::KnobUchar("volume", &v, 0, 127, ImVec2(width, 40), "Track volume"))
            {
                track->setPvolume(v);
                _state->_currentTrack = trackIndex;
            }
        }
        else
        {
            float db = rap2dB(_state->_mixer->GetMeter()->GetOutPeak(trackIndex));

            db = (MIN_DB - db) / MIN_DB;
            if (db < 0.0f)
                db = 0.0f;
            else if (db > 1.0f)
                db = 1.0f;

            db *= faderHeight;

            int idb = static_cast<int>(db);

            ImGui::Spacing();
            ImGui::SameLine(0.0f, (width - (40.0f + (3 * io.ItemSpacing.x))) / 2.0f);

            int v = static_cast<int>(track->Pvolume);
            if (ImGui::Fader("##vol", ImVec2(20, faderHeight), &v, 0, 127))
            {
                track->setPvolume(static_cast<unsigned char>(v));
                _state->_currentTrack = trackIndex;
            }
            ImGui::ShowTooltipOnHover("Track volume");
            ImGui::SameLine();
            ImGui::Spacing();

            ImGui::SameLine();
            ImGui::UvMeter("##instrument_uvr", ImVec2(20, faderHeight), &idb, 0, static_cast<int>(faderHeight));
        }

        char tmp[32] = {0};
        sprintf(tmp, "track %d", trackIndex + 1);
        ImGui::TextCentered(ImVec2(width, lineHeight), tmp);
        ImGui::ShowTooltipOnHover(trackTooltip.str().c_str());
    }
    ImGui::EndChild();
    if (ImGui::IsItemClicked())
    {
        _state->_currentTrack = trackIndex;
    }

    ImGui::PopStyleColor(1);
}

void zyn::ui::Mixer::ImGuiChangeInstrumentTypePopup()
{
    if (!_iconImagesAreLoaded)
    {
        return;
    }

    if (_state->_showTrackTypeChanger < 0)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(570, 500));
    if (ImGui::Begin("Select Instrument Type"))
    {
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            _state->_showTrackTypeChanger = -1;
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
                auto track = _state->_mixer->GetTrack(_state->_showTrackTypeChanger);
                if (track != nullptr)
                {
                    track->info.Ptype = static_cast<unsigned char>(i);
                    _state->_showTrackTypeChanger = -1;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::ShowTooltipOnHover(instrumentCategoryNames[i]);
        }
        ImGui::EndChild();
        ImGui::End();
    }
}

void zyn::ui::Mixer::AddInsertFx(int track)
{
    for (int i = 0; i < NUM_INS_EFX; i++)
    {
        if (_state->_mixer->Pinsparts[i] == -1)
        {
            _state->_mixer->Pinsparts[i] = static_cast<short>(track);
            return;
        }
    }
}

void zyn::ui::Mixer::RemoveInsertFxFromTrack(int fx)
{
    _state->_mixer->Pinsparts[fx] = -1;
}
