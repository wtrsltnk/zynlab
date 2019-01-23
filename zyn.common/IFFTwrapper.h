#ifndef IFFTWRAPPER_H
#define IFFTWRAPPER_H

#include <complex>

typedef double fftw_real;
typedef std::complex<fftw_real> fft_t;

class IFFTwrapper
{
public:
    virtual ~IFFTwrapper();

    /**Convert Samples to Frequencies using Fourier Transform
     * @param smps Pointer to Samples to be converted; has length fftsize_
     * @param freqs Structure FFTFREQS which stores the frequencies*/
    virtual void smps2freqs(const float *smps, fft_t *freqs) = 0;
    virtual void freqs2smps(const fft_t *freqs, float *smps) = 0;
};

#endif // IFFTWRAPPER_H
