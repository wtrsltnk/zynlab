#include "ADnoteVoiceParam.h"

void ADnoteVoiceParam::Enable(IFFTwrapper *fft, Resonance *Reson)
{
    OscilSmp = new OscilGen(fft, Reson);
    FMSmp = new OscilGen(fft, nullptr);

    AmpEnvelope = EnvelopeParams::ADSRinit_dB(64, 1, 0, 100, 127, 100);
    AmpLfo = new LFOParams(90, 32, 64, 0, 0, 30, 0, 1);

    FreqEnvelope = EnvelopeParams::ASRinit(0, 0, 30, 40, 64, 60);
    FreqLfo = new LFOParams(50, 40, 0, 0, 0, 0, 0, 0);

    VoiceFilter = new FilterParams(2, 50, 60);
    FilterEnvelope = EnvelopeParams::ADSRinit_filter(0, 0, 90, 70, 40, 70, 10, 40);
    FilterLfo = new LFOParams(50, 20, 64, 0, 0, 0, 0, 2);

    FMFreqEnvelope = EnvelopeParams::ASRinit(0, 0, 20, 90, 40, 80);
    FMAmpEnvelope = EnvelopeParams::ADSRinit(64, 1, 80, 90, 127, 100);
}

void ADnoteVoiceParam::Disable()
{
    delete OscilSmp;
    delete FMSmp;

    delete AmpEnvelope;
    delete AmpLfo;

    delete FreqEnvelope;
    delete FreqLfo;

    delete VoiceFilter;
    delete FilterEnvelope;
    delete FilterLfo;

    delete FMFreqEnvelope;
    delete FMAmpEnvelope;
}

void ADnoteVoiceParam::Defaults()
{
    Enabled = 0;

    Unison_size = 1;
    Unison_frequency_spread = 60;
    Unison_stereo_spread = 64;
    Unison_vibratto = 64;
    Unison_vibratto_speed = 64;
    Unison_invert_phase = 0;
    Unison_phase_randomness = 127;

    Type = 0;
    Pfixedfreq = 0;
    PfixedfreqET = 0;
    Presonance = 1;
    Pfilterbypass = 0;
    Pextoscil = -1;
    PextFMoscil = -1;
    Poscilphase = 64;
    PFMoscilphase = 64;
    PDelay = 0;
    PVolume = 100;
    PVolumeminus = 0;
    PPanning = 64;  //center
    PDetune = 8192; //8192=0
    PCoarseDetune = 0;
    PDetuneType = 0;
    PFreqLfoEnabled = 0;
    PFreqEnvelopeEnabled = 0;
    PAmpEnvelopeEnabled = 0;
    PAmpLfoEnabled = 0;
    PAmpVelocityScaleFunction = 127;
    PFilterEnabled = 0;
    PFilterEnvelopeEnabled = 0;
    PFilterLfoEnabled = 0;
    PFMEnabled = 0;

    //I use the internal oscillator (-1)
    PFMVoice = -1;

    PFMVolume = 90;
    PFMVolumeDamp = 64;
    PFMDetune = 8192;
    PFMCoarseDetune = 0;
    PFMDetuneType = 0;
    PFMFreqEnvelopeEnabled = 0;
    PFMAmpEnvelopeEnabled = 0;
    PFMVelocityScaleFunction = 64;

    OscilSmp->Defaults();
    FMSmp->Defaults();

    AmpEnvelope->Defaults();
    AmpLfo->Defaults();

    FreqEnvelope->Defaults();
    FreqLfo->Defaults();

    VoiceFilter->Defaults();
    FilterEnvelope->Defaults();
    FilterLfo->Defaults();

    FMFreqEnvelope->Defaults();
    FMAmpEnvelope->Defaults();
}

