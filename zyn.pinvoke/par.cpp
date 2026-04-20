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
const char *VOICES_ID = "Voices";

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
    if (IS_PAR(id, Pforcedrelease)) return sPar(params->Pforcedrelease);
    if (IS_PAR(id, Plinearenvelope)) return sPar(params->Plinearenvelope);
    if (IS_PAR(id, PStretch)) return sPar(params->Penvstretch);

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

bool GetAbstractNoteParameters(
    AbstractNoteParameters *params,
    const char *id,
    sPar &par)
{
    logfile << "GetAbstractNoteParameters" << id << std::endl;
    if (IS_PAR(id, PVolume))
    {
        par.byteValue = &(params->PVolume);

        return true;
    }

    if (IS_PAR(id, PPanning))
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
        par.unsignedShortIntValue = &(params->PDetune);

        return true;
    }

    if (IS_PAR(id, PCoarseDetune))
    {
        par.unsignedShortIntValue = &(params->PCoarseDetune);

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
        par = GetEnvelopePar(params->FreqEnvelope, id + std::string("FrequencyEnvelope").length() + 1);

        return true;
    }

    if (IS_PAR(id, PFreqLfoEnabled))
    {
        par.byteValue = &(params->PFreqLfoEnabled);

        return true;
    }

    if (IS_PAR(id, FrequencyLfo))
    {
        par = GetLfoParametersPar(params->FreqLfo, id + std::string("FrequencyLfo").length() + 1);

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
        par = GetEnvelopePar(params->AmpEnvelope, id + std::string("AmpEnvelope").length() + 1);

        return true;
    }

    if (IS_PAR(id, GlobalFilter))
    {
        par = GetFilterPar(params->GlobalFilter, id + std::string("GlobalFilter").length() + 1);

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
        par = GetEnvelopePar(params->FilterEnvelope, id + std::string("FilterEnvelope").length() + 1);

        return true;
    }

    return false;
}

