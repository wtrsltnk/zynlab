/*
  ZynAddSubFX - a software synthesizer

  OscilGen.cpp - Waveform generator for ADnote
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

#include "OscilGenSerializer.h"

OscilGenSerializer::OscilGenSerializer(OscilGen *parameters)
    : _parameters(parameters)
{}

OscilGenSerializer::~OscilGenSerializer() = default;

inline void clearDC(fft_t *freqs)
{
    freqs[0] = fft_t(0.0, 0.0);
}

//return magnitude squared
inline float normal(const fft_t *freqs, off_t x)
{
    return norm(freqs[x]);
}

/**
 * Take frequency spectrum and ensure values are normalized based upon
 * magnitude to 0<=x<=1
 */
void normalize(fft_t *freqs)
{
    float normMax = 0.0f;
    for (unsigned int i = 0; i < SystemSettings::Instance().oscilsize / 2; ++i)
    {
        //magnitude squared
        const float norm = normal(freqs, i);
        if (normMax < norm)
            normMax = norm;
    }

    const float max = std::sqrt(normMax);
    if (max < 1e-8) //data is all ~zero, do not amplify noise
        return;

    for (unsigned int i = 0; i < SystemSettings::Instance().oscilsize / 2; ++i)
        freqs[i] /= max;
}

inline void normalize(float *smps, size_t N)
{
    //Find max
    float max = 0.0f;
    for (size_t i = 0; i < N; ++i)
        if (max < std::fabs(smps[i]))
            max = std::fabs(smps[i]);
    if (max < 0.00001f)
        max = 1.0f;

    //Normalize to +-1
    for (size_t i = 0; i < N; ++i)
        smps[i] /= max;
}

void OscilGen::Serialize(IPresetsSerializer *xml)
{
    xml->addpar("harmonic_mag_type", Phmagtype);

    xml->addpar("base_function", Pcurrentbasefunc);
    xml->addpar("base_function_par", Pbasefuncpar);
    xml->addpar("base_function_modulation", Pbasefuncmodulation);
    xml->addpar("base_function_modulation_par1", Pbasefuncmodulationpar1);
    xml->addpar("base_function_modulation_par2", Pbasefuncmodulationpar2);
    xml->addpar("base_function_modulation_par3", Pbasefuncmodulationpar3);

    xml->addpar("modulation", Pmodulation);
    xml->addpar("modulation_par1", Pmodulationpar1);
    xml->addpar("modulation_par2", Pmodulationpar2);
    xml->addpar("modulation_par3", Pmodulationpar3);

    xml->addpar("wave_shaping", Pwaveshaping);
    xml->addpar("wave_shaping_function", Pwaveshapingfunction);

    xml->addpar("filter_type", Pfiltertype);
    xml->addpar("filter_par1", Pfilterpar1);
    xml->addpar("filter_par2", Pfilterpar2);
    xml->addpar("filter_before_wave_shaping", Pfilterbeforews);

    xml->addpar("spectrum_adjust_type", Psatype);
    xml->addpar("spectrum_adjust_par", Psapar);

    xml->addpar("rand", Prand);
    xml->addpar("amp_rand_type", Pamprandtype);
    xml->addpar("amp_rand_power", Pamprandpower);

    xml->addpar("harmonic_shift", Pharmonicshift);
    xml->addparbool("harmonic_shift_first", Pharmonicshiftfirst);

    xml->addpar("adaptive_harmonics", Padaptiveharmonics);
    xml->addpar("adaptive_harmonics_base_frequency", Padaptiveharmonicsbasefreq);
    xml->addpar("adaptive_harmonics_power", Padaptiveharmonicspower);

    xml->beginbranch("HARMONICS");
    for (int n = 0; n < MAX_AD_HARMONICS; ++n)
    {
        if ((Phmag[n] == 64) && (Phphase[n] == 64) && xml->minimal)
        {
            continue;
        }

        xml->beginbranch("HARMONIC", n + 1);
        xml->addpar("mag", Phmag[n]);
        xml->addpar("phase", Phphase[n]);
        xml->endbranch();
    }
    xml->endbranch();

    if (Pcurrentbasefunc == 127)
    {
        normalize(basefuncFFTfreqs);

        xml->beginbranch("BASE_FUNCTION");
        for (unsigned int i = 1; i < SystemSettings::Instance().oscilsize / 2; ++i)
        {
            double xc = basefuncFFTfreqs[i].real();
            double xs = basefuncFFTfreqs[i].imag();
            if ((std::fabs(xs) > 0.00001) && (std::fabs(xc) > 0.00001))
            {
                xml->beginbranch("BF_HARMONIC", static_cast<int>(i));
                xml->addparreal("cos", static_cast<float>(xc));
                xml->addparreal("sin", static_cast<float>(xs));
                xml->endbranch();
            }
        }
        xml->endbranch();
    }
}

