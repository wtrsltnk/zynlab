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

#ifndef WAVFILEREADER_H
#define WAVFILEREADER_H

#include "WavFile.h"

class WavFileReader
{
public:
    WavFileReader();
    virtual ~WavFileReader();

    bool Read(char const *filename, Wav::WAVDATA *target);
};

#endif // WAVFILEREADER_H
