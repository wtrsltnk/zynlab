#include "par.h"

#include <fstream>
#include <regex>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/PADnoteParams.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.synth/SampleNoteParams.h>

#define IS_PAR(id, name) std::string(id).substr(0, std::string(#name).length()) == #name

extern std::ofstream logfile;

const char *TRACK_ID = "Track";
const char *INSTRUMENTS_ID = "Instruments";
const char *EFFECTS_ID = "Effects";
const char *ADDSYNTH_ID = "AddSynth";
const char *SUBSYNTH_ID = "SubSynth";
const char *PADSYNTH_ID = "PadSynth";
const char *SMPLSYNTH_ID = "SampleSynth";

sPar sPar::emptyPar = sPar();

struct sPar GetLfoParametersPar(
    LFOParams *params,
    const char *id)
{
    if (IS_PAR(id, Pfreq)) return sPar(params->Pfreq);
    if (IS_PAR(id, Pintensity)) return sPar(params->Pintensity);
    if (IS_PAR(id, Pstartphase)) return sPar(params->Pstartphase);
    if (IS_PAR(id, PLFOtype)) return sPar(params->PLFOtype);
    if (IS_PAR(id, Prandomness)) return sPar(params->Prandomness);
    if (IS_PAR(id, Pfreqrand)) return sPar(params->Pfreqrand);
    if (IS_PAR(id, Pdelay)) return sPar(params->Pdelay);
    if (IS_PAR(id, Pcontinous)) return sPar(params->Pcontinous);
    if (IS_PAR(id, Pstretch)) return sPar(params->Pstretch);

    return sPar::emptyPar;
}

struct sPar GetEnvelopePar(
    EnvelopeParams *params,
    const char *id)
{
    if (IS_PAR(id, PAttackTime)) return sPar(params->PA_dt);
    if (IS_PAR(id, PAttackValue)) return sPar(params->PA_val);
    if (IS_PAR(id, PDecayTime)) return sPar(params->PD_dt);
    if (IS_PAR(id, PDecayValue)) return sPar(params->PD_val);
    if (IS_PAR(id, PSustainValue)) return sPar(params->PS_val);
    if (IS_PAR(id, PReleaseTime)) return sPar(params->PR_dt);
    if (IS_PAR(id, PReleaseValue)) return sPar(params->PR_val);

    return sPar::emptyPar;
}

struct sPar GetFilterPar(
    FilterParams *params,
    const char *id)
{
    if (IS_PAR(id, Pcategory)) return sPar(params->Pcategory);
    if (IS_PAR(id, Pcenterfreq)) return sPar(params->Pcenterfreq);
    if (IS_PAR(id, Pformantslowness)) return sPar(params->Pformantslowness);
    if (IS_PAR(id, Pfreq)) return sPar(params->Pfreq);
    if (IS_PAR(id, Pfreqtrack)) return sPar(params->Pfreqtrack);
    if (IS_PAR(id, Pgain)) return sPar(params->Pgain);
    if (IS_PAR(id, Pnumformants)) return sPar(params->Pnumformants);
    if (IS_PAR(id, Poctavesfreq)) return sPar(params->Poctavesfreq);
    if (IS_PAR(id, Pq)) return sPar(params->Pq);
    if (IS_PAR(id, Pstages)) return sPar(params->Pstages);
    if (IS_PAR(id, Ptype)) return sPar(params->Ptype);
    if (IS_PAR(id, Pvowelclearness)) return sPar(params->Pvowelclearness);

    return sPar::emptyPar;
}

