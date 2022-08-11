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

OscilGenSerializer::OscilGenSerializer(
    OscilGen *parameters)
    : _parameters(parameters)
{}

OscilGenSerializer::~OscilGenSerializer() = default;

namespace Serialization
{

    inline void clearDC(fft_t *freqs)
    {
        freqs[0] = fft_t(0.0, 0.0);
    }

    //return magnitude squared
    inline float normal(
        const fft_t *freqs,
        off_t x)
    {
        return norm(freqs[x]);
    }

    /**
 * Take frequency spectrum and ensure values are normalized based upon
 * magnitude to 0<=x<=1
 */
    void normalize(
        fft_t *freqs)
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

    inline void normalize(
        float *smps,
        size_t N)
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

} // namespace Serialization

void OscilGenSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->addpar("harmonic_mag_type", _parameters->Phmagtype);

    xml->addpar("base_function", _parameters->Pcurrentbasefunc);
    xml->addpar("base_function_par", _parameters->Pbasefuncpar);
    xml->addpar("base_function_modulation", _parameters->Pbasefuncmodulation);
    xml->addpar("base_function_modulation_par1", _parameters->Pbasefuncmodulationpar1);
    xml->addpar("base_function_modulation_par2", _parameters->Pbasefuncmodulationpar2);
    xml->addpar("base_function_modulation_par3", _parameters->Pbasefuncmodulationpar3);

    xml->addpar("modulation", _parameters->Pmodulation);
    xml->addpar("modulation_par1", _parameters->Pmodulationpar1);
    xml->addpar("modulation_par2", _parameters->Pmodulationpar2);
    xml->addpar("modulation_par3", _parameters->Pmodulationpar3);

    xml->addpar("wave_shaping", _parameters->Pwaveshaping);
    xml->addpar("wave_shaping_function", _parameters->Pwaveshapingfunction);

    xml->addpar("filter_type", _parameters->Pfiltertype);
    xml->addpar("filter_par1", _parameters->Pfilterpar1);
    xml->addpar("filter_par2", _parameters->Pfilterpar2);
    xml->addpar("filter_before_wave_shaping", _parameters->Pfilterbeforews);

    xml->addpar("spectrum_adjust_type", _parameters->Psatype);
    xml->addpar("spectrum_adjust_par", _parameters->Psapar);

    xml->addpar("rand", _parameters->Prand);
    xml->addpar("amp_rand_type", _parameters->Pamprandtype);
    xml->addpar("amp_rand_power", _parameters->Pamprandpower);

    xml->addpar("harmonic_shift", _parameters->Pharmonicshift);
    xml->addparbool("harmonic_shift_first", _parameters->Pharmonicshiftfirst);

    xml->addpar("adaptive_harmonics", _parameters->Padaptiveharmonics);
    xml->addpar("adaptive_harmonics_base_frequency", _parameters->Padaptiveharmonicsbasefreq);
    xml->addpar("adaptive_harmonics_power", _parameters->Padaptiveharmonicspower);

    xml->beginbranch("HARMONICS");
    for (int n = 0; n < MAX_AD_HARMONICS; ++n)
    {
        if ((_parameters->Phmag[n] == 64) && (_parameters->Phphase[n] == 64) && xml->minimal)
        {
            continue;
        }

        xml->beginbranch("HARMONIC", n + 1);
        xml->addpar("mag", _parameters->Phmag[n]);
        xml->addpar("phase", _parameters->Phphase[n]);
        xml->endbranch();
    }
    xml->endbranch();

    if (_parameters->Pcurrentbasefunc == 127)
    {
        Serialization::normalize(_parameters->basefuncFFTfreqs);

        xml->beginbranch("BASE_FUNCTION");
        for (unsigned int i = 1; i < SystemSettings::Instance().oscilsize / 2; ++i)
        {
            double xc = _parameters->basefuncFFTfreqs[i].real();
            double xs = _parameters->basefuncFFTfreqs[i].imag();
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

void OscilGenSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    _parameters->Phmagtype = xml->getpar127("harmonic_mag_type", _parameters->Phmagtype);

    _parameters->Pcurrentbasefunc = xml->getpar127("base_function", _parameters->Pcurrentbasefunc);
    _parameters->Pbasefuncpar = xml->getpar127("base_function_par", _parameters->Pbasefuncpar);

    _parameters->Pbasefuncmodulation = xml->getpar127("base_function_modulation", _parameters->Pbasefuncmodulation);
    _parameters->Pbasefuncmodulationpar1 = xml->getpar127("base_function_modulation_par1", _parameters->Pbasefuncmodulationpar1);
    _parameters->Pbasefuncmodulationpar2 = xml->getpar127("base_function_modulation_par2", _parameters->Pbasefuncmodulationpar2);
    _parameters->Pbasefuncmodulationpar3 = xml->getpar127("base_function_modulation_par3", _parameters->Pbasefuncmodulationpar3);

    _parameters->Pmodulation = xml->getpar127("modulation", _parameters->Pmodulation);
    _parameters->Pmodulationpar1 = xml->getpar127("modulation_par1", _parameters->Pmodulationpar1);
    _parameters->Pmodulationpar2 = xml->getpar127("modulation_par2", _parameters->Pmodulationpar2);
    _parameters->Pmodulationpar3 = xml->getpar127("modulation_par3", _parameters->Pmodulationpar3);

    _parameters->Pwaveshaping = xml->getpar127("wave_shaping", _parameters->Pwaveshaping);
    _parameters->Pwaveshapingfunction = xml->getpar127("wave_shaping_function", _parameters->Pwaveshapingfunction);

    _parameters->Pfiltertype = xml->getpar127("filter_type", _parameters->Pfiltertype);
    _parameters->Pfilterpar1 = xml->getpar127("filter_par1", _parameters->Pfilterpar1);
    _parameters->Pfilterpar2 = xml->getpar127("filter_par2", _parameters->Pfilterpar2);
    _parameters->Pfilterbeforews = xml->getpar127("filter_before_wave_shaping", _parameters->Pfilterbeforews);

    _parameters->Psatype = xml->getpar127("spectrum_adjust_type", _parameters->Psatype);
    _parameters->Psapar = xml->getpar127("spectrum_adjust_par", _parameters->Psapar);

    _parameters->Prand = xml->getpar127("rand", _parameters->Prand);
    _parameters->Pamprandtype = xml->getpar127("amp_rand_type", _parameters->Pamprandtype);
    _parameters->Pamprandpower = xml->getpar127("amp_rand_power", _parameters->Pamprandpower);

    _parameters->Pharmonicshift = xml->getpar("harmonic_shift", _parameters->Pharmonicshift, -64, 64);
    _parameters->Pharmonicshiftfirst = xml->getparbool("harmonic_shift_first", _parameters->Pharmonicshiftfirst);

    _parameters->Padaptiveharmonics = xml->getpar("adaptive_harmonics", _parameters->Padaptiveharmonics, 0, 127);
    _parameters->Padaptiveharmonicsbasefreq = xml->getpar("adaptive_harmonics_base_frequency", _parameters->Padaptiveharmonicsbasefreq, 0, 255);
    _parameters->Padaptiveharmonicspower = xml->getpar("adaptive_harmonics_power", _parameters->Padaptiveharmonicspower, 0, 200);

    if (xml->enterbranch("HARMONICS"))
    {
        _parameters->Phmag[0] = 64;
        _parameters->Phphase[0] = 64;
        for (int n = 0; n < MAX_AD_HARMONICS; ++n)
        {
            if (xml->enterbranch("HARMONIC", n + 1) == 0)
            {
                continue;
            }
            _parameters->Phmag[n] = xml->getpar127("mag", 64);
            _parameters->Phphase[n] = xml->getpar127("phase", 64);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (_parameters->Pcurrentbasefunc != 0)
    {
        _parameters->changebasefunction();
    }

    if (xml->enterbranch("BASE_FUNCTION"))
    {
        for (unsigned int i = 1; i < SystemSettings::Instance().oscilsize / 2; ++i)
        {
            if (xml->enterbranch("BF_HARMONIC", i))
            {
                _parameters->basefuncFFTfreqs[i] = std::complex<float>(xml->getparreal("cos", 0.0f), xml->getparreal("sin", 0.0f));
                xml->exitbranch();
            }
        }
        xml->exitbranch();

        Serialization::clearDC(_parameters->basefuncFFTfreqs);
        Serialization::normalize(_parameters->basefuncFFTfreqs);
    }
}
