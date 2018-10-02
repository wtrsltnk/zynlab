/*
  ZynAddSubFX - a software synthesizer

  FFTwrapper.h  -  A wrapper for Fast Fourier Transforms
  Copyright (C) 2002-2005 Nasca Octavian Paul
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

#ifndef FFT_WRAPPER_H
#define FFT_WRAPPER_H

#include <fftw3.h>

#include "ifftwrapper.h"

/**A wrapper for the FFTW library (Fast Fourier Transforms)*/
class FFTwrapper : public IFFTwrapper
{
public:
    /**Constructor
         * @param fftsize The size of samples to be fed to fftw*/
    FFTwrapper(unsigned int fftsize_);

    /**Destructor*/
    virtual ~FFTwrapper();

    /**Convert Samples to Frequencies using Fourier Transform
         * @param smps Pointer to Samples to be converted; has length fftsize_
         * @param freqs Structure FFTFREQS which stores the frequencies*/
    void smps2freqs(const float *smps, fft_t *freqs);
    void freqs2smps(const fft_t *freqs, float *smps);

private:
    unsigned int fftsize;
    fftw_real *time;
    fftw_complex *fft;
    fftw_plan planfftw, planfftw_inv;
};

void FFT_cleanup();

#endif
