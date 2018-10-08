#ifndef METER_H
#define METER_H

#include <pthread.h>
#include <zyn.common/globals.h>

struct vuData
{
    vuData();
    float outpeakl, outpeakr, maxoutpeakl, maxoutpeakr, rmspeakl, rmspeakr;
    int clipped;
};

class Meter
{
public:
    Meter(SystemSettings *synth);
    virtual ~Meter();

    void Tick(const float *outl, const float *outr, class Instrument *part, float volume);

    void SetFakePeak(int instrument, unsigned char velocity);
    unsigned char GetFakePeak(int instrument);
    float GetOutPeak(int instrument);

    //peaks for VU-meter
    void ResetPeaks();
    //get VU-meter data
    vuData GetVuData();

private:
    vuData vu;

    float vuoutpeakpart[NUM_MIDI_PARTS]{};
    unsigned char fakepeakpart[NUM_MIDI_PARTS]{};

    pthread_mutex_t vumutex{};
    SystemSettings *_synth;
};

#endif // METER_H
