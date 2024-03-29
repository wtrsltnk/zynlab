/*
  ZynAddSubFX - a software synthesizer

  FFTwrapper.c  -  A wrapper for Fast Fourier Transforms
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

#include "FFTwrapper.h"
#include <cmath>
#include <cstring>
#include <memory>
#include <zyn.common/globals.h>

static FFTwrapper instance = FFTwrapper(SystemSettings::Instance().oscilsize);

IFFTwrapper *IFFTwrapper::GlobalInstance()
{
    return &instance;
}

IFFTwrapper::~IFFTwrapper() = default;

FFTwrapper::FFTwrapper(unsigned int fftsize_)
{
    fftsize = fftsize_;
    time = new fftw_real[fftsize];
    fft = new fftw_complex[fftsize + 1];
    planfftw = fftw_plan_dft_r2c_1d(static_cast<int>(fftsize), time, fft, FFTW_ESTIMATE);
    planfftw_inv = fftw_plan_dft_c2r_1d(static_cast<int>(fftsize), fft, time, FFTW_ESTIMATE);
}

FFTwrapper::~FFTwrapper()
{
    fftw_destroy_plan(planfftw_inv);
    fftw_destroy_plan(planfftw);
    fftw_cleanup();

    delete[] time;
    delete[] fft;
}

void FFTwrapper::smps2freqs(const float *smps, fft_t *freqs)
{
    //Load data
    for (unsigned int i = 0; i < fftsize; ++i)
    {
        time[i] = static_cast<double>(smps[i]);
    }

    //DFT
    fftw_execute(planfftw);

    //Grab data
    memcpy(static_cast<void *>(freqs), static_cast<const void *>(fft), fftsize * sizeof(double));
}

void FFTwrapper::freqs2smps(const fft_t *freqs, float *smps)
{
    //Load data
    memcpy(static_cast<void *>(fft), static_cast<const void *>(freqs), fftsize * sizeof(double));

    //clear unused freq channel
    fft[fftsize / 2][0] = 0.0;
    fft[fftsize / 2][1] = 0.0;

    //IDFT
    fftw_execute(planfftw_inv);

    //Grab data
    for (unsigned int i = 0; i < fftsize; ++i)
    {
        smps[i] = static_cast<float>(time[i]);
    }
}
