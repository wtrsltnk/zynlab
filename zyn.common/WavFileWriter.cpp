/*
  Copyright (C) 2006 Nasca Octavian Paul
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

#include "WavFileWriter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

WavFileWriter::WavFileWriter(const string &filename, unsigned int samplerate, unsigned short int channels)
    : _filename(filename)
{
    _audioFile.setSampleRate(samplerate);
    _audioFile.setNumChannels(channels);

    for (int i = 0; i < channels; i++)
        _audioBuffer.push_back(std::vector<float>());
}

WavFileWriter::~WavFileWriter()
{
    _audioFile.setAudioBuffer(_audioBuffer);
    _audioFile.save(_filename, AudioFileFormat::Wave);
}

void WavFileWriter::addSample(std::vector<float> sample)
{
    assert(_audioFile.getNumChannels() == sample.size());

    for (int i = 0; i < _audioFile.getNumChannels(); i++)
        _audioBuffer[i].push_back(sample[i]);
}