struct sPar GetAddVoiceParam(
    ADnoteParameters *params,
    const char *id)
{
    std::cmatch m;
    std::regex_search(id, m, std::regex("^(\\[([0-9]+)\\]\\.)"));

    if (m.empty())
    {
        return sPar::emptyPar;
    }

    auto index = std::atoi(m[2].str().c_str());

    if (index < 0 || index >= NUM_VOICES)
    {
        return sPar::emptyPar;
    }

    auto relativeId = id + m[1].str().size();

    /*************************************
     *     ABSTRACTNOTEPARAMETERS        *
     *************************************/
    {
        if (IS_PAR(relativeId, PStereo)) return sPar(params->VoicePar[index].PStereo);
        if (IS_PAR(relativeId, PPanning)) return sPar(params->VoicePar[index].PPanning);
        if (IS_PAR(relativeId, PVolume)) return sPar(params->VoicePar[index].PVolume);
        if (IS_PAR(relativeId, PAmpVelocityScaleFunction)) return sPar(params->VoicePar[index].PAmpVelocityScaleFunction);

        /******************************************
         *     FREQUENCY GLOBAL PARAMETERS        *
         ******************************************/
        if (IS_PAR(relativeId, Pfixedfreq)) return sPar(params->VoicePar[index].Pfixedfreq);
        if (IS_PAR(relativeId, PfixedfreqET)) return sPar(params->VoicePar[index].PfixedfreqET);
        if (IS_PAR(relativeId, PDetune)) return sPar(params->VoicePar[index].PDetune);
        if (IS_PAR(relativeId, PCoarseDetune)) return sPar(params->VoicePar[index].PCoarseDetune);
        if (IS_PAR(relativeId, PDetuneType)) return sPar(params->VoicePar[index].PDetuneType);

        if (IS_PAR(relativeId, PFreqEnvelopeEnabled)) return sPar(params->VoicePar[index].PFreqEnvelopeEnabled);
        if (IS_PAR(relativeId, FreqEnvelope)) return GetEnvelopePar(params->VoicePar[index].FreqEnvelope, id + std::string("FreqEnvelope").length() + 1);

        if (IS_PAR(relativeId, PFreqLfoEnabled)) return sPar(params->VoicePar[index].PFreqLfoEnabled);
        if (IS_PAR(relativeId, FreqLfo)) return GetLfoParametersPar(params->VoicePar[index].FreqLfo, id + std::string("FreqLfo").length() + 1);

        if (IS_PAR(relativeId, PBandwidth)) return sPar(params->VoicePar[index].PBandwidth);

        /****************************
         *   AMPLITUDE PARAMETERS   *
         ***************************/
        if (IS_PAR(relativeId, PAmpEnvelopeEnabled)) return sPar(params->VoicePar[index].PAmpEnvelopeEnabled);
        if (IS_PAR(relativeId, AmpEnvelope)) return GetEnvelopePar(params->VoicePar[index].AmpEnvelope, id + std::string("AmpEnvelope").length() + 1);

        /******************************************
         *        FILTER GLOBAL PARAMETERS        *
         ******************************************/
        if (IS_PAR(relativeId, GlobalFilter)) return GetFilterPar(params->VoicePar[index].GlobalFilter, id + std::string("GlobalFilter").length() + 1);
        if (IS_PAR(relativeId, PFilterVelocityScale)) return sPar(params->VoicePar[index].PFilterVelocityScale);
        if (IS_PAR(relativeId, PFilterVelocityScaleFunction)) return sPar(params->VoicePar[index].PFilterVelocityScaleFunction);
        if (IS_PAR(relativeId, FilterEnvelope)) return GetEnvelopePar(params->VoicePar[index].FilterEnvelope, id + std::string("FilterEnvelope").length() + 1);
    }

    /***********************************************************
     *                    VOICE PARAMETERS                     *
     ***********************************************************/
    {
        if (IS_PAR(relativeId, Enabled)) return sPar(params->VoicePar[index].Enabled);
        if (IS_PAR(relativeId, Unison_size)) return sPar(params->VoicePar[index].Unison_size);
        if (IS_PAR(relativeId, Unison_frequency_spread)) return sPar(params->VoicePar[index].Unison_frequency_spread);
        if (IS_PAR(relativeId, Unison_phase_randomness)) return sPar(params->VoicePar[index].Unison_phase_randomness);
        if (IS_PAR(relativeId, Unison_stereo_spread)) return sPar(params->VoicePar[index].Unison_stereo_spread);
        if (IS_PAR(relativeId, Unison_vibratto)) return sPar(params->VoicePar[index].Unison_vibratto);
        if (IS_PAR(relativeId, Unison_vibratto_speed)) return sPar(params->VoicePar[index].Unison_vibratto_speed);
        if (IS_PAR(relativeId, Unison_invert_phase)) return sPar(params->VoicePar[index].Unison_invert_phase);
        if (IS_PAR(relativeId, Type)) return sPar(params->VoicePar[index].Type);
        if (IS_PAR(relativeId, PDelay)) return sPar(params->VoicePar[index].PDelay);
        if (IS_PAR(relativeId, Presonance)) return sPar(params->VoicePar[index].Presonance);
        if (IS_PAR(relativeId, Pextoscil)) return sPar(params->VoicePar[index].Pextoscil);
        if (IS_PAR(relativeId, PextFMoscil)) return sPar(params->VoicePar[index].PextFMoscil);
        if (IS_PAR(relativeId, Poscilphase)) return sPar(params->VoicePar[index].Poscilphase);
        if (IS_PAR(relativeId, PFMoscilphase)) return sPar(params->VoicePar[index].PFMoscilphase);
        if (IS_PAR(relativeId, Pfilterbypass)) return sPar(params->VoicePar[index].Pfilterbypass);
        // TODO : OscilSmp

        /****************************
         *   AMPLITUDE PARAMETERS   *
         ***************************/
        if (IS_PAR(relativeId, PVolumeminus)) return sPar(params->VoicePar[index].PVolumeminus);

        if (IS_PAR(relativeId, PAmpLfoEnabled)) return sPar(params->VoicePar[index].PAmpLfoEnabled);
        if (IS_PAR(relativeId, AmpLfo)) return GetLfoParametersPar(params->VoicePar[index].AmpLfo, id + std::string("AmpLfo").length() + 1);

        /**************************
         *   FILTER PARAMETERS    *
         *************************/
        if (IS_PAR(relativeId, PFilterEnabled)) return sPar(params->VoicePar[index].PFilterEnabled);
        if (IS_PAR(relativeId, VoiceFilter)) return GetFilterPar(params->VoicePar[index].VoiceFilter, id + std::string("VoiceFilter").length() + 1);

        if (IS_PAR(relativeId, PFilterEnvelopeEnabled)) return sPar(params->VoicePar[index].PFilterEnvelopeEnabled);

        if (IS_PAR(relativeId, PFilterLfoEnabled)) return sPar(params->VoicePar[index].PFilterLfoEnabled);
        if (IS_PAR(relativeId, FilterLfo)) return GetLfoParametersPar(params->VoicePar[index].FilterLfo, id + std::string("FilterLfo").length() + 1);

        /*****************************
         *   MODULLATOR PARAMETERS   *
         ****************************/
        if (IS_PAR(relativeId, PFMEnabled)) return sPar(params->VoicePar[index].PFMEnabled);
        if (IS_PAR(relativeId, PFMVoice)) return sPar(params->VoicePar[index].PFMVoice);
        // TODO : FMSmp
        if (IS_PAR(relativeId, PFMVolume)) return sPar(params->VoicePar[index].PFMVolume);
        if (IS_PAR(relativeId, PFMVolumeDamp)) return sPar(params->VoicePar[index].PFMVolumeDamp);
        if (IS_PAR(relativeId, PFMVelocityScaleFunction)) return sPar(params->VoicePar[index].PFMVelocityScaleFunction);
        if (IS_PAR(relativeId, PFMDetune)) return sPar(params->VoicePar[index].PFMDetune);
        if (IS_PAR(relativeId, PFMCoarseDetune)) return sPar(params->VoicePar[index].PFMCoarseDetune);
        if (IS_PAR(relativeId, PFMDetuneType)) return sPar(params->VoicePar[index].PFMDetuneType);

        if (IS_PAR(relativeId, PFMFreqEnvelopeEnabled)) return sPar(params->VoicePar[index].PFMFreqEnvelopeEnabled);
        if (IS_PAR(relativeId, FMFreqEnvelope)) return GetEnvelopePar(params->VoicePar[index].FMFreqEnvelope, id + std::string("FMFreqEnvelope").length() + 1);

        if (IS_PAR(relativeId, PFMAmpEnvelopeEnabled)) return sPar(params->VoicePar[index].PFMAmpEnvelopeEnabled);
        if (IS_PAR(relativeId, FMAmpEnvelope)) return GetEnvelopePar(params->VoicePar[index].FMAmpEnvelope, id + std::string("FMAmpEnvelope").length() + 1);
    }

    return sPar::emptyPar;
}

