/*
  ZynAddSubFX - a software synthesizer

  FilterParams.cpp - Parameters for filter
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

#include "FilterParamsSerializer.h"

FilterParamsSerializer::FilterParamsSerializer(
    FilterParams *parameters)
    : _parameters(parameters)
{}

FilterParamsSerializer::~FilterParamsSerializer() = default;

void FilterParamsSerializer::Serialize(
    IPresetsSerializer *xml)
{
    //filter parameters
    xml->addpar("category", _parameters->Pcategory);
    xml->addpar("type", _parameters->Ptype);
    xml->addpar("freq", _parameters->Pfreq);
    xml->addpar("q", _parameters->Pq);
    xml->addpar("stages", _parameters->Pstages);
    xml->addpar("freq_track", _parameters->Pfreqtrack);
    xml->addpar("gain", _parameters->Pgain);

    //formant filter parameters
    if ((_parameters->Pcategory == 1) || (!xml->minimal))
    {
        xml->beginbranch("FORMANT_FILTER");
        xml->addpar("num_formants", _parameters->Pnumformants);
        xml->addpar("formant_slowness", _parameters->Pformantslowness);
        xml->addpar("vowel_clearness", _parameters->Pvowelclearness);
        xml->addpar("center_freq", _parameters->Pcenterfreq);
        xml->addpar("octaves_freq", _parameters->Poctavesfreq);
        for (int nvowel = 0; nvowel < FF_MAX_VOWELS; ++nvowel)
        {
            xml->beginbranch("VOWEL", nvowel);
            for (int nformant = 0; nformant < FF_MAX_FORMANTS; ++nformant)
            {
                xml->beginbranch("FORMANT", nformant);
                xml->addpar("freq", _parameters->Pvowels[nvowel].formants[nformant].freq);
                xml->addpar("amp", _parameters->Pvowels[nvowel].formants[nformant].amp);
                xml->addpar("q", _parameters->Pvowels[nvowel].formants[nformant].q);
                xml->endbranch();
            }
            xml->endbranch();
        }
        xml->addpar("sequence_size", _parameters->Psequencesize);
        xml->addpar("sequence_stretch", _parameters->Psequencestretch);
        xml->addparbool("sequence_reversed", _parameters->Psequencereversed);
        for (int nseq = 0; nseq < FF_MAX_SEQUENCE; ++nseq)
        {
            xml->beginbranch("SEQUENCE_POS", nseq);
            xml->addpar("vowel_id", _parameters->Psequence[nseq].nvowel);
            xml->endbranch();
        }
        xml->endbranch();
    }
}

void FilterParamsSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    //filter parameters
    _parameters->Pcategory = xml->getpar127("category", _parameters->Pcategory);
    _parameters->Ptype = xml->getpar127("type", _parameters->Ptype);
    _parameters->Pfreq = xml->getpar127("freq", _parameters->Pfreq);
    _parameters->Pq = xml->getpar127("q", _parameters->Pq);
    _parameters->Pstages = xml->getpar127("stages", _parameters->Pstages);
    _parameters->Pfreqtrack = xml->getpar127("freq_track", _parameters->Pfreqtrack);
    _parameters->Pgain = xml->getpar127("gain", _parameters->Pgain);

    //formant filter parameters
    if (xml->enterbranch("FORMANT_FILTER"))
    {
        _parameters->Pnumformants = xml->getpar127("num_formants", _parameters->Pnumformants);
        _parameters->Pformantslowness = xml->getpar127("formant_slowness", _parameters->Pformantslowness);
        _parameters->Pvowelclearness = xml->getpar127("vowel_clearness", _parameters->Pvowelclearness);
        _parameters->Pcenterfreq = xml->getpar127("center_freq", _parameters->Pcenterfreq);
        _parameters->Poctavesfreq = xml->getpar127("octaves_freq", _parameters->Poctavesfreq);

        for (int nvowel = 0; nvowel < FF_MAX_VOWELS; ++nvowel)
        {
            if (xml->enterbranch("VOWEL", nvowel) == 0)
            {
                continue;
            }

            for (int nformant = 0; nformant < FF_MAX_FORMANTS; ++nformant)
            {
                if (xml->enterbranch("FORMANT", nformant) == 0)
                    continue;
                _parameters->Pvowels[nvowel].formants[nformant].freq = xml->getpar127("freq", _parameters->Pvowels[nvowel].formants[nformant].freq);
                _parameters->Pvowels[nvowel].formants[nformant].amp = xml->getpar127("amp", _parameters->Pvowels[nvowel].formants[nformant].amp);
                _parameters->Pvowels[nvowel].formants[nformant].q = xml->getpar127("q", _parameters->Pvowels[nvowel].formants[nformant].q);
                xml->exitbranch();
            }
            xml->exitbranch();
        }
        _parameters->Psequencesize = xml->getpar127("sequence_size", _parameters->Psequencesize);
        _parameters->Psequencestretch = xml->getpar127("sequence_stretch", _parameters->Psequencestretch);
        _parameters->Psequencereversed = xml->getparbool("sequence_reversed", _parameters->Psequencereversed);
        for (int nseq = 0; nseq < FF_MAX_SEQUENCE; ++nseq)
        {
            if (xml->enterbranch("SEQUENCE_POS", nseq) == 0)
            {
                continue;
            }
            _parameters->Psequence[nseq].nvowel = xml->getpar("vowel_id", _parameters->Psequence[nseq].nvowel, 0, FF_MAX_VOWELS - 1);
            xml->exitbranch();
        }
        xml->exitbranch();
    }
}
