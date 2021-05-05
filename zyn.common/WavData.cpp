#include "WavData.h"

#include "base64.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <string>

WavData *WavData::Load(const std::string &filename)
{
    drwav *pWav = drwav_open_file(filename.c_str());
    if (pWav == nullptr)
    {
        return nullptr;
    }

    auto channels = pWav->channels;
    auto samplesPerChannel = static_cast<size_t>(pWav->totalPCMFrameCount);
    auto PwavData = reinterpret_cast<float *>(malloc(static_cast<size_t>(pWav->totalPCMFrameCount) * pWav->channels * sizeof(float)));

    auto read = drwav_read_pcm_frames_f32(pWav, samplesPerChannel, PwavData);

    if (read == samplesPerChannel)
    {
        auto result = new WavData();

        auto found = filename.find_last_of('/');
        if (found == std::string::npos)
        {
            found = filename.find_last_of('\\');
        }
        if (found == std::string::npos)
        {
            result->name = filename;
        }
        else
        {
            result->name = filename.substr(found + 1);
        }
        result->path = filename;
        result->PwavData = PwavData;
        result->channels = channels;
        result->samplesPerChannel = samplesPerChannel;

        return result;
    }

    free(PwavData);

    return nullptr;
}

std::string WavData::toBase64(WavData *wavData)
{
    return base64_encode(reinterpret_cast<const unsigned char *>(wavData->PwavData), wavData->samplesPerChannel * wavData->channels * sizeof(float));
}

bool WavData::fromBase64(std::string const &data, WavData *wavData)
{
    auto size = static_cast<size_t>(wavData->samplesPerChannel) * wavData->channels * sizeof(float);
    wavData->PwavData = reinterpret_cast<float *>(malloc(size));

    memcpy(wavData->PwavData, reinterpret_cast<float const *>(base64_decode(data).c_str()), size);

    return true;
}