bool GetAbstractSynthPar(
    AbstractNoteParameters *params,
    const char *id,
    sPar &par)
{
    logfile << "GetAbstractSynthPar" << id << std::endl;
    if (IS_PAR(id, Pvolume))
    {
        par.byteValue = &(params->PVolume);

        return true;
    }

    if (IS_PAR(id, Ppanning))
    {
        par.byteValue = &(params->PPanning);

        return true;
    }

    if (IS_PAR(id, PAmpVelocityScaleFunction))
    {
        par.byteValue = &(params->PAmpVelocityScaleFunction);

        return true;
    }

    if (IS_PAR(id, Pfixedfreq))
    {
        par.byteValue = &(params->Pfixedfreq);

        return true;
    }

    if (IS_PAR(id, PfixedfreqET))
    {
        par.byteValue = &(params->PfixedfreqET);

        return true;
    }

    if (IS_PAR(id, PDetune))
    {
        par.shortIntValue = &(params->PDetune);

        return true;
    }

    if (IS_PAR(id, PCoarseDetune))
    {
        par.shortIntValue = &(params->PCoarseDetune);

        return true;
    }

    if (IS_PAR(id, PDetuneType))
    {
        par.byteValue = &(params->PDetuneType);

        return true;
    }

    if (IS_PAR(id, PFreqEnvelopeEnabled))
    {
        par.byteValue = &(params->PFreqEnvelopeEnabled);

        return true;
    }

    if (IS_PAR(id, FrequencyEnvelope))
    {
        par = GetEnvelopePar(params->FreqEnvelope, id);

        return true;
    }

    if (IS_PAR(id, PFreqLfoEnabled))
    {
        par.byteValue = &(params->PFreqLfoEnabled);

        return true;
    }

    if (IS_PAR(id, FrequencyLfo))
    {
        par = GetLfoParametersPar(params->FreqLfo, id);

        return true;
    }

    if (IS_PAR(id, PBandwidth))
    {
        par.byteValue = &(params->PBandwidth);

        return true;
    }

    if (IS_PAR(id, PAmpEnvelopeEnabled))
    {
        par.byteValue = &(params->PAmpEnvelopeEnabled);

        return true;
    }

    if (IS_PAR(id, AmpEnvelope))
    {
        par = GetEnvelopePar(params->AmpEnvelope, id);

        return true;
    }

    if (IS_PAR(id, GlobalFilter))
    {
        par = GetFilterPar(params->GlobalFilter, id);

        return true;
    }

    if (IS_PAR(id, PFilterVelocityScale))
    {
        par.byteValue = &(params->PFilterVelocityScale);

        return true;
    }

    if (IS_PAR(id, PFilterVelocityScaleFunction))
    {
        par.byteValue = &(params->PFilterVelocityScaleFunction);

        return true;
    }

    if (IS_PAR(id, FilterEnvelope))
    {
        par = GetEnvelopePar(params->FilterEnvelope, id);

        return true;
    }

    return false;
}

struct sPar GetAddSynthPar(
    ADnoteParameters *params,
    const char *id)
{
    sPar par;

    if (GetAbstractSynthPar(params, id, par))
    {
        return par;
    }

    if (IS_PAR(id, PPunchStrength)) return sPar(params->PPunchStrength);
    if (IS_PAR(id, PPunchTime)) return sPar(params->PPunchTime);
    if (IS_PAR(id, PPunchStretch)) return sPar(params->PPunchStretch);
    if (IS_PAR(id, PPunchVelocitySensing)) return sPar(params->PPunchVelocitySensing);
    if (IS_PAR(id, AmpLfo)) return GetLfoParametersPar(params->AmpLfo, id);
    if (IS_PAR(id, AmpEnvelope)) return GetEnvelopePar(params->AmpEnvelope, id);
    if (IS_PAR(id, FilterLfo)) return GetLfoParametersPar(params->FilterLfo, id);
    if (IS_PAR(id, FilterEnvelope)) return GetEnvelopePar(params->FilterEnvelope, id);
    if (IS_PAR(id, FreqLfo)) return GetLfoParametersPar(params->FreqLfo, id);
    if (IS_PAR(id, FreqEnvelope)) return GetEnvelopePar(params->FreqEnvelope, id);

    if (IS_PAR(id, Voices))
    {
    }

    return sPar::emptyPar;
}

