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
    const char *id = "null";
    ParTypes type = ParTypes::Unknown;
    unsigned char *ucvalue = nullptr;
    unsigned short *usvalue = nullptr;
    float *fvalue = nullptr;
    bool *bvalue = nullptr;

    unsigned char ucmin = 0;
    unsigned short usmin = 0;
    float fmin = 0;
    bool bmin = false;

    unsigned char ucmax = 0;
    unsigned short usmax = 0;
    float fmax = 0;
    bool bmax = false;

    unsigned char AsUnsignedChar() const;
    unsigned short AsUnsignedShort() const;
    float AsUnsignedFloat() const;
    bool AsUnsignedBool() const;

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
    static Par GetPar(class Resonance *pars, const char *id, const char *relativeid);
};

bool operator==(const Par &p1, const Par &p2);
bool operator!=(const Par &p1, const Par &p2);

#endif // PAR_H
