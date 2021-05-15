#include "par.h"

#include <regex>
#include <zyn.mixer/Track.h>
#include <zyn.synth/ADnoteParams.h>

bool CheckId(const char *id, const std::string &relativeid, const char *checkid, unsigned char *value, Par &p)
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

bool CheckId(const char *id, const std::string &relativeid, const char *checkid, unsigned short *value, Par &p)
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

bool CheckId(const char *id, const std::string &relativeid, const char *checkid, float *value, Par &p)
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

bool CheckId(const char *id, const std::string &relativeid, const char *checkid, bool *value, Par &p)
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

Par Par::Empty()
{
    static Par i = {"null", ParTypes::Unknown, {nullptr}, {0}, {0}};

    return i;
}

Par Par::GetPar(FilterParams *pars, const char *id, const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(LFOParams *pars, const char *id, const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(EnvelopeParams *pars, const char *id, const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(ADnoteParameters *pars, const char *id, const char *relativeid)
{
    Par p;

    if (CheckId(id, relativeid, "/detune", &(pars->PDetune), p)) return p;
    if (CheckId(id, relativeid, "/detune-type", &(pars->PDetuneType), p)) return p;

    return Empty();
}

Par Par::GetPar(ADnoteVoiceParam *pars, const char *id, const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(SUBnoteParameters *pars, const char *id, const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(PADnoteParameters *pars, const char *id, const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(SampleNoteParameters *pars, const char *id, const char *relativeid)
{
    return Empty();
}

Par Par::GetPar(Instrument *instrument, const char *id, const char *relativeid)
{
    if (std::string(relativeid).substr(0, 4) == "/add")
    {
        return GetPar(instrument->adpars, id, relativeid + 4);
    }
    else if (std::string(relativeid).substr(0, 4) == "/sub")
    {
        return GetPar(instrument->subpars, id, relativeid + 4);
    }
    else if (std::string(relativeid).substr(0, 4) == "/pad")
    {
        return GetPar(instrument->padpars, id, relativeid + 4);
    }
    else if (std::string(relativeid).substr(0, 5) == "/smpl")
    {
        return GetPar(instrument->smplpars, id, relativeid + 5);
    }

    return Empty();
}

Par Par::GetPar(Track *track, const char *id)
{
    std::cmatch m;
    std::regex_search(id, m, std::regex("(\\/kit\\[([0-9]+)\\])\\/"));

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
