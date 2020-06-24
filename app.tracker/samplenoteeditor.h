#ifndef SAMPLENOTEEDITOR_H
#define SAMPLENOTEEDITOR_H

#include "applicationsession.h"
#include "librarydialog.h"

#define SAMPLE_NOTE_MIN 35
#define SAMPLE_NOTE_MAX 81

class SampleNoteEditor
{
    ApplicationSession *_session = nullptr;
    LibraryDialog _libraryDialog;

public:
    SampleNoteEditor();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // SAMPLENOTEEDITOR_H
