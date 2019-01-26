/*
  ZynAddSubFX - a software synthesizer

  SUBnoteParameters.h - Parameters for SUBnote (SUBsynth)
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

#ifndef SUB_NOTE_PARAMETERS_SERIALIZER_H
#define SUB_NOTE_PARAMETERS_SERIALIZER_H

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.synth/SUBnoteParams.h>

class SUBnoteParametersSerializer
{
    SUBnoteParameters *_parameters;

public:
    SUBnoteParametersSerializer(SUBnoteParameters *parameters);
    virtual ~SUBnoteParametersSerializer();

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
};

#endif // SUB_NOTE_PARAMETERS_SERIALIZER_H
