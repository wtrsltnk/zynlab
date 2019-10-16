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

#include "dr_wav.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

/*
WavFileWriter::WavFileWriter(const string &filename, unsigned int samplerate, unsigned short int channels)
    : _filename(filename)
{}

WavFileWriter::~WavFileWriter()
{
    drwav_data_format format;
    format.container = drwav_container_riff; // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;      // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = 2;
    format.sampleRate = 44100;
    format.bitsPerSample = 16;
    auto f = drwav_open_file_write(_filename.c_str(), &format);
    drwav_write_pcm_frames(f, _audioBuffer.size(), _audioBuffer.data());
}

bool WavFileWriter::good() const { return true; }

void WavFileWriter::writeStereoSamples(int nsmps, short int *smps)
{
    for (int i = 0; i < nsmps; i++)
    {
        _audioBuffer.push_back(smps[i * 2]);
        _audioBuffer.push_back(smps[i * 2 + 1]);
    }
}

void WavFileWriter::writeMonoSamples(int nsmps, short int *smps)
{
    for (int i = 0; i < nsmps; i++)
    {
        _audioBuffer.push_back(smps[i]);
    }
}

void WavFileWriter::addSample(std::vector<float> sample)
{
}

/*/
WavFileWriter::WavFileWriter(string filename, unsigned int samplerate, unsigned int channels)
    : sampleswritten(0), samplerate(samplerate), channels(channels), file(fopen(filename.c_str(), "w"))
{
    if(file)
    {
        cout << "INFO: Making space for wave file header" << endl;
        //making space for the header written at destruction
        char tmp[44];
        memset(tmp, 0, 44 * sizeof(char));
        fwrite(tmp, 1, 44, file);
    }
}

WavFileWriter::~WavFileWriter()
{
    if(file)
    {
        cout << "INFO: Writing wave file header" << endl;

        unsigned int chunksize;
        rewind(file);

        fwrite("RIFF", 4, 1, file);
        chunksize = sampleswritten * 4 + 36;
        fwrite(&chunksize, 4, 1, file);

        fwrite("WAVEfmt ", 8, 1, file);
        chunksize = 16;
        fwrite(&chunksize, 4, 1, file);
        unsigned short int formattag = 1;     //uncompresed wave
        fwrite(&formattag, 2, 1, file);
        unsigned short int nchannels = static_cast<unsigned short int>(channels);     //stereo
        fwrite(&nchannels, 2, 1, file);
        unsigned int samplerate_ = samplerate;         //samplerate
        fwrite(&samplerate_, 4, 1, file);
        unsigned int bytespersec = samplerate * 2 * channels;         //bytes/sec
        fwrite(&bytespersec, 4, 1, file);
        unsigned short int blockalign = static_cast<unsigned short int>(2 * channels);    //2 channels * 16 bits/8
        fwrite(&blockalign, 2, 1, file);
        unsigned short int bitspersample = 16;
        fwrite(&bitspersample, 2, 1, file);

        fwrite("data", 4, 1, file);
        chunksize = sampleswritten * blockalign;
        fwrite(&chunksize, 4, 1, file);

        fclose(file);
        file = nullptr;
    }
}

bool WavFileWriter::good() const
{
    return file;
}

void WavFileWriter::writeStereoSamples(unsigned int nsmps, short int *smps)
{
    if(file)
    {
        fwrite(smps, nsmps, 4, file);
        sampleswritten += nsmps;
    }
}

void WavFileWriter::writeMonoSamples(unsigned int nsmps, short int *smps)
{
    if(file)
    {
        fwrite(smps, nsmps, 2, file);
        sampleswritten += nsmps;
    }
}

//*/
