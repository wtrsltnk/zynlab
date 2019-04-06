#ifndef WAVDATA_H
#define WAVDATA_H

#include <string>

class WavData
{
public:
    std::string name;
    unsigned int channels;
    unsigned int samplesPerChannel;
    float *PwavData;

    static WavData *Load(std::string const &filename);
    static std::string toBase64(WavData *wavData);
    static bool fromBase64(std::string const &data, WavData *wavData);
};

#endif // WAVDATA_H
