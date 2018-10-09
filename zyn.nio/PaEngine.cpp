/*
  ZynAddSubFX - a software synthesizer

  PaEngine.cpp - Audio output for PortAudio
  Copyright (C) 2002 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include "PaEngine.h"

#include <iostream>

using namespace std;

PaEngine::PaEngine(unsigned int sampleRate, unsigned int bufferSize)
    : AudioOutput(sampleRate, bufferSize), stream(nullptr)
{
    _name = "PA";
}

PaEngine::~PaEngine()
{
    Stop();
}

bool PaEngine::Start()
{
    if (IsAudioEnabled())
    {
        return true;
    }

    Pa_Initialize();

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice)
    {
        cerr << "Error: No default output device." << endl;
        Pa_Terminate();
        return false;
    }
    outputParameters.channelCount = 2;         /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    Pa_OpenStream(&stream,
                  nullptr,
                  &outputParameters,
                  this->SampleRate(),
                  this->BufferSize(),
                  0,
                  PAprocess,
                  reinterpret_cast<void *>(this));
    Pa_StartStream(stream);
    return true;
}

bool PaEngine::IsAudioEnabled() const
{
    return stream;
}

int PaEngine::PAprocess(const void * /*inputBuffer*/,
                        void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo * /*outTime*/,
                        PaStreamCallbackFlags /*flags*/,
                        void *userData)
{
    auto engine = static_cast<PaEngine *>(userData);

    return engine->process(reinterpret_cast<float *>(outputBuffer), framesPerBuffer);
}

int PaEngine::process(float *out, unsigned long framesPerBuffer)
{
    auto smp = NextSample();

    for (unsigned i = 0; i < framesPerBuffer; ++i)
    {
        *out++ = smp._left[i];
        *out++ = smp._right[i];
    }

    return 0;
}

void PaEngine::Stop()
{
    if (!IsAudioEnabled())
    {
        return;
    }
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    stream = nullptr;
    Pa_Terminate();
}
