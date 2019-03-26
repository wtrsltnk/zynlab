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

#include <string>

class WavFileWriter
{
public:
    WavFileWriter(const std::string &filename, unsigned int samplerate, unsigned short int channels);
    ~WavFileWriter();

    bool good() const;

    void writeMonoSamples(unsigned int nsmps, short int *smps);
    void writeStereoSamples(unsigned int nsmps, short int *smps);

private:
    unsigned int sampleswritten;
    unsigned int samplerate;
    unsigned short int channels;
    FILE *file;
};

#endif // WAVFILEWRITER_H