struct sPar GetAddSynthPar(
    ADnoteParameters *params,
    const char *id)
{
    sPar par;

    if (GetAbstractNoteParameters(params, id, par))
    {
        return par;
    }

    /********************************************
     *     AMPLITUDE GLOBAL PARAMETERS          *
     ********************************************/
    if (IS_PAR(id, AmpLfo)) return GetLfoParametersPar(params->AmpLfo, id + std::string("AmpLfo").length() + 1);
    if (IS_PAR(id, PPunchStrength)) return sPar(params->PPunchStrength);
    if (IS_PAR(id, PPunchTime)) return sPar(params->PPunchTime);
    if (IS_PAR(id, PPunchStretch)) return sPar(params->PPunchStretch);
    if (IS_PAR(id, PPunchVelocitySensing)) return sPar(params->PPunchVelocitySensing);

    /******************************************
     *        FILTER GLOBAL PARAMETERS        *
     ******************************************/
    if (IS_PAR(id, FilterLfo)) return GetLfoParametersPar(params->FilterLfo, id + std::string("FilterLfo").length() + 1);

    /*******************************************
     *            OTHER PARAMETERS             *
     ******************************************/
    // TODO Reson
    if (IS_PAR(id, Hrandgrouping)) return sPar(params->Hrandgrouping);

    if (IS_PAR(id, Voices))
    {
        return GetAddVoiceParam(params, id + std::string(VOICES_ID).length());
    }

    return sPar::emptyPar;
}

struct sPar GetSubSynthPar(
    SUBnoteParameters *params,
    const char *id)
{
    sPar par;

    if (GetAbstractNoteParameters(params, id, par))
    {
        return par;
    }

    /******************************************
     *     FREQUENCY GLOBAL PARAMETERS        *
     ******************************************/
    if (IS_PAR(id, PBandWidthEnvelopeEnabled)) return sPar(params->PBandWidthEnvelopeEnabled);
    if (IS_PAR(id, BandWidthEnvelope)) return GetEnvelopePar(params->BandWidthEnvelope, id + std::string("BandWidthEnvelope").length() + 1);

    /******************************************
     *        FILTER GLOBAL PARAMETERS        *
     ******************************************/
    if (IS_PAR(id, PGlobalFilterEnabled)) return sPar(params->PGlobalFilterEnabled);

    /*******************************************
     *            OTHER PARAMETERS             *
     ******************************************/
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

    if (GetAbstractNoteParameters(params, id, par))
    {
        return par;
    }

    // if (IS_PAR(id, PBandwidth)) return sPar(params->PBandwidth);
    if (IS_PAR(id, Pbwscale)) return sPar(params->Pbwscale);

    /****************************
     *   AMPLITUDE PARAMETERS   *
     ***************************/
    if (IS_PAR(id, AmpLfo)) return GetLfoParametersPar(params->AmpLfo, id + std::string("AmpLfo").length() + 1);
    if (IS_PAR(id, PPunchStrength)) return sPar(params->PPunchStrength);
    if (IS_PAR(id, PPunchTime)) return sPar(params->PPunchTime);
    if (IS_PAR(id, PPunchStretch)) return sPar(params->PPunchStretch);
    if (IS_PAR(id, PPunchVelocitySensing)) return sPar(params->PPunchVelocitySensing);

    /**************************
     *   FILTER PARAMETERS    *
     *************************/
    if (IS_PAR(id, FilterLfo)) return GetLfoParametersPar(params->FilterLfo, id + std::string("FilterLfo").length() + 1);
    // TODO oscilgen
    // TODO resonance

    return sPar::emptyPar;
}

