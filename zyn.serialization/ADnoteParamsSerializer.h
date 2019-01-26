/*
  ZynAddSubFX - a software synthesizer

  ADnoteParameters.h - Parameters for ADnote (ADsynth)
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

#ifndef AD_NOTE_PARAMETERS_SERIALIZER_H
#define AD_NOTE_PARAMETERS_SERIALIZER_H

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.synth/ADnoteParams.h>

class ADnoteParametersSerializer
{
    ADnoteParameters *_parameters;

public:
    ADnoteParametersSerializer(ADnoteParameters *parameters);
    virtual ~ADnoteParametersSerializer();

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
};

#endif // AD_NOTE_PARAMETERS_SERIALIZER_H
