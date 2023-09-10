#include "par.h"

#include <regex>
#include <zyn.mixer/Track.h>
#include <zyn.synth/ADnoteParams.h>

bool operator==(
    const Par &p1,
    const Par &p2)
{
    return p1.id == p2.id;
}

bool operator!=(
    const Par &p1,
    const Par &p2)
{
    return !(p1 == p2);
}

bool CheckId(
    const char *id,
    const std::string &relativeid,
    const char *checkid,
    unsigned char *value,
    Par &p)
{
    if (relativeid == checkid)
    {
        p.id = id;

        p.ucvalue = value;
        p.type = ParTypes::UnsignedChar;

        return true;
    }

    return false;
}

bool CheckId(
    const char *id,
    const std::string &relativeid,
    const char *checkid,
    unsigned short *value,
    Par &p)
{
    if (relativeid == checkid)
    {
        p.id = id;

        p.usvalue = value;
        p.type = ParTypes::UnsignedShort;

        return true;
    }

    return false;
}

bool CheckId(
    const char *id,
    const std::string &relativeid,
    const char *checkid,
    float *value,
    Par &p)
{
    if (relativeid == checkid)
    {
        p.id = id;

        p.fvalue = value;
        p.type = ParTypes::Float;

        return true;
    }

    return false;
}

bool CheckId(
    const char *id,
    const std::string &relativeid,
    const char *checkid,
    bool *value,
    Par &p)
{
    if (relativeid == checkid)
    {
        p.id = id;

        p.bvalue = value;
        p.type = ParTypes::Bool;

        return true;
    }

    return false;
}

unsigned char Par::AsUnsignedChar() const
{
    if (type == ParTypes::UnsignedChar && ucvalue != nullptr)
    {
        return *ucvalue;
    }

    throw;
}

unsigned short Par::AsUnsignedShort() const
{
    if (type == ParTypes::UnsignedShort && usvalue != nullptr)
    {
        return *usvalue;
    }

    throw;
}

float Par::AsUnsignedFloat() const
{
    if (type == ParTypes::Float && fvalue != nullptr)
    {
        return *fvalue;
    }

    throw;
}

bool Par::AsUnsignedBool() const
{
    if (type == ParTypes::Bool && bvalue != nullptr)
    {
        return *bvalue;
    }

    throw;
}

Par Par::Empty()
{
    static Par i;

    return i;
}

Par Par::GetPar(
    FilterParams *pars,
    const char *id,
    const char *relativeid)
{
    if (pars == nullptr)
    {
        return Empty();
    }

    return Empty();
}

Par Par::GetPar(
    LFOParams *pars,
    const char *id,
    const char *relativeid)
{
    if (pars == nullptr)
    {
        return Empty();
    }

    return Empty();
}

Par Par::GetPar(
    EnvelopeParams *pars,
    const char *id,
    const char *relativeid)
{
    if (pars == nullptr)
    {
        return Empty();
    }

    return Empty();
}

Par Par::GetPar(
    Resonance *pars,
    const char *id,
    const char *relativeid)
{
    if (pars == nullptr)
    {
        return Empty();
    }

    return Empty();
}

