#ifndef ADDNOTEEDITOR_H
#define ADDNOTEEDITOR_H

#include "applicationsession.h"
#include "envelopeeditor.h"

class AddNoteEditor
{
    ApplicationSession *_session = nullptr;
    EnvelopeEditor _ampEnvelope;

public:
    AddNoteEditor();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // ADDNOTEEDITOR_H