struct sPar GetSampleSynthPar(
    SampleNoteParameters *params,
    const char *id)
{
    sPar par;

    if (GetAbstractNoteParameters(params, id, par))
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
    int maxEffects,
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

    if (effectIndex < 0 || effectIndex >= maxEffects)
    {
        return sPar::emptyPar;
    }

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
        logfile << "seteffectpar(" << parameterIndex << ", " << value << ")" << std::endl;
        partefx[effectIndex].seteffectpar(parameterIndex, value);
    };
    par.getByteIsSet = true;
    par.getByte = [partefx, effectIndex, parameterIndex]() -> unsigned char {
        return partefx[effectIndex].geteffectpar(parameterIndex);
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

    if (IS_PAR(id, Pkeylimit))
    {
        sPar par(track->Pkeylimit);

        par.setByteIsSet = true;
        par.setByte = [track](unsigned char value) {
            track->setkeylimit(value);
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
        return GetEffectPar(track->partefx, NUM_TRACK_EFX, id + std::string(EFFECTS_ID).length());
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
        return GetEffectPar(mixer->sysefx, NUM_SYS_EFX, id + std::string("SystemEffects").length());
    }

    if (IS_PAR(id, InsertEffects))
    {
        return GetEffectPar(mixer->insefx, NUM_INS_EFX, id + std::string("InsertEffects").length());
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

    if (IS_PAR(id + trackId.length() + 1, Pfxsend1))
    {
        sPar par(mixer->Psysefxvol[0][trackIndex]);

        par.setByteIsSet = true;
        par.setByte = [mixer, trackIndex](unsigned char value) {
            mixer->SetSystemEffectVolume(trackIndex, 0, value);
        };

        return par;
    }

    if (IS_PAR(id + trackId.length() + 1, Pfxsend2))
    {
        sPar par(mixer->Psysefxvol[1][trackIndex]);

        par.setByteIsSet = true;
        par.setByte = [mixer, trackIndex](unsigned char value) {
            mixer->SetSystemEffectVolume(trackIndex, 1, value);
        };
    }

    if (IS_PAR(id + trackId.length() + 1, Pfxsend3))
    {
        sPar par(mixer->Psysefxvol[2][trackIndex]);

        par.setByteIsSet = true;
        par.setByte = [mixer, trackIndex](unsigned char value) {
            mixer->SetSystemEffectVolume(trackIndex, 2, value);
        };
    }

    if (IS_PAR(id + trackId.length() + 1, Pfxsend4))
    {
        sPar par(mixer->Psysefxvol[3][trackIndex]);

        par.setByteIsSet = true;
        par.setByte = [mixer, trackIndex](unsigned char value) {
            mixer->SetSystemEffectVolume(trackIndex, 3, value);
        };
    }

    return GetTrackPar(track, id + trackId.length() + 1); // the +1 is for the dot
}

EffectManager *GetEffectManagerById(
    Mixer *mixer,
    unsigned char trackIndex,
    const char *id)
{
    std::cmatch m;
    std::regex_search(id, m, std::regex("(\\[([0-9]+)\\])$"));

    if (m.empty())
    {
        return nullptr;
    }

    auto effectIndex = std::atoi(m[2].str().c_str());

    if (IS_PAR(id, SystemEffects))
    {
        if (effectIndex < 0 || effectIndex >= NUM_SYS_EFX)
        {
            return nullptr;
        }

        return &mixer->sysefx[effectIndex];
    }

    if (IS_PAR(id, InsertEffects))
    {
        if (effectIndex < 0 || effectIndex >= NUM_INS_EFX)
        {
            return nullptr;
        }

        return &mixer->insefx[effectIndex];
    }

    if (IS_PAR(id, Track.Effects))
    {
        if (effectIndex < 0 || effectIndex >= NUM_TRACK_EFX)
        {
            return nullptr;
        }

        auto track = mixer->GetTrack(trackIndex);

        if (track == nullptr)
        {
            return nullptr;
        }

        return &track->partefx[effectIndex];
    }

    return nullptr;
}
