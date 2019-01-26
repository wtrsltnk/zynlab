#ifndef ADNOTE_VOICE_PARAM_SERIALIZER_H
#define ADNOTE_VOICE_PARAM_SERIALIZER_H

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.synth/ADnoteVoiceParam.h>

class ADnoteVoiceParamSerializer
{
    ADnoteVoiceParam *_parameters;

public:
    ADnoteVoiceParamSerializer(ADnoteVoiceParam *parameters);
    virtual ~ADnoteVoiceParamSerializer();

    void Serialize(IPresetsSerializer *xml, bool fmoscilused);
    void Deserialize(IPresetsSerializer *xml, unsigned nvoice);
};

#endif // ADNOTE_VOICE_PARAM_SERIALIZER_H
