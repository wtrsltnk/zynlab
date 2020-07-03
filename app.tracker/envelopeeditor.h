#ifndef ENVELOPEEDITOR_H
#define ENVELOPEEDITOR_H

#include <zyn.synth/EnvelopeParams.h>

class EnvelopeEditor
{
    EnvelopeParams *_params = nullptr;

public:
    EnvelopeEditor();

    void Setup(
        EnvelopeParams *_params);
    void Render2d();
};

#endif // ENVELOPEEDITOR_H