void OscilGen::Deserialize(IPresetsSerializer *xml)
{
    Phmagtype = xml->getpar127("harmonic_mag_type", Phmagtype);

    Pcurrentbasefunc = xml->getpar127("base_function", Pcurrentbasefunc);
    Pbasefuncpar = xml->getpar127("base_function_par", Pbasefuncpar);

    Pbasefuncmodulation = xml->getpar127("base_function_modulation",
                                         Pbasefuncmodulation);
    Pbasefuncmodulationpar1 = xml->getpar127("base_function_modulation_par1",
                                             Pbasefuncmodulationpar1);
    Pbasefuncmodulationpar2 = xml->getpar127("base_function_modulation_par2",
                                             Pbasefuncmodulationpar2);
    Pbasefuncmodulationpar3 = xml->getpar127("base_function_modulation_par3",
                                             Pbasefuncmodulationpar3);

    Pmodulation = xml->getpar127("modulation", Pmodulation);
    Pmodulationpar1 = xml->getpar127("modulation_par1",
                                     Pmodulationpar1);
    Pmodulationpar2 = xml->getpar127("modulation_par2",
                                     Pmodulationpar2);
    Pmodulationpar3 = xml->getpar127("modulation_par3",
                                     Pmodulationpar3);

    Pwaveshaping = xml->getpar127("wave_shaping", Pwaveshaping);
    Pwaveshapingfunction = xml->getpar127("wave_shaping_function",
                                          Pwaveshapingfunction);

    Pfiltertype = xml->getpar127("filter_type", Pfiltertype);
    Pfilterpar1 = xml->getpar127("filter_par1", Pfilterpar1);
    Pfilterpar2 = xml->getpar127("filter_par2", Pfilterpar2);
    Pfilterbeforews = xml->getpar127("filter_before_wave_shaping",
                                     Pfilterbeforews);

    Psatype = xml->getpar127("spectrum_adjust_type", Psatype);
    Psapar = xml->getpar127("spectrum_adjust_par", Psapar);

    Prand = xml->getpar127("rand", Prand);
    Pamprandtype = xml->getpar127("amp_rand_type", Pamprandtype);
    Pamprandpower = xml->getpar127("amp_rand_power", Pamprandpower);

    Pharmonicshift = xml->getpar("harmonic_shift",
                                 Pharmonicshift,
                                 -64,
                                 64);
    Pharmonicshiftfirst = xml->getparbool("harmonic_shift_first",
                                          Pharmonicshiftfirst);

    Padaptiveharmonics = xml->getpar("adaptive_harmonics",
                                     Padaptiveharmonics,
                                     0,
                                     127);
    Padaptiveharmonicsbasefreq = xml->getpar(
        "adaptive_harmonics_base_frequency",
        Padaptiveharmonicsbasefreq,
        0,
        255);
    Padaptiveharmonicspower = xml->getpar("adaptive_harmonics_power",
                                          Padaptiveharmonicspower,
                                          0,
                                          200);

    if (xml->enterbranch("HARMONICS"))
    {
        Phmag[0] = 64;
        Phphase[0] = 64;
        for (int n = 0; n < MAX_AD_HARMONICS; ++n)
        {
            if (xml->enterbranch("HARMONIC", n + 1) == 0)
                continue;
            Phmag[n] = xml->getpar127("mag", 64);
            Phphase[n] = xml->getpar127("phase", 64);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (Pcurrentbasefunc != 0)
        changebasefunction();

    if (xml->enterbranch("BASE_FUNCTION"))
    {
        for (int i = 1; i < SystemSettings::Instance().oscilsize / 2; ++i)
            if (xml->enterbranch("BF_HARMONIC", i))
            {
                basefuncFFTfreqs[i] =
                    std::complex<float>(xml->getparreal("cos", 0.0f),
                                        xml->getparreal("sin", 0.0f));
                xml->exitbranch();
            }
        xml->exitbranch();

        clearDC(basefuncFFTfreqs);
        normalize(basefuncFFTfreqs);
    }
}
