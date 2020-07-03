#ifndef SUBNOTEEDITOR_H
#define SUBNOTEEDITOR_H

#include "applicationsession.h"

class SubNoteEditor
{
    ApplicationSession *_session = nullptr;

public:
    SubNoteEditor();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // SUBNOTEEDITOR_H
