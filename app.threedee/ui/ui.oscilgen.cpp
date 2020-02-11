#include "ui.oscilgen.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.mixer/Mixer.h>
#include <zyn.mixer/Track.h>
#include <zyn.synth/ADnoteParams.h>

char const *const WaveShapingFunctions[] = {
    "None",
    "Atan",
    "Asym1",
    "Pow",
    "Sine",
    "Qnts",
    "Zigzg",
    "Lmt",
    "LmtU",
    "LmtL",
    "ILmt",
    "Clip",
    "Asym2",
    "Pow2",
    "Sgm",
};
unsigned int WaveShapingFunctionCount = 15;

char const *const OscilFilterTypes[] = {
    "None",
    "LP1",
    "HP1a",
    "HP1b",
    "BP1",
    "BS1",
    "LP2",
    "HP2",
    "BP2",
    "BS2",
    "Cos",
    "Sin",
    "LSh",
    "S",
};
unsigned int OscilFilterTypeCount = 14;

char const *const ModulationTypes[] = {
    "None",
    "Rev",
    "Sine",
    "Pow",
};
unsigned int ModulationTypeCount = 4;

char const *const SpectrumAdjustmentTypes[] = {
    "None",
    "Pow",
    "ThrsD",
    "ThrsU",
};
unsigned int SpectrumAdjustmentTypeCount = 4;

char const *const AddaptiveHarmonicsTypes[] = {
    "OFF",
    "ON",
    "Square",
    "2xSub",
    "2xAdd",
    "3xSub",
    "3xAdd",
    "4xSub",
    "4xAdd",
};
unsigned int AddaptiveHarmonicsTypeCount = 9;

zyn::ui::OscilGen::OscilGen(AppState *state)
    : _state(state)
{}

zyn::ui::OscilGen::~OscilGen() = default;

bool zyn::ui::OscilGen::Setup()
{
    return true;
}

void zyn::ui::OscilGen::Render()
{
    auto track = _state->_mixer->GetTrack(_state->_currentTrack);

    ImGui::Text("Oscillator");

    if (ImGui::BeginChild(OscillatorEditorID, ImVec2(0,0), true) && track != nullptr && _state->_currentVoiceOscil >= 0)
    {
        auto oscil = track->Instruments[0].adpars->VoicePar[_state->_currentVoiceOscil].OscilSmp;

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 200);
        {
            ImGui::Text("Waveshaping");

            ImGui::PushItemWidth(175);
            ImGui::DropDown("##Waveshaping function", oscil->Pwaveshapingfunction, WaveShapingFunctions, WaveShapingFunctionCount, "Waveshaping function");

            ImGui::KnobUchar("##Waveshaping Parameter", &oscil->Pwaveshaping, 0, 127, ImVec2(30, 30), "Waveshaping Parameter");
        }

        {
            ImGui::Text("Oscillator's filter");

            ImGui::PushItemWidth(175);
            ImGui::DropDown("##OscillatorsFilter", oscil->Pfiltertype, OscilFilterTypes, OscilFilterTypeCount, "Oscillator's filter type");

            ImGui::KnobUchar("##OscillatorsFilterParameter2", &oscil->Pfilterpar1, 0, 127, ImVec2(30, 30), "Oscillator's filter parameter 1");

            ImGui::SameLine();

            ImGui::KnobUchar("##OscillatorsFilterParameter1", &oscil->Pfilterpar2, 0, 127, ImVec2(30, 30), "Oscillator's filter parameter 2");
        }

        {
            ImGui::Text("Modulation");

            ImGui::PushItemWidth(175);
            ImGui::DropDown("##Modulation", oscil->Pmodulation, ModulationTypes, ModulationTypeCount, "Modulation");

            ImGui::KnobUchar("##Modulation Parameter1", &oscil->Pmodulationpar1, 0, 127, ImVec2(30, 30), "Modulation Parameter 1");

            ImGui::SameLine();

            ImGui::KnobUchar("##Modulation Parameter2", &oscil->Pmodulationpar2, 0, 127, ImVec2(30, 30), "Modulation Parameter 2");

            ImGui::SameLine();

            ImGui::KnobUchar("##Modulation Parameter3", &oscil->Pmodulationpar3, 0, 127, ImVec2(30, 30), "Modulation Parameter 3");
        }

        {
            ImGui::Text("Oscillator's spectrum adjust");

            ImGui::PushItemWidth(175);
            ImGui::DropDown("##OscillatorSpectrum", oscil->Psatype, SpectrumAdjustmentTypes, SpectrumAdjustmentTypeCount, "Oscillator's spectrum adjust");

            ImGui::KnobUchar("##Oscillator Spectrum Parameter", &oscil->Psapar, 0, 127, ImVec2(30, 30), "Oscillator's spectrum adjust Parameter");
        }

        {
            ImGui::Text("Addaptive Harmonics");

            ImGui::PushItemWidth(175);
            ImGui::DropDown("##AddaptiveHarmonics", oscil->Padaptiveharmonics, AddaptiveHarmonicsTypes, AddaptiveHarmonicsTypeCount, "Addaptive Harmonics");

            ImGui::KnobUchar("##AdaptiveHarmonicsPower", &oscil->Padaptiveharmonicspower, 0, 200, ImVec2(30, 30), "Adaptive harmonics power");

            ImGui::SameLine();

            ImGui::KnobUchar("##AdaptiveHarmonicsBaseFrequency", &oscil->Padaptiveharmonicsbasefreq, 0, 255, ImVec2(30, 30), "Adaptive harmonics base frequency");

            ImGui::SameLine();

            ImGui::KnobUchar("##AdaptiveHarmonicsParameter", &oscil->Padaptiveharmonicspar, 0, 50, ImVec2(30, 30), "Adaptive harmonics parameter");
        }

        ImGui::NextColumn();

        std::vector<float> spc(175);
        oscil->getspectrum(175, spc.data(), 1);
        float max = 0;
        for (size_t i = 0; i < 175; i++)
        {
            if (max < spc[i])
                max = spc[i];
        }
        ImGui::PlotHistogram("##spectrum", spc.data(), 175, 0, nullptr, 0.0f, std::max(1.0f, max), ImVec2(350, 50));

        std::vector<float> smps(SystemSettings::Instance().oscilsize);
        oscil->get(smps.data(), -1.0);
        ImGui::PlotLines("##oscillator", smps.data(), static_cast<int>(SystemSettings::Instance().oscilsize), 0, nullptr, -1.1f, 1.1f, ImVec2(350, 240));

        ImGui::Columns(1);
    }
    ImGui::EndChild();
}
