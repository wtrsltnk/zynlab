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

#include "WavFileReader.h"

#include <iostream>
#include <stdio.h>
#include <string.h>

WavFileReader::WavFileReader() = default;

WavFileReader::~WavFileReader() = default;

bool WavFileReader::Read(char const *filename, Wav::WAVDATA *target)
{
    if (filename == nullptr)
    {
        std::cout << "no valid filename given" << std::endl;
        return false;
    }

    auto file = fopen(filename, "rb");

    if (file == nullptr)
    {
        std::cout << filename << " is not found" << std::endl;
        return false;
    }

    Wav::HEADER header;

    auto read = fread(&header, sizeof(Wav::HEADER), 1, file);

    if (read != 1)
    {
        std::cout << filename << " is not a supported WAV file" << std::endl;
        fclose(file);
        return false;
    }

    if (strncmp("RIFF", (char *)header.riff, 4) != 0)
    {
        std::cout << filename << " is not a supported WAV file (wrong riff: " << header.riff << ")" << std::endl;
        fclose(file);
        return false;
    }

    if (strncmp("WAVE", (char *)header.wave, 4) != 0)
    {
        std::cout << filename << " is not a supported WAV file (wrong wave: " << header.wave << ")" << std::endl;
        fclose(file);
        return false;
    }

    if (strncmp("fmt", (char *)header.fmt_chunk_marker, 3) != 0)
    {
        std::cout << filename << " is not a supported WAV file (wrong fmt: " << header.fmt_chunk_marker << ")" << std::endl;
        fclose(file);
        return false;
    }

    if (header.format_type != 1)
    {
        std::cout << filename << " is not a supported WAV file (unsupported format: " << header.format_type << ")" << std::endl;
        fclose(file);
        return false;
    }

    if (header.bits_per_sample != 16 && header.bits_per_sample != 8)
    {
        std::cout << filename << " is not a supported WAV file (bad bits per sample: " << header.bits_per_sample << ")" << std::endl;
        fclose(file);
        return false;
    }

    fseek(file, long(header.length_of_fmt - (sizeof(Wav::HEADER) - 20)), SEEK_CUR);

    Wav::CHUNK chunkHeader;
    auto sFlag = 1;
    while (sFlag != 0)
    {
        // check attempts.
        if (sFlag > 10)
        {
            std::cout << "Too many chunks" << std::endl;
            fclose(file);
            return false;
        }

        // read chunk header
        read = fread(&chunkHeader, 1, sizeof(Wav::CHUNK), file);
        if (read != sizeof(Wav::CHUNK))
        {
            std::cout << "I just can't read data. Sorry!" << std::endl;
            fclose(file);
            return false;
        }

        // check chunk type.
        if (strncmp("data", chunkHeader.id, 4) == 0)
        {
            break;
        }

        // skip over chunk.
        sFlag++;
        read = fseek(file, chunkHeader.length, SEEK_CUR);
        if (read != 0)
        {
            std::cout << "Can't seek." << std::endl;
            fclose(file);
            return false;
        }
    }

    auto maxInSamples = chunkHeader.length / (header.bits_per_sample / 8);

    auto data = new char[chunkHeader.length];
    target->_samples = new float[maxInSamples];

    read = fread(data, chunkHeader.length, 1, file);

    if (read != 1)
    {
        std::cout << "wrong chunk length" << std::endl;
        fclose(file);
        return false;
    }

    if (header.bits_per_sample == 16)
    {
        auto pU = reinterpret_cast<short *>(data);
        for (unsigned int i = 0; i < maxInSamples; i++)
        {
            target->_samples[i] = (float)(pU[i]);
        }
    }
    else if (header.bits_per_sample == 8)
    {
        auto pC = reinterpret_cast<unsigned char *>(data);
        for (unsigned int i = 0; i < maxInSamples; i++)
        {
            target->_samples[i] = (float)(pC[i]);
        }
    }

    target->_bits_per_sample = header.bits_per_sample;
    target->_channels = header.channels;
    target->_sample_rate = header.sample_rate;
    target->_sample_count = maxInSamples;

    delete[] data;

    fclose(file);

    std::cout << filename << " read successful, this sample has a sample rate of " << target->_sample_rate << std::endl;

    return true;
}
