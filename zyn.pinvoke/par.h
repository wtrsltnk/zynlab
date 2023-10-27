#ifndef PAR_H
#define PAR_H

#include <functional>
#include <zyn.mixer/Mixer.h>

struct sPar
{
    sPar() {}
    sPar(unsigned char &value) : byteValue(&value) {}
    sPar(char &value) : charValue(&value) {}
    sPar(float &value) : floatValue(&value) {}
    sPar(bool &value) : boolValue(&value) {}
    sPar(unsigned short int &value) : shortIntValue(&value) {}

    unsigned char *byteValue = nullptr;
    char *charValue = nullptr;
    float *floatValue = nullptr;
    bool *boolValue = nullptr;
    unsigned short int *shortIntValue = nullptr;

    bool setByteIsSet = false;
    std::function<void(unsigned char)> setByte;

    bool getByteIsSet = false;
    std::function<unsigned char()> getByte;

    bool setCharIsSet = false;
    std::function<void(char)> setChar;

    bool setFloatIsSet = false;
    std::function<void(float)> setFloat;

    bool setBoolIsSet = false;
    std::function<void(bool)> setBool;

    static sPar emptyPar;
};

sPar GetParById(
    Mixer *mixer,
    unsigned char chan,
    const char *id);

EffectManager *GetEffectManagerById(
    Mixer *mixer,
    unsigned char trackIndex,
    const char *id);

#endif // PAR_H
