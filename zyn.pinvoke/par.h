#ifndef PAR_H
#define PAR_H

enum class ParTypes
{
    Unknown,
    UnsignedChar,
    UnsignedShort,
    Float,
    Bool,
};

class Par
{
public:
    const char *id;
    ParTypes type;
    union {
        unsigned char *ucvalue;
        unsigned short *usvalue;
        float *fvalue;
        bool *bvalue;
    };

    union {
        unsigned char ucmin;
        unsigned short usmin;
        float fmin;
        bool bmin;
    };
    union {
        unsigned char ucmax;
        unsigned short usmax;
        float fmax;
        bool bmax;
    };

    static Par Empty();

    static Par GetPar(class Track *track, const char *id);
    static Par GetPar(class Instrument *instrument, const char *id, const char *relativeid);
    static Par GetPar(class ADnoteParameters *pars, const char *id, const char *relativeid);
    static Par GetPar(class ADnoteVoiceParam *pars, const char *id, const char *relativeid);
    static Par GetPar(class SUBnoteParameters *pars, const char *id, const char *relativeid);
    static Par GetPar(class PADnoteParameters *pars, const char *id, const char *relativeid);
    static Par GetPar(class SampleNoteParameters *pars, const char *id, const char *relativeid);
    static Par GetPar(class EnvelopeParams *pars, const char *id, const char *relativeid);
    static Par GetPar(class LFOParams *pars, const char *id, const char *relativeid);
    static Par GetPar(class FilterParams *pars, const char *id, const char *relativeid);
    static Par GetPar(class Resonance*pars, const char *id, const char *relativeid);
};

#endif // PAR_H