void ADnoteVoiceParam::InitPresets()
{
    _presets.clear();

    AddPresetAsBool("enabled", &Enabled);

    AddPreset("type", &Type);

    AddPreset("unison_size", &Unison_size);
    AddPreset("unison_frequency_spread", &Unison_frequency_spread);
    AddPreset("unison_stereo_spread", &Unison_stereo_spread);
    AddPreset("unison_vibratto", &Unison_vibratto);
    AddPreset("unison_vibratto_speed", &Unison_vibratto_speed);
    AddPreset("unison_invert_phase", &Unison_invert_phase);
    AddPreset("unison_phase_randomness", &Unison_phase_randomness);

    AddPreset("delay", &PDelay);
    AddPresetAsBool("resonance", &Presonance);

    AddPreset("ext_oscil", &Pextoscil);
    AddPreset("ext_fm_oscil", &PextFMoscil);

    AddPreset("oscil_phase", &Poscilphase);
    AddPreset("oscil_fm_phase", &PFMoscilphase);

    AddPresetAsBool("filter_enabled", &PFilterEnabled);
    AddPresetAsBool("filter_bypass", &Pfilterbypass);

    AddPreset("fm_enabled", &PFMEnabled);

    OscilSmp->InitPresets();
    AddContainer(Preset("OSCIL", *OscilSmp));

    Preset amplitudeParameters("AMPLITUDE_PARAMETERS");
    {
        amplitudeParameters.AddPreset("panning", &PPanning);
        amplitudeParameters.AddPreset("volume", &PVolume);
        amplitudeParameters.AddPresetAsBool("volume_minus", &PVolumeminus);
        amplitudeParameters.AddPreset("velocity_sensing", &PAmpVelocityScaleFunction);

        amplitudeParameters.AddPresetAsBool("amp_envelope_enabled", &PAmpEnvelopeEnabled);
        AmpEnvelope->InitPresets();
        amplitudeParameters.AddContainer(Preset("AMPLITUDE_ENVELOPE", *AmpEnvelope));

        amplitudeParameters.AddPresetAsBool("amp_lfo_enabled", &PAmpLfoEnabled);
        AmpLfo->InitPresets();
        amplitudeParameters.AddContainer(Preset("AMPLITUDE_LFO", *AmpLfo));
    }
    AddContainer(amplitudeParameters);

    Preset frequencyParameters("FREQUENCY_PARAMETERS");
    {
        frequencyParameters.AddPresetAsBool("fixed_freq", &Pfixedfreq);
        frequencyParameters.AddPreset("fixed_freq_et", &PfixedfreqET);
        frequencyParameters.AddPreset("detune", &PDetune);
        frequencyParameters.AddPreset("coarse_detune", &PCoarseDetune);
        frequencyParameters.AddPreset("detune_type", &PDetuneType);

        frequencyParameters.AddPresetAsBool("freq_envelope_enabled", &PFreqEnvelopeEnabled);
        FreqEnvelope->InitPresets();
        frequencyParameters.AddContainer(Preset("FREQUENCY_ENVELOPE", *FreqEnvelope));

        frequencyParameters.AddPresetAsBool("freq_lfo_enabled", &PFreqLfoEnabled);
        FreqLfo->InitPresets();
        frequencyParameters.AddContainer(Preset("FREQUENCY_LFO", *FreqLfo));
    }
    AddContainer(frequencyParameters);

    Preset filterParameters("FILTER_PARAMETERS");
    {
        VoiceFilter->InitPresets();
        filterParameters.AddContainer(Preset("FILTER", *VoiceFilter));

        filterParameters.AddPresetAsBool("filter_envelope_enabled", &PFilterEnvelopeEnabled);
        FilterEnvelope->InitPresets();
        filterParameters.AddContainer(Preset("FILTER_ENVELOPE", *FilterEnvelope));

        filterParameters.AddPresetAsBool("filter_lfo_enabled", &PFilterLfoEnabled);
        FilterLfo->InitPresets();
        filterParameters.AddContainer(Preset("FILTER_LFO", *FilterLfo));
    }
    AddContainer(filterParameters);

    Preset fmParameters("FM_PARAMETERS");
    {
        fmParameters.AddPreset("input_voice", &PFMVoice);

        fmParameters.AddPreset("volume", &PFMVolume);
        fmParameters.AddPreset("volume_damp", &PFMVolumeDamp);
        fmParameters.AddPreset("velocity_sensing", &PFMVelocityScaleFunction);

        fmParameters.AddPresetAsBool("amp_envelope_enabled", &PFMAmpEnvelopeEnabled);
        FMAmpEnvelope->InitPresets();
        fmParameters.AddContainer(Preset("AMPLITUDE_ENVELOPE", *FMAmpEnvelope));

        Preset modulator("MODULATOR");
        {
            modulator.AddPreset("detune", &PFMDetune);
            modulator.AddPreset("coarse_detune", &PFMCoarseDetune);
            modulator.AddPreset("detune_type", &PFMDetuneType);

            modulator.AddPresetAsBool("freq_envelope_enabled", &PFMFreqEnvelopeEnabled);
            FMFreqEnvelope->InitPresets();
            modulator.AddContainer(Preset("FREQUENCY_ENVELOPE", *FMFreqEnvelope));

            FMSmp->InitPresets();
            modulator.AddContainer(Preset("OSCIL", *FMSmp));
        }
        fmParameters.AddContainer(modulator);
    }
    AddContainer(fmParameters);
}
