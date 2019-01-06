#ifndef METER_H
#define METER_H

#include <pthread.h>
#include <zyn.common/globals.h>

class Meter : public IMeter
{
public:
    Meter();
    virtual ~Meter();

    void Setup();

    void Tick(const float *outl, const float *outr, class Channel *part, float volume);

    virtual void SetFakePeak(int instrument, unsigned char velocity);
    virtual unsigned char GetFakePeak(int instrument);
    virtual float GetOutPeak(int instrument);

    //peaks for VU-meter
    virtual void ResetPeaks();
    //get VU-meter data
    virtual vuData GetVuData();

private:
    vuData _vu;

    float _vuoutpeakpart[NUM_MIXER_CHANNELS];
    unsigned char _fakepeakpart[NUM_MIXER_CHANNELS];

    pthread_mutex_t _vumutex;
};

#endif // METER_H
