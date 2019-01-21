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

void ADnoteVoiceParam::Serialize(IPresetsSerializer *xml, bool fmoscilused)
{
    xml->addpar("type", Type);

    xml->addpar("unison_size", Unison_size);
    xml->addpar("unison_frequency_spread", Unison_frequency_spread);
    xml->addpar("unison_stereo_spread", Unison_stereo_spread);
    xml->addpar("unison_vibratto", Unison_vibratto);
    xml->addpar("unison_vibratto_speed", Unison_vibratto_speed);
    xml->addpar("unison_invert_phase", Unison_invert_phase);
    xml->addpar("unison_phase_randomness", Unison_phase_randomness);

    xml->addpar("delay", PDelay);
    xml->addparbool("resonance", Presonance);

    xml->addpar("ext_oscil", Pextoscil);
    xml->addpar("ext_fm_oscil", PextFMoscil);

    xml->addpar("oscil_phase", Poscilphase);
    xml->addpar("oscil_fm_phase", PFMoscilphase);

    xml->addparbool("filter_enabled", PFilterEnabled);
    xml->addparbool("filter_bypass", Pfilterbypass);

    xml->addpar("fm_enabled", PFMEnabled);

    xml->beginbranch("OSCIL");
    OscilSmp->Serialize(xml);
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    xml->addpar("panning", PPanning);
    xml->addpar("volume", PVolume);
    xml->addparbool("volume_minus", PVolumeminus);
    xml->addpar("velocity_sensing", PAmpVelocityScaleFunction);

    xml->addparbool("amp_envelope_enabled", PAmpEnvelopeEnabled);
    if ((PAmpEnvelopeEnabled != 0) || (!xml->minimal))
    {
        xml->beginbranch("AMPLITUDE_ENVELOPE");
        AmpEnvelope->Serialize(xml);
        xml->endbranch();
    }
    xml->addparbool("amp_lfo_enabled", PAmpLfoEnabled);
    if ((PAmpLfoEnabled != 0) || (!xml->minimal))
    {
        xml->beginbranch("AMPLITUDE_LFO");
        AmpLfo->Serialize(xml);
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    xml->addparbool("fixed_freq", Pfixedfreq);
    xml->addpar("fixed_freq_et", PfixedfreqET);
    xml->addpar("detune", PDetune);
    xml->addpar("coarse_detune", PCoarseDetune);
    xml->addpar("detune_type", PDetuneType);

    xml->addparbool("freq_envelope_enabled", PFreqEnvelopeEnabled);
    if ((PFreqEnvelopeEnabled != 0) || (!xml->minimal))
    {
        xml->beginbranch("FREQUENCY_ENVELOPE");
        FreqEnvelope->Serialize(xml);
        xml->endbranch();
    }
    xml->addparbool("freq_lfo_enabled", PFreqLfoEnabled);
    if ((PFreqLfoEnabled != 0) || (!xml->minimal))
    {
        xml->beginbranch("FREQUENCY_LFO");
        FreqLfo->Serialize(xml);
        xml->endbranch();
    }
    xml->endbranch();

    if ((PFilterEnabled != 0) || (!xml->minimal))
    {
        xml->beginbranch("FILTER_PARAMETERS");
        xml->beginbranch("FILTER");
        VoiceFilter->Serialize(xml);
        xml->endbranch();

        xml->addparbool("filter_envelope_enabled", PFilterEnvelopeEnabled);
        if ((PFilterEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FILTER_ENVELOPE");
            FilterEnvelope->Serialize(xml);
            xml->endbranch();
        }

        xml->addparbool("filter_lfo_enabled", PFilterLfoEnabled);
        if ((PFilterLfoEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FILTER_LFO");
            FilterLfo->Serialize(xml);
            xml->endbranch();
        }
        xml->endbranch();
    }

    if ((PFMEnabled != 0) || fmoscilused || (!xml->minimal))
    {
        xml->beginbranch("FM_PARAMETERS");
        xml->addpar("input_voice", PFMVoice);

        xml->addpar("volume", PFMVolume);
        xml->addpar("volume_damp", PFMVolumeDamp);
        xml->addpar("velocity_sensing", PFMVelocityScaleFunction);

        xml->addparbool("amp_envelope_enabled", PFMAmpEnvelopeEnabled);
        if ((PFMAmpEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("AMPLITUDE_ENVELOPE");
            FMAmpEnvelope->Serialize(xml);
            xml->endbranch();
        }
        xml->beginbranch("MODULATOR");
        xml->addpar("detune", PFMDetune);
        xml->addpar("coarse_detune", PFMCoarseDetune);
        xml->addpar("detune_type", PFMDetuneType);

        xml->addparbool("freq_envelope_enabled", PFMFreqEnvelopeEnabled);
        if ((PFMFreqEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FREQUENCY_ENVELOPE");
            FMFreqEnvelope->Serialize(xml);
            xml->endbranch();
        }

        xml->beginbranch("OSCIL");
        FMSmp->Serialize(xml);
        xml->endbranch();

        xml->endbranch();
        xml->endbranch();
    }
}

void ADnoteVoiceParam::Deserialize(IPresetsSerializer *xml, unsigned nvoice)
{
    Enabled = xml->getparbool("enabled", 0);
    Unison_size = xml->getpar127("unison_size", Unison_size);
    Unison_frequency_spread = xml->getpar127("unison_frequency_spread", Unison_frequency_spread);
    Unison_stereo_spread = xml->getpar127("unison_stereo_spread", Unison_stereo_spread);
    Unison_vibratto = xml->getpar127("unison_vibratto", Unison_vibratto);
    Unison_vibratto_speed = xml->getpar127("unison_vibratto_speed", Unison_vibratto_speed);
    Unison_invert_phase = xml->getpar127("unison_invert_phase", Unison_invert_phase);
    Unison_phase_randomness = xml->getpar127("unison_phase_randomness", Unison_phase_randomness);

    Type = xml->getpar127("type", Type);
    PDelay = xml->getpar127("delay", PDelay);
    Presonance = xml->getparbool("resonance", Presonance);

    Pextoscil = xml->getpar("ext_oscil", -1, -1, nvoice - 1);
    PextFMoscil = xml->getpar("ext_fm_oscil", -1, -1, nvoice - 1);

    Poscilphase = xml->getpar127("oscil_phase", Poscilphase);
    PFMoscilphase = xml->getpar127("oscil_fm_phase", PFMoscilphase);
    PFilterEnabled = xml->getparbool("filter_enabled", PFilterEnabled);
    Pfilterbypass = xml->getparbool("filter_bypass", Pfilterbypass);
    PFMEnabled = xml->getpar127("fm_enabled", PFMEnabled);

    if (xml->enterbranch("OSCIL"))
    {
        OscilSmp->Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        PPanning = xml->getpar127("panning", PPanning);
        PVolume = xml->getpar127("volume", PVolume);
        PVolumeminus = xml->getparbool("volume_minus", PVolumeminus);
        PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", PAmpVelocityScaleFunction);

        PAmpEnvelopeEnabled = xml->getparbool("amp_envelope_enabled", PAmpEnvelopeEnabled);
        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            AmpEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        PAmpLfoEnabled = xml->getparbool("amp_lfo_enabled", PAmpLfoEnabled);
        if (xml->enterbranch("AMPLITUDE_LFO"))
        {
            AmpLfo->Deserialize(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("FREQUENCY_PARAMETERS"))
    {
        Pfixedfreq = xml->getparbool("fixed_freq", Pfixedfreq);
        PfixedfreqET = xml->getpar127("fixed_freq_et", PfixedfreqET);
        PDetune = xml->getpar("detune", PDetune, 0, 16383);
        PCoarseDetune = xml->getpar("coarse_detune", PCoarseDetune, 0, 16383);
        PDetuneType = xml->getpar127("detune_type", PDetuneType);
        PFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled", PFreqEnvelopeEnabled);

        if (xml->enterbranch("FREQUENCY_ENVELOPE"))
        {
            FreqEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        PFreqLfoEnabled = xml->getparbool("freq_lfo_enabled", PFreqLfoEnabled);

        if (xml->enterbranch("FREQUENCY_LFO"))
        {
            FreqLfo->Deserialize(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("FILTER_PARAMETERS"))
    {
        if (xml->enterbranch("FILTER"))
        {
            VoiceFilter->Deserialize(xml);
            xml->exitbranch();
        }

        PFilterEnvelopeEnabled = xml->getparbool("filter_envelope_enabled", PFilterEnvelopeEnabled);
        if (xml->enterbranch("FILTER_ENVELOPE"))
        {
            FilterEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        PFilterLfoEnabled = xml->getparbool("filter_lfo_enabled", PFilterLfoEnabled);
        if (xml->enterbranch("FILTER_LFO"))
        {
            FilterLfo->Deserialize(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("FM_PARAMETERS"))
    {
        PFMVoice = xml->getpar("input_voice", PFMVoice, -1, nvoice - 1);
        PFMVolume = xml->getpar127("volume", PFMVolume);
        PFMVolumeDamp = xml->getpar127("volume_damp", PFMVolumeDamp);
        PFMVelocityScaleFunction = xml->getpar127("velocity_sensing", PFMVelocityScaleFunction);

        PFMAmpEnvelopeEnabled = xml->getparbool("amp_envelope_enabled", PFMAmpEnvelopeEnabled);
        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            FMAmpEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        if (xml->enterbranch("MODULATOR"))
        {
            PFMDetune = xml->getpar("detune", PFMDetune, 0, 16383);
            PFMCoarseDetune = xml->getpar("coarse_detune", PFMCoarseDetune, 0, 16383);
            PFMDetuneType = xml->getpar127("detune_type", PFMDetuneType);

            PFMFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled", PFMFreqEnvelopeEnabled);
            if (xml->enterbranch("FREQUENCY_ENVELOPE"))
            {
                FMFreqEnvelope->Deserialize(xml);
                xml->exitbranch();
            }

            if (xml->enterbranch("OSCIL"))
            {
                FMSmp->Deserialize(xml);
                xml->exitbranch();
            }

            xml->exitbranch();
        }
        xml->exitbranch();
    }
}
