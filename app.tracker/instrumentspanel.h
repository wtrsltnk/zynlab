#ifndef INSTRUMENTSPANEL_H
#define INSTRUMENTSPANEL_H

#include "applicationsession.h"
#include <zyn.common/globals.h>
#include <zyn.serialization/LibraryManager.h>

#define NOTE_C4 60
#define NOTE_C5 72

class InstrumentsPanel
{
    ApplicationSession *_session;
    IMixer *_mixer;
    ILibraryManager *_library;

    ILibraryItem *LibraryTree(
        ILibrary *library);

public:
    InstrumentsPanel();

    void SetUp(
        ApplicationSession *session,
        IMixer *mixer,
        ILibraryManager *library);
    void Render2d();
};

#endif // INSTRUMENTSPANEL_H
