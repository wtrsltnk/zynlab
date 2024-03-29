/*
  ZynAddSubFX - a software synthesizer

  WavEngine.h - Records sound to a file
  Copyright (C) 2008 Nasca Octavian Paul
  Author: Nasca Octavian Paul
          Mark McCurry

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

#ifndef WAVENGINE_H
#define WAVENGINE_H

#include "AudioOutput.h"
#include "SafeQueue.h"
#include "ZynSema.h"

#include <string>
#include <thread>

class WavFileWriter;
class WavEngine : public AudioOutput
{
public:
    WavEngine(
        unsigned int sampleRate,
        unsigned int bufferSize);

    virtual ~WavEngine();

    bool openAudio();

    bool Start();

    void Stop();

    bool IsAudioEnabled() const { return true; }

    void push(
        Stereo<float *> smps,
        size_t len);

    void newFile(
        WavFileWriter *_file);

    void destroyFile();

protected:
    void *AudioThread();

    static void *_AudioThread(
        void *arg);

private:
    WavFileWriter *file;
    ZynSema work;
    SafeQueue<float> buffer;

    std::thread *_thread = nullptr;
};
#endif
