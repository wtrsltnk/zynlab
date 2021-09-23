#include <catch2/catch.hpp>
#include <zyn.synth/FFTwrapper.h>

#include <iostream>
#include <memory>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

std::vector<float> generateSamples(int sampleCount)
{
    std::vector<float> result;
    double angle = 0.0;

    for (int i = 0; i < sampleCount; i++)
    {
        auto v = int(3276 * sin(angle) + 32767);

        result.push_back(v / 32767.0f);

        angle += (2 * M_PI) / 100;
    }

    return result;
}

TEST_CASE("FFTW")
{
    const int oscilsize = 1024;

    FFTwrapper fftwWrapper(oscilsize);
    auto oscil = generateSamples(oscilsize);
    auto freqs = new fft_t[oscilsize / 2];

    fftwWrapper.smps2freqs(oscil.data(), freqs);
}
