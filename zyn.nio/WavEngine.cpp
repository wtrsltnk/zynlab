/*
  Copyright (C) 2006 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#include "WavEngine.h"
#include <zyn.common/Util.h>
#include <zyn.common/WavFileWriter.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;

WavEngine::WavEngine(
    unsigned int sampleRate,
    unsigned int bufferSize)
    : AudioOutput(sampleRate, bufferSize),
      file(nullptr), buffer(sampleRate * 4)
{
    _name = "WAV";
    work.init(0 /*PROCESS_PRIVATE*/, 0);
}

WavEngine::~WavEngine()
{
    Stop();
    destroyFile();
}

bool WavEngine::openAudio()
{
    return file && file->good();
}

bool WavEngine::Start()
{
    if (_thread != nullptr)
    {
        return true;
    }

    _thread = new std::thread(_AudioThread, this);

    std::cout << "start recording" << std::endl;

    return true;
}

void WavEngine::Stop()
{
    if (_thread == nullptr)
    {
        return;
    }

    std::cout << "stop recording" << std::endl;

    auto tmp = _thread;
    _thread = nullptr;

    work.post();

    tmp->join();

    delete tmp;
}

void WavEngine::push(Stereo<float *> smps, size_t len)
{
    if (_thread == nullptr)
    {
        return;
    }

    //copy the input [overflow when needed]
    for (size_t i = 0; i < len; ++i)
    {
        buffer.push(*smps._left++);
        buffer.push(*smps._right++);
    }

    work.post();
}

void WavEngine::newFile(
    WavFileWriter *_file)
{
    //ensure system is clean
    destroyFile();
    file = _file;

    //check state
    if (!file->good())
    {
        std::cerr
            << "ERROR: WavEngine handed bad file output WavEngine::newFile()"
            << std::endl;
    }
}

void WavEngine::destroyFile()
{
    if (file != nullptr)
    {
        delete file;
    }
    file = nullptr;
}

void *WavEngine::_AudioThread(
    void *arg)
{
    return (static_cast<WavEngine *>(arg))->AudioThread();
}

void *WavEngine::AudioThread()
{
    short *recordbuf_16bit = new short[2 * _bufferSize];

    while (!work.wait() && _thread != nullptr)
    {
        for (unsigned int i = 0; i < _bufferSize; ++i)
        {
            float left = 0.0f, right = 0.0f;
            buffer.pop(left);
            buffer.pop(right);
            recordbuf_16bit[2 * i] = limit(static_cast<int>(left * 32767.0f),
                                           -32768,
                                           32767);
            recordbuf_16bit[2 * i + 1] = limit(static_cast<int>(right * 32767.0f),
                                               -32768,
                                               32767);
        }
        if (file != nullptr)
        {
            file->writeStereoSamples(_bufferSize, recordbuf_16bit);
        }
    }

    delete[] recordbuf_16bit;

    return nullptr;
}
