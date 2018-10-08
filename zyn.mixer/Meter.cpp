#include "Meter.h"

#include "Instrument.h"
#include <cmath>

vuData::vuData()
    : outpeakl(0.0f), outpeakr(0.0f), maxoutpeakl(0.0f), maxoutpeakr(0.0f),
      rmspeakl(0.0f), rmspeakr(0.0f), clipped(0)
{}

Meter::Meter(SystemSettings *synth)
    : _synth(synth)
{
    pthread_mutex_init(&vumutex, nullptr);
    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        vuoutpeakpart[npart] = 1e-9f;
        fakepeakpart[npart] = 0;
    }
}

Meter::~Meter()
{
    pthread_mutex_destroy(&vumutex);
}

IMeter::~IMeter() = default;

/*
 * Reset peaks and clear the "cliped" flag (for VU-meter)
 */
void Meter::ResetPeaks()
{
    pthread_mutex_lock(&vumutex);
    vu.outpeakl = 1e-9f;
    vu.outpeakr = 1e-9f;
    vu.maxoutpeakl = 1e-9f;
    vu.maxoutpeakr = 1e-9f;
    vu.clipped = 0;
    pthread_mutex_unlock(&vumutex);
}

vuData Meter::GetVuData()
{
    vuData tmp;
    pthread_mutex_lock(&vumutex);
    tmp = vu;
    pthread_mutex_unlock(&vumutex);
    return tmp;
}

void Meter::SetFakePeak(int instrument, unsigned char velocity)
{
    fakepeakpart[instrument] = velocity;
}

unsigned char Meter::GetFakePeak(int instrument)
{
    pthread_mutex_lock(&vumutex);
    unsigned char peak = fakepeakpart[instrument];
    pthread_mutex_unlock(&vumutex);

    return peak;
}

float Meter::GetOutPeak(int instrument)
{
    pthread_mutex_lock(&vumutex);
    float db = vuoutpeakpart[instrument];
    pthread_mutex_unlock(&vumutex);

    return db;
}

void Meter::Tick(const float *outl, const float *outr, Instrument *part, float volume)
{
    if (pthread_mutex_trylock(&vumutex))
    {
        return;
    }

    //Peak computation (for vumeters)
    vu.outpeakl = 1e-12f;
    vu.outpeakr = 1e-12f;
    for (unsigned int i = 0; i < _synth->buffersize; ++i)
    {
        if (std::fabs(outl[i]) > vu.outpeakl)
        {
            vu.outpeakl = std::fabs(outl[i]);
        }
        if (std::fabs(outr[i]) > vu.outpeakr)
        {
            vu.outpeakr = std::fabs(outr[i]);
        }
    }
    if ((vu.outpeakl > 1.0f) || (vu.outpeakr > 1.0f))
    {
        vu.clipped = 1;
    }
    if (vu.maxoutpeakl < vu.outpeakl)
    {
        vu.maxoutpeakl = vu.outpeakl;
    }
    if (vu.maxoutpeakr < vu.outpeakr)
    {
        vu.maxoutpeakr = vu.outpeakr;
    }

    //RMS Peak computation (for vumeters)
    vu.rmspeakl = 1e-12f;
    vu.rmspeakr = 1e-12f;
    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        vu.rmspeakl += outl[i] * outl[i];
        vu.rmspeakr += outr[i] * outr[i];
    }
    vu.rmspeakl = sqrt(vu.rmspeakl / _synth->buffersize_f);
    vu.rmspeakr = sqrt(vu.rmspeakr / _synth->buffersize_f);

    //Part Peak computation (for Part vumeters or fake part vumeters)
    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        vuoutpeakpart[npart] = 1.0e-12f;
        if (part[npart].Penabled != 0)
        {
            float *outl = part[npart].partoutl,
                  *outr = part[npart].partoutr;
            for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
            {
                float tmp = fabs(outl[i] + outr[i]);
                if (tmp > vuoutpeakpart[npart])
                {
                    vuoutpeakpart[npart] = tmp;
                }
            }
            vuoutpeakpart[npart] *= volume;
        }
        else if (fakepeakpart[npart] > 1)
        {
            fakepeakpart[npart]--;
        }
    }

    pthread_mutex_unlock(&vumutex);
}
