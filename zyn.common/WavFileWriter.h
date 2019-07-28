/*
  ZynAddSubFX - a software synthesizer

  WavFile.h - Records sound to a file
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

#ifndef WAVFILEWRITER_H
#define WAVFILEWRITER_H

#include "AudioFile.h"
#include <string>
#include <vector>

/*
class WavFileWriter
{
public:
    WavFileWriter(std::string const &filename, unsigned int samplerate, unsigned short int channels);
    virtual ~WavFileWriter();

    void addSample(std::vector<float> sample);

    bool good() const;

    void writeMonoSamples(int nsmps, short int *smps);
    void writeStereoSamples(int nsmps, short int *smps);

private:
    std::string _filename;
    std::vector<short int> _audioBuffer;
};
/*/
class WavFileWriter
{
public:
    WavFileWriter(std::string filename, int samplerate, int channels);
    ~WavFileWriter();

    bool good() const;

    void writeMonoSamples(int nsmps, short int *smps);
    void writeStereoSamples(int nsmps, short int *smps);

private:
    int sampleswritten;
    int samplerate;
    int channels;
    FILE *file;
};
//*/

#endif // WAVFILEWRITER_H
