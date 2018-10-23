#ifndef METER_H
#define METER_H

#include <pthread.h>
#include <zyn.common/globals.h>

class Meter : public IMeter
{
public:
    Meter(SystemSettings *synth);
    virtual ~Meter();

    void Tick(const float *outl, const float *outr, class Instrument *part, float volume);

    virtual void SetFakePeak(int instrument, unsigned char velocity);
    virtual unsigned char GetFakePeak(int instrument);
    virtual float GetOutPeak(int instrument);

    //peaks for VU-meter
    virtual void ResetPeaks();
    //get VU-meter data
    virtual vuData GetVuData();

private:
    vuData vu;

    float vuoutpeakpart[NUM_MIXER_CHANNELS]{};
    unsigned char fakepeakpart[NUM_MIXER_CHANNELS]{};

    pthread_mutex_t vumutex{};
    SystemSettings *_synth;
};

#endif // METER_H