struct sPar GetSubSynthPar(
    SUBnoteParameters *params,
    const char *id)
{
    sPar par;

    if (GetAbstractSynthPar(params, id, par))
    {
        return par;
    }

    if (IS_PAR(id, PBandWidthEnvelopeEnabled)) return sPar(params->PBandWidthEnvelopeEnabled);
    if (IS_PAR(id, BandWidthEnvelope)) return GetEnvelopePar(params->BandWidthEnvelope, id);
    if (IS_PAR(id, PGlobalFilterEnabled)) return sPar(params->PGlobalFilterEnabled);
    if (IS_PAR(id, POvertoneSpread.type)) return sPar(params->POvertoneSpread.type);
    if (IS_PAR(id, POvertoneSpread.par1)) return sPar(params->POvertoneSpread.par1);
    if (IS_PAR(id, POvertoneSpread.par2)) return sPar(params->POvertoneSpread.par2);
    if (IS_PAR(id, POvertoneSpread.par3)) return sPar(params->POvertoneSpread.par3);
    if (IS_PAR(id, Pnumstages)) return sPar(params->Pnumstages);
    if (IS_PAR(id, Phmagtype)) return sPar(params->Phmagtype);
    if (IS_PAR(id, Pbwscale)) return sPar(params->Pbwscale);
    if (IS_PAR(id, Pstart)) return sPar(params->Pstart);

    if (IS_PAR(id, POvertoneFreqMult))
    {
        std::cmatch m;
        std::regex_search(id, m, std::regex("POvertoneFreqMult(\\[([0-9]+)\\]\\)"));

        if (m.empty())
        {
            return sPar::emptyPar;
        }

        auto index = std::atoi(m[2].str().c_str());

        if (index < 0 || index >= MAX_SUB_HARMONICS)
        {
            return sPar::emptyPar;
        }

        return sPar(params->POvertoneFreqMult[index]);
    }

    if (IS_PAR(id, Phmag))
    {
        std::cmatch m;
        std::regex_search(id, m, std::regex("Phmag(\\[([0-9]+)\\]\\)"));

        if (m.empty())
        {
            return sPar::emptyPar;
        }

        auto index = std::atoi(m[2].str().c_str());

        if (index < 0 || index >= MAX_SUB_HARMONICS)
        {
            return sPar::emptyPar;
        }

        return sPar(params->Phmag[index]);
    }

    if (IS_PAR(id, Phrelbw))
    {
        std::cmatch m;
        std::regex_search(id, m, std::regex("Phrelbw(\\[([0-9]+)\\]\\)"));

        if (m.empty())
        {
            return sPar::emptyPar;
        }

        auto index = std::atoi(m[2].str().c_str());

        if (index < 0 || index >= MAX_SUB_HARMONICS)
        {
            return sPar::emptyPar;
        }

        return sPar(params->Phrelbw[index]);
    }

    return sPar::emptyPar;
}

struct sPar GetPadSynthPar(
    PADnoteParameters *params,
    const char *id)
{
    sPar par;

    if (GetAbstractSynthPar(params, id, par))
    {
        return par;
    }

    return sPar::emptyPar;
}

struct sPar GetSampleSynthPar(
    SampleNoteParameters *params,
    const char *id)
{
    sPar par;

    if (GetAbstractSynthPar(params, id, par))
    {
        return par;
    }

    return sPar::emptyPar;
}

struct sPar GetInstrumentPar(
    Track *track,
    const char *id)
{
    std::cmatch m;
    std::regex_search(id, m, std::regex("^(\\[([0-9]+)\\]\\.)"));

    if (m.empty())
    {
        return sPar::emptyPar;
    }

    auto index = std::atoi(m[2].str().c_str());

    if (index < 0 || index >= NUM_TRACK_INSTRUMENTS)
    {
        return sPar::emptyPar;
    }

    auto relativeId = id + m[1].str().size();

    if (IS_PAR(relativeId, Padenabled)) return sPar(track->Instruments[index].Padenabled);
    if (IS_PAR(relativeId, Psubenabled)) return sPar(track->Instruments[index].Psubenabled);
    if (IS_PAR(relativeId, Ppadenabled)) return sPar(track->Instruments[index].Ppadenabled);
    if (IS_PAR(relativeId, Psmplenabled)) return sPar(track->Instruments[index].Psmplenabled);
    if (IS_PAR(relativeId, Psendtoparteffect)) return sPar(track->Instruments[index].Psendtoparteffect);
    if (IS_PAR(relativeId, Pminkey)) return sPar(track->Instruments[index].Pminkey);
    if (IS_PAR(relativeId, Pmaxkey)) return sPar(track->Instruments[index].Pmaxkey);

    if (IS_PAR(relativeId, AddSynth))
    {
        return GetAddSynthPar(track->Instruments[index].adpars, relativeId + std::string(ADDSYNTH_ID).length() + 1);
    }

    if (IS_PAR(relativeId, SubSynth))
    {
        return GetSubSynthPar(track->Instruments[index].subpars, relativeId + std::string(SUBSYNTH_ID).length() + 1);
    }

    if (IS_PAR(relativeId, PadSynth))
    {
        return GetPadSynthPar(track->Instruments[index].padpars, relativeId + std::string(PADSYNTH_ID).length() + 1);
    }

    if (IS_PAR(relativeId, SampleSynth))
    {
        return GetSampleSynthPar(track->Instruments[index].smplpars, relativeId + std::string(SMPLSYNTH_ID).length() + 1);
    }

    return sPar::emptyPar;
}

