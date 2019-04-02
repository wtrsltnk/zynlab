#include "WavData.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <system.io/system.io.fileinfo.h>

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

        result->name = System::IO::FileInfo(filename).Name();
        result->PwavData = PwavData;
        result->channels = channels;
        result->samplesPerChannel = samplesPerChannel;

        return result;
    }

    free(PwavData);

    return nullptr;
}