Par Par::GetPar(
    ADnoteParameters *pars,
    const char *id,
    const char *relativeid)
{
    if (pars == nullptr)
    {
        return Empty();
    }

    if (std::string(relativeid).substr(0, sizeof("/FREQUENCY_PARAMETERS/FREQUENCY_ENVELOPE")) == "/FREQUENCY_PARAMETERS/FREQUENCY_ENVELOPE")
    {
        return GetPar(pars->FreqEnvelope, id, relativeid + sizeof("/FREQUENCY_PARAMETERS/FREQUENCY_ENVELOPE"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/FREQUENCY_PARAMETERS/FREQUENCY_LFO")) == "/FREQUENCY_PARAMETERS/FREQUENCY_LFO")
    {
        return GetPar(pars->FreqLfo, id, relativeid + sizeof("/FREQUENCY_PARAMETERS/FREQUENCY_LFO"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/AMPLITUDE_PARAMETERS/AMPLITUDE_ENVELOPE")) == "/AMPLITUDE_PARAMETERS/AMPLITUDE_ENVELOPE")
    {
        return GetPar(pars->AmpEnvelope, id, relativeid + sizeof("/AMPLITUDE_PARAMETERS/AMPLITUDE_ENVELOPE"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/AMPLITUDE_PARAMETERS/AMPLITUDE_LFO")) == "/AMPLITUDE_PARAMETERS/AMPLITUDE_LFO")
    {
        return GetPar(pars->AmpLfo, id, relativeid + sizeof("/AMPLITUDE_PARAMETERS/AMPLITUDE_LFO"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/FILTER_PARAMETERS/FILTER")) == "/FILTER_PARAMETERS/FILTER")
    {
        return GetPar(pars->GlobalFilter, id, relativeid + sizeof("/FILTER_PARAMETERS/FILTER"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/FILTER_PARAMETERS/FILTER_ENVELOPE")) == "/FILTER_PARAMETERS/FILTER_ENVELOPE")
    {
        return GetPar(pars->FilterEnvelope, id, relativeid + sizeof("/FILTER_PARAMETERS/FILTER_ENVELOPE"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/FILTER_PARAMETERS/FILTER_LFO")) == "/FILTER_PARAMETERS/FILTER_LFO")
    {
        return GetPar(pars->FilterLfo, id, relativeid + sizeof("/FILTER_PARAMETERS/FILTER_LFO"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/RESONANCE")) == "/RESONANCE")
    {
        return GetPar(pars->Reson, id, relativeid + sizeof("/RESONANCE"));
    }

    Par p;

    if (CheckId(id, relativeid, "/FREQUENCY_PARAMETERS/detune", &(pars->PDetune), p)) return p;
    if (CheckId(id, relativeid, "/FREQUENCY_PARAMETERS/coarse_detune", &(pars->PCoarseDetune), p)) return p;
    if (CheckId(id, relativeid, "/FREQUENCY_PARAMETERS/detune_type", &(pars->PDetuneType), p)) return p;
    if (CheckId(id, relativeid, "/FREQUENCY_PARAMETERS/bandwidth", &(pars->PBandwidth), p)) return p;

    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/volume", &(pars->PVolume), p)) return p;
    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/panning", &(pars->PPanning), p)) return p;
    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/velocity_sensing", &(pars->PAmpVelocityScaleFunction), p)) return p;
    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/punch_strength", &(pars->PPunchStrength), p)) return p;
    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/punch_time", &(pars->PPunchTime), p)) return p;
    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/punch_stretch", &(pars->PPunchStretch), p)) return p;
    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/punch_velocity_sensing", &(pars->PPunchVelocitySensing), p)) return p;
    if (CheckId(id, relativeid, "/AMPLITUDE_PARAMETERS/harmonic_randomness_grouping", &(pars->Hrandgrouping), p)) return p;

    if (CheckId(id, relativeid, "/FILTER_PARAMETERS/velocity_sensing_amplitude", &(pars->PFilterVelocityScale), p)) return p;
    if (CheckId(id, relativeid, "/FILTER_PARAMETERS/velocity_sensing", &(pars->PFilterVelocityScaleFunction), p)) return p;

    return Empty();
}

Par Par::GetPar(
    ADnoteVoiceParam *pars,
    const char *id,
    const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(
    SUBnoteParameters *pars,
    const char *id,
    const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(
    PADnoteParameters *pars,
    const char *id,
    const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(
    SampleNoteParameters *pars,
    const char *id,
    const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(
    Instrument *instrument,
    const char *id,
    const char *relativeid)
{
    if (std::string(relativeid).substr(0, sizeof("/ADD_SYNTH_PARAMETERS")) == "/ADD_SYNTH_PARAMETERS")
    {
        return GetPar(instrument->adpars, id, relativeid + sizeof("/ADD_SYNTH_PARAMETERS"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/SUB_SYNTH_PARAMETERS")) == "/SUB_SYNTH_PARAMETERS")
    {
        return GetPar(instrument->subpars, id, relativeid + sizeof("/SUB_SYNTH_PARAMETERS"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/PAD_SYNTH_PARAMETERS")) == "/PAD_SYNTH_PARAMETERS")
    {
        return GetPar(instrument->padpars, id, relativeid + sizeof("/PAD_SYNTH_PARAMETERS"));
    }
    else if (std::string(relativeid).substr(0, sizeof("/SMPL_SYNTH_PARAMETERS")) == "/SMPL_SYNTH_PARAMETERS")
    {
        return GetPar(instrument->smplpars, id, relativeid + sizeof("/SMPL_SYNTH_PARAMETERS"));
    }

    return Empty();
}

Par Par::GetPar(
    Track *track,
    const char *id)
{
    std::cmatch m;
    std::regex_search(id, m, std::regex("(\\/INSTRUMENT_KIT_ITEM\\[([0-9]+)\\])\\/"));

    if (m.empty())
    {
        return Empty();
    }

    auto index = std::atoi(m[2].str().c_str());

    if (index < 0 || index >= NUM_TRACK_INSTRUMENTS)
    {
        return Empty();
    }

    return GetPar(&track->Instruments[index], id, id + m[1].str().size());
}