struct sPar GetEffectPar(
    EffectManager partefx[],
    const char *id)
{
    logfile << "GetEffectPar() : " << id << std::endl;

    std::cmatch m;
    std::regex_search(id, m, std::regex("^(\\[([0-9]+)\\]\\.)"));

    if (m.empty())
    {
        return sPar::emptyPar;
    }

    auto effectIndex = std::atoi(m[2].str().c_str());

    if (effectIndex < 0 || effectIndex >= NUM_TRACK_EFX)
    {
        return sPar::emptyPar;
    }

    EffectManager &effect = partefx[effectIndex];

    auto relativeId = id + m[1].str().size();

    std::regex_search(relativeId, m, std::regex("^Parameters(\\[([0-9]+)\\])$"));

    if (m.empty())
    {
        return sPar::emptyPar;
    }

    auto parameterIndex = std::atoi(m[2].str().c_str());

    if (parameterIndex < 0)
    {
        return sPar::emptyPar;
    }

    sPar par;

    par.setByteIsSet = true;
    par.setByte = [partefx, effectIndex, parameterIndex](unsigned char value) {
        partefx[effectIndex].seteffectpar(parameterIndex, value);
    };

    return par;
}

struct sPar GetTrackPar(
    Track *track,
    const char *id)
{
    if (IS_PAR(id, Pvolume))
    {
        sPar par(track->Pvolume);

        par.setByteIsSet = true;
        par.setByte = [track](unsigned char value) {
            track->SetVolume(value);
        };

        return par;
    }

    if (IS_PAR(id, Ppanning))
    {
        sPar par(track->Ppanning);

        par.setByteIsSet = true;
        par.setByte = [track](unsigned char value) {
            track->setPpanning(value);
        };

        return par;
    }

    if (IS_PAR(id, Pvelsns)) return sPar(track->Pvelsns);
    if (IS_PAR(id, Pveloffs)) return sPar(track->Pveloffs);
    if (IS_PAR(id, Pminkey)) return sPar(track->Pminkey);
    if (IS_PAR(id, Pmaxkey)) return sPar(track->Pmaxkey);
    if (IS_PAR(id, Pkeyshift)) return sPar(track->Pkeyshift);
    if (IS_PAR(id, Prcvchn)) return sPar(track->Prcvchn);
    if (IS_PAR(id, Pkitmode)) return sPar(track->Pkitmode);
    if (IS_PAR(id, Pminkey)) return sPar(track->Pminkey);
    if (IS_PAR(id, Pmaxkey)) return sPar(track->Pmaxkey);

    if (IS_PAR(id, Instruments))
    {
        return GetInstrumentPar(track, id + std::string(INSTRUMENTS_ID).length());
    }

    if (IS_PAR(id, Effects))
    {
        return GetEffectPar(track->partefx, id + std::string(EFFECTS_ID).length());
    }

    return sPar::emptyPar;
}

sPar GetParById(
    Mixer *mixer,
    unsigned char trackIndex,
    const char *id)
{
    logfile << "GetParById : " << id << std::endl;

    if (IS_PAR(id, Pvolume))
    {
        sPar par(mixer->Pvolume);

        par.setByteIsSet = true;
        par.setByte = [mixer](unsigned char value) {
            mixer->SetVolume(value);
        };

        return par;
    }

    if (IS_PAR(id, Pkeyshift)) return sPar(mixer->Pkeyshift);

    if (IS_PAR(id, SystemEffects))
    {
        logfile << "SystemEffects : " << id << std::endl;

        return GetEffectPar(mixer->sysefx, id + std::string("SystemEffects").length() + 1);
    }

    if (IS_PAR(id, InsertEffects))
    {
        logfile << "InsertEffects : " << id << std::endl;

        return GetEffectPar(mixer->insefx, id + std::string("InsertEffects").length() + 1);
    }

    auto track = mixer->GetTrack(trackIndex);

    if (track == nullptr)
    {
        return sPar::emptyPar;
    }

    auto const trackId = std::string(id).substr(0, std::string(TRACK_ID).length());

    if (trackId != TRACK_ID)
    {
        logfile << "id not valid : " << id << " (" << trackId << ")" << std::endl;

        return sPar::emptyPar;
    }

    if (IS_PAR(id + trackId.length() + 1, Pfxsend1)) return sPar(mixer->Psysefxvol[0][trackIndex]);
    if (IS_PAR(id + trackId.length() + 1, Pfxsend2)) return sPar(mixer->Psysefxvol[1][trackIndex]);
    if (IS_PAR(id + trackId.length() + 1, Pfxsend3)) return sPar(mixer->Psysefxvol[2][trackIndex]);
    if (IS_PAR(id + trackId.length() + 1, Pfxsend4)) return sPar(mixer->Psysefxvol[3][trackIndex]);

    return GetTrackPar(track, id + trackId.length() + 1); // the +1 is for the dot
}
