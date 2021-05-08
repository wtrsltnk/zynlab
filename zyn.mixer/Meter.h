#ifndef METER_H
#define METER_H

#include <mutex>
#include <zyn.common/globals.h>

class Meter : public IMeter
{
public:
    Meter();
    virtual ~Meter();

    void Setup();

    void Tick(const float *outl, const float *outr, class Track *part, float volume);

    virtual void SetFakePeak(int instrument, unsigned char velocity);
    virtual unsigned char GetFakePeak(int instrument);
    virtual float GetOutPeak(int instrument);

    //peaks for VU-meter
    virtual void ResetPeaks();
    //get VU-meter data
    virtual vuData GetVuData();

private:
    vuData _vu;

    float _vuoutpeakpart[NUM_MIXER_TRACKS];
    unsigned char _fakepeakpart[NUM_MIXER_TRACKS];

    std::mutex _vumutex;
};

#endif // METER_H
