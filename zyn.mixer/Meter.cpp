#include "Meter.h"

#include "Channel.h"
#include <cmath>

vuData::vuData()
    : outpeakl(0.0f), outpeakr(0.0f), maxoutpeakl(0.0f), maxoutpeakr(0.0f),
      rmspeakl(0.0f), rmspeakr(0.0f), clipped(0)
{}

Meter::Meter()
{
}

Meter::~Meter()
{
    pthread_mutex_destroy(&_vumutex);
}

IMeter::~IMeter() = default;

void Meter::Setup()
{
    pthread_mutex_init(&_vumutex, nullptr);
    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        _vuoutpeakpart[npart] = 1e-9f;
        _fakepeakpart[npart] = 0;
    }
}

/*
 * Reset peaks and clear the "cliped" flag (for VU-meter)
 */
void Meter::ResetPeaks()
{
    pthread_mutex_lock(&_vumutex);
    _vu.outpeakl = 1e-9f;
    _vu.outpeakr = 1e-9f;
    _vu.maxoutpeakl = 1e-9f;
    _vu.maxoutpeakr = 1e-9f;
    _vu.clipped = 0;
    pthread_mutex_unlock(&_vumutex);
}

vuData Meter::GetVuData()
{
    vuData tmp;
    pthread_mutex_lock(&_vumutex);
    tmp = _vu;
    pthread_mutex_unlock(&_vumutex);
    return tmp;
}

void Meter::SetFakePeak(int instrument, unsigned char velocity)
{
    _fakepeakpart[instrument] = velocity;
}

unsigned char Meter::GetFakePeak(int instrument)
{
    pthread_mutex_lock(&_vumutex);
    unsigned char peak = _fakepeakpart[instrument];
    pthread_mutex_unlock(&_vumutex);

    return peak;
}

float Meter::GetOutPeak(int instrument)
{
    pthread_mutex_lock(&_vumutex);
    float db = _vuoutpeakpart[instrument];
    pthread_mutex_unlock(&_vumutex);

    return db;
}

void Meter::Tick(const float *outl, const float *outr, Channel *part, float volume)
{
    if (pthread_mutex_trylock(&_vumutex))
    {
        return;
    }

    //Peak computation (for vumeters)
    _vu.outpeakl = 1e-12f;
    _vu.outpeakr = 1e-12f;
    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        if (std::fabs(outl[i]) > _vu.outpeakl)
        {
            _vu.outpeakl = std::fabs(outl[i]);
        }
        if (std::fabs(outr[i]) > _vu.outpeakr)
        {
            _vu.outpeakr = std::fabs(outr[i]);
        }
    }
    if ((_vu.outpeakl > 1.0f) || (_vu.outpeakr > 1.0f))
    {
        _vu.clipped = 1;
    }
    if (_vu.maxoutpeakl < _vu.outpeakl)
    {
        _vu.maxoutpeakl = _vu.outpeakl;
    }
    if (_vu.maxoutpeakr < _vu.outpeakr)
    {
        _vu.maxoutpeakr = _vu.outpeakr;
    }

    //RMS Peak computation (for vumeters)
    _vu.rmspeakl = 1e-12f;
    _vu.rmspeakr = 1e-12f;
    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        _vu.rmspeakl += outl[i] * outl[i];
        _vu.rmspeakr += outr[i] * outr[i];
    }
    _vu.rmspeakl = std::sqrt(_vu.rmspeakl / SystemSettings::Instance().buffersize_f);
    _vu.rmspeakr = std::sqrt(_vu.rmspeakr / SystemSettings::Instance().buffersize_f);

    //Part Peak computation (for Part vumeters or fake part vumeters)
    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        _vuoutpeakpart[npart] = part[npart].ComputePeak(volume);
        if (!part[npart].Penabled && _fakepeakpart[npart] > 1)
        {
            _fakepeakpart[npart]--;
        }
    }

    pthread_mutex_unlock(&_vumutex);
}
