#include "ADnoteGlobalParam.h"

ADnoteGlobalParam::ADnoteGlobalParam()
{
    FreqEnvelope = EnvelopeParams::ASRinit(0, 0, 64, 50, 64, 60);
    FreqLfo = new LFOParams(70, 0, 64, 0, 0, 0, 0, 0);

    AmpEnvelope = EnvelopeParams::ADSRinit_dB(64, 1, 0, 40, 127, 25);
    AmpLfo = new LFOParams(80, 0, 64, 0, 0, 0, 0, 1);

    GlobalFilter = new FilterParams(2, 94, 40);
    FilterEnvelope = EnvelopeParams::ADSRinit_filter(0, 1, 64, 40, 64, 70, 60, 64);
    FilterLfo = new LFOParams(80, 0, 64, 0, 0, 0, 0, 2);
    Reson = new Resonance();
}

ADnoteGlobalParam::~ADnoteGlobalParam()
{
    delete FreqEnvelope;
    delete FreqLfo;
    delete AmpEnvelope;
    delete AmpLfo;
    delete GlobalFilter;
    delete FilterEnvelope;
    delete FilterLfo;
    delete Reson;
}

void ADnoteGlobalParam::Defaults()
{
    /* Frequency Global Parameters */
    PStereo = 1;    //stereo
    PDetune = 8192; //zero
    PCoarseDetune = 0;
    PDetuneType = 1;
    FreqEnvelope->Defaults();
    FreqLfo->Defaults();
    PBandwidth = 64;

    /* Amplitude Global Parameters */
    PVolume = 90;
    PPanning = 64; //center
    PAmpVelocityScaleFunction = 64;
    AmpEnvelope->Defaults();
    AmpLfo->Defaults();
    PPunchStrength = 0;
    PPunchTime = 60;
    PPunchStretch = 64;
    PPunchVelocitySensing = 72;
    Hrandgrouping = 0;

    /* Filter Global Parameters*/
    PFilterVelocityScale = 64;
    PFilterVelocityScaleFunction = 64;
    GlobalFilter->Defaults();
    FilterEnvelope->Defaults();
    FilterLfo->Defaults();
    Reson->Defaults();
}

void ADnoteGlobalParam::Serialize(IPresetsSerializer *xml)
{
    xml->addparbool("stereo", PStereo);

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    xml->addpar("volume", PVolume);
    xml->addpar("panning", PPanning);
    xml->addpar("velocity_sensing", PAmpVelocityScaleFunction);
    xml->addpar("punch_strength", PPunchStrength);
    xml->addpar("punch_time", PPunchTime);
    xml->addpar("punch_stretch", PPunchStretch);
    xml->addpar("punch_velocity_sensing", PPunchVelocitySensing);
    xml->addpar("harmonic_randomness_grouping", Hrandgrouping);

    xml->beginbranch("AMPLITUDE_ENVELOPE");
    AmpEnvelope->Serialize(xml);
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_LFO");
    AmpLfo->Serialize(xml);
    xml->endbranch();
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    xml->addpar("detune", PDetune);

    xml->addpar("coarse_detune", PCoarseDetune);
    xml->addpar("detune_type", PDetuneType);

    xml->addpar("bandwidth", PBandwidth);

    xml->beginbranch("FREQUENCY_ENVELOPE");
    FreqEnvelope->Serialize(xml);
    xml->endbranch();

    xml->beginbranch("FREQUENCY_LFO");
    FreqLfo->Serialize(xml);
    xml->endbranch();
    xml->endbranch();

    xml->beginbranch("FILTER_PARAMETERS");
    xml->addpar("velocity_sensing_amplitude", PFilterVelocityScale);
    xml->addpar("velocity_sensing", PFilterVelocityScaleFunction);

    xml->beginbranch("FILTER");
    GlobalFilter->Serialize(xml);
    xml->endbranch();

    xml->beginbranch("FILTER_ENVELOPE");
    FilterEnvelope->Serialize(xml);
    xml->endbranch();

    xml->beginbranch("FILTER_LFO");
    FilterLfo->Serialize(xml);
    xml->endbranch();
    xml->endbranch();

    xml->beginbranch("RESONANCE");
    Reson->Serialize(xml);
    xml->endbranch();
}

void ADnoteGlobalParam::Deserialize(IPresetsSerializer *xml)
{
    PStereo = xml->getparbool("stereo", PStereo);

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        PVolume = xml->getpar127("volume", PVolume);
        PPanning = xml->getpar127("panning", PPanning);
        PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", PAmpVelocityScaleFunction);

        PPunchStrength = xml->getpar127("punch_strength", PPunchStrength);
        PPunchTime = xml->getpar127("punch_time", PPunchTime);
        PPunchStretch = xml->getpar127("punch_stretch", PPunchStretch);
        PPunchVelocitySensing = xml->getpar127("punch_velocity_sensing", PPunchVelocitySensing);
        Hrandgrouping = xml->getpar127("harmonic_randomness_grouping", Hrandgrouping);

        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            AmpEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        if (xml->enterbranch("AMPLITUDE_LFO"))
        {
            AmpLfo->Deserialize(xml);
            xml->exitbranch();
        }

        xml->exitbranch();
    }

    if (xml->enterbranch("FREQUENCY_PARAMETERS"))
    {
        PDetune = xml->getpar("detune", PDetune, 0, 16383);
        PCoarseDetune = xml->getpar("coarse_detune", PCoarseDetune, 0, 16383);
        PDetuneType = xml->getpar127("detune_type", PDetuneType);
        PBandwidth = xml->getpar127("bandwidth", PBandwidth);

        xml->enterbranch("FREQUENCY_ENVELOPE");
        FreqEnvelope->Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FREQUENCY_LFO");
        FreqLfo->Deserialize(xml);
        xml->exitbranch();

        xml->exitbranch();
    }

    if (xml->enterbranch("FILTER_PARAMETERS"))
    {
        PFilterVelocityScale = xml->getpar127("velocity_sensing_amplitude", PFilterVelocityScale);
        PFilterVelocityScaleFunction = xml->getpar127("velocity_sensing", PFilterVelocityScaleFunction);

        xml->enterbranch("FILTER");
        GlobalFilter->Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_ENVELOPE");
        FilterEnvelope->Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_LFO");
        FilterLfo->Deserialize(xml);
        xml->exitbranch();
        xml->exitbranch();
    }

    if (xml->enterbranch("RESONANCE"))
    {
        Reson->Deserialize(xml);
        xml->exitbranch();
    }
}
