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

    ILibraryItem *LibraryTree(
        ILibrary *library);

public:
    InstrumentsPanel();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // INSTRUMENTSPANEL_H
