#ifndef SYNTHEDITOR_H
#define SYNTHEDITOR_H

#include "applicationsession.h"
#include "samplenoteeditor.h"
#include <zyn.synth/EnvelopeParams.h>
#include <zyn.synth/LFOParams.h>

class SynthEditor
{
    ApplicationSession *_session = nullptr;

    SampleNoteEditor _sampleNoteEditor;

public:
    SynthEditor();

    void SetUp(ApplicationSession *session);
    void Render2d();

    void RenderEnvelope(EnvelopeParams *envelope, unsigned char *enabled);
    void RenderLfo(LFOParams *lfo, unsigned char *enabled);

    static char const *ID;
};

#endif // SYNTHEDITOR_H
