#ifndef PADNOTEEDITOR_H
#define PADNOTEEDITOR_H

#include "applicationsession.h"

class PadNoteEditor
{
    ApplicationSession *_session = nullptr;

public:
    PadNoteEditor();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // PADNOTEEDITOR_H
