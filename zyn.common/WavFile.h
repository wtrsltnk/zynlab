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

#ifndef WAVFILE_H
#define WAVFILE_H

namespace Wav {

// WAVE file header format
typedef struct
{
    char riff[4];               // RIFF string
    unsigned int overall_size;  // overall size of file in bytes
    char wave[4];               // WAVE string
    char fmt_chunk_marker[4];   // fmt string with trailing null char
    unsigned int length_of_fmt; // length of the format data
    short int format_type;      // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    short int channels;         // no.of channels
    unsigned int sample_rate;   // sampling rate (blocks per second)
    unsigned int byterate;      // SampleRate * NumChannels * BitsPerSample/8
    short int block_align;      // NumChannels * BitsPerSample/8
    short int bits_per_sample;  // bits per sample, 8- 8bits, 16- 16 bits etc
} HEADER;

typedef struct
{
    char id[4]; // 'data' or 'fact'
    unsigned int length;
} CHUNK;

typedef struct
{
    float *_samples;
    short int _bits_per_sample;
    short int _channels;
    unsigned int _sample_rate;
    unsigned int _sample_count;

} WAVDATA;

} // namespace Wav

#endif // WAVFILE_H